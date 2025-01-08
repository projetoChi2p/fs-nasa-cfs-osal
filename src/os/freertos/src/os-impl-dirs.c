/*
 *  NASA Docket No. GSC-18,370-1, and identified as "Operating System Abstraction Layer"
 *
 *
 *  Copyright (c) 2024 Universidade Federal do Rio Grande do Sul
 *
 *
 *  Copyright (c) 2021 Patrick Paul
 *  SPDX-License-Identifier: MIT-0
 *
 *
 *  Copyright (c) 2019 United States Government as represented by
 *  the Administrator of the National Aeronautics and Space Administration.
 *  All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * \file     os-impl-dirs.c
 * \ingroup  freertos
 * \author   joseph.p.hickey@nasa.gov
 * \author   Patrick Paul
 * \author   Fabio Benevenuti
 */


#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "os-freertos.h"

#ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
/* xilinx memory filesystem */
#include "xilmfs.h"
#else
/* freertos-plus-fat */
#include "portable/common/ff_ramdisk.h"
#include "include/ff_stdio.h"
#include "include/ff_headers.h"
#endif

#include "common_types.h"
#include "osapi.h"
//#include "osapi-os-core.h"
//#include "osapi-os-filesys.h"
#include "os-shared-filesys.h"
#include "os-shared-file.h"
#include "os-shared-idmap.h"
#include "os-shared-dir.h"
#include "os-shared-globaldefs.h"
#include "os-impl-dirs.h"
#include "os-impl-files.h"
#include "os-impl-filesys.h"



/****************************************************************************************
                                   Data Types
****************************************************************************************/


/****************************************************************************************
                                   GLOBAL DATA
 ***************************************************************************************/

/*
 * The open directory handle table.
 */
OS_impl_dir_internal_record_t OS_impl_dir_table[OS_MAX_NUM_OPEN_DIRS];


#define DIR_FLAG_IS_DYN_ALLOC 0x01
#ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
    // No action
#else
#define DIR_FLAG_GOT_FIRST 0x02  /* used by FreeRTOS+FAT to distinguish between find first and find next */
#endif


/****************************************************************************************
                         IMPLEMENTATION-SPECIFIC ROUTINES
             These are specific to this particular operating system
 ****************************************************************************************/

/****************************************************************************************
                                Filesys API
 ***************************************************************************************/

/* --------------------------------------------------------------------------------------
    Name: OS_FreeRTOS_DirAPI_Impl_Init

    Purpose: Directory subsystem global initialization

    Returns: OS_SUCCESS if success
 ---------------------------------------------------------------------------------------*/
int32 OS_FreeRTOS_DirAPI_Impl_Init(void)
{
    memset(OS_impl_dir_table, 0, sizeof(OS_impl_dir_table));

    osal_index_t local_id;

    for (local_id = 0; local_id < OS_MAX_NUM_OPEN_DIRS; ++local_id)
    {
        #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
            OS_impl_dir_table[local_id].device = -1;
            OS_impl_dir_table[local_id].fd     = -1;
        #else
            // no action 
        #endif
    }

    return OS_SUCCESS;
}


/*----------------------------------------------------------------
 *
 * Function: OS_DirOpen_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_DirOpen_Impl(const OS_object_token_t *token, const char *local_path)
{
    OS_object_token_t filesys_token;
    OS_filesys_internal_record_t  *filesys;
    OS_impl_filesys_internal_record_t *filesys_impl;
    osal_status_t return_code;
    os_fstat_t FileStats;

    //OS_dir_internal_record_t *     dir;
    OS_impl_dir_internal_record_t* impl;

    char device_path [OS_MAX_LOCAL_PATH_LEN];

    OS_CHECK_STRING(local_path, sizeof(impl->device_path), OS_FS_ERR_PATH_TOO_LONG);
    OS_CHECK_PATHNAME(local_path);

    return_code = OS_FileStat_Impl(local_path, &FileStats);
    if  ( return_code != OS_SUCCESS )
    {
        return OS_ERROR;
    }
    if ( !OS_FILESTAT_ISDIR(FileStats) )
    {
        OS_DEBUG("Path is not a directory.\n");
        return OS_ERROR;
    }

    return_code = OS_FreeRTOS_TranslateLocalPath(local_path, &filesys_token, device_path);
    if (return_code != OS_SUCCESS)
    {
        return return_code;
    }

    filesys = OS_OBJECT_TABLE_GET(OS_filesys_table, filesys_token);
    filesys_impl = OS_OBJECT_TABLE_GET(OS_impl_filesys_table, filesys_token);

    //dir = OS_OBJECT_TABLE_GET(OS_dir_table, *token);
    impl  = OS_OBJECT_TABLE_GET(OS_impl_dir_table, *token);

    impl->fstype = filesys->fstype;
    #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
    impl->device = filesys_impl->device;
    #endif
    OS_ObjectIdRelease(&filesys_token);

    /*
     * Take action based on the type of volume
     */
    switch(impl->fstype) 
    {
        case OS_FILESYS_TYPE_VOLATILE_DISK:
            #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
                // MFS uses path relative to device root
                strncpy(impl->device_path, device_path, sizeof(impl->device_path) - 1);
                impl->device_path[ sizeof(impl->device_path) - 1 ] = 0;

                impl->fd =  mfs_dir_open(impl->device, device_path);
                if (impl->fd < 0)
                {
                    OS_DEBUG("Failed mfs_dir_open(). Result %d.\n", impl->fd);
                    return_code = OS_ERROR;
                    break;
                }
                break;
            #else /* !OS_FILESYSTEM_RAMDISK_IS_XILMFS */
                // FreeRTOS+FAT uses the full local_path to switch between multiple filesystems
                strncpy(impl->device_path, local_path, sizeof(impl->device_path) - 1);
                impl->device_path[ sizeof(impl->device_path) - 1 ] = 0;

                impl->pxFindStruct = ( FF_FindData_t * ) pvPortMalloc( sizeof( FF_FindData_t ) );
                if ( impl->pxFindStruct == NULL )
                {
                    OS_DEBUG("Failed to allocated working storage.\n");
                    return_code = OS_ERROR;
                    break;
                }

                impl->flags = DIR_FLAG_IS_DYN_ALLOC;
                memset( impl->pxFindStruct, 0, sizeof( FF_FindData_t ) );
            #endif /* !OS_FILESYSTEM_RAMDISK_IS_XILMFS */
            return_code = OS_SUCCESS;
            break;
        default:
            OS_DEBUG("OS_ERR_NOT_IMPLEMENTED \n");
            return_code = OS_ERR_NOT_IMPLEMENTED;
    }

    return return_code;
} /* end OS_DirOpen_Impl */


/*----------------------------------------------------------------
 *
 * Function: OS_DirClose_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_DirClose_Impl(const OS_object_token_t *token)
{

    //OS_dir_internal_record_t *     dir;
    OS_impl_dir_internal_record_t* impl;

    //dir = OS_OBJECT_TABLE_GET(OS_dir_table, *token);
    impl  = OS_OBJECT_TABLE_GET(OS_impl_dir_table, *token);

    #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
        int mfs_result;
        mfs_result = mfs_dir_close(impl->device, impl->fd);
        if ( mfs_result != MFS_SUCCESS )
        {
            OS_DEBUG("Failed mfs_dir_close(). Result %d.\n", mfs_result);
            return OS_ERROR;
        }
    #else

        if ( (impl->pxFindStruct != NULL) && ( impl->flags & DIR_FLAG_IS_DYN_ALLOC) )
        {
            vPortFree(impl->pxFindStruct);
        }
    #endif

    /* Reset the table entry */
    memset(impl, 0, sizeof(*impl));

    return OS_SUCCESS;
} /* end OS_DirClose_Impl */



/*----------------------------------------------------------------
 *
 * Function: OS_DirRead_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_DirRead_Impl(const OS_object_token_t *token, os_dirent_t *dirent)
{

    //OS_dir_internal_record_t *     dir;
    OS_impl_dir_internal_record_t* impl;

    //dir = OS_OBJECT_TABLE_GET(OS_dir_table, *token);
    impl  = OS_OBJECT_TABLE_GET(OS_impl_dir_table, *token);

    #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
        int mfs_result;
        int filesize_UNUSED;
        int filetype_UNUSED;
        char *filename;
        mfs_result = mfs_dir_read(impl->device, impl->fd, &filename, &filesize_UNUSED, &filetype_UNUSED);
        if ( mfs_result != MFS_SUCCESS )
        {
            // Return OS_ERROR at the end of the directory or if the OS call otherwise fails
            return OS_ERROR;
        }
        strncpy(dirent->FileName, filename, sizeof(dirent->FileName) - 1 );
        dirent->FileName[ sizeof(dirent->FileName) - 1 ] = 0;
    #else
        if ( impl->flags & DIR_FLAG_GOT_FIRST )
        {
            if ( ff_findnext( impl->pxFindStruct ) != FF_ERR_NONE )
            {
                // Return OS_ERROR at the end of the directory or if the OS call otherwise fails
                return OS_ERROR;
            }
        }
        else
        {
            impl->flags |= DIR_FLAG_GOT_FIRST;
            if ( ff_findfirst( impl->device_path, impl->pxFindStruct ) != FF_ERR_NONE )
            {
                // Return OS_ERROR at the end of the directory or if the OS call otherwise fails
                return OS_ERROR;
            }
        }
        strncpy(dirent->FileName, impl->pxFindStruct->pcFileName, sizeof(dirent->FileName) - 1 );
        dirent->FileName[ sizeof(dirent->FileName) - 1 ] = 0;
    #endif

    return OS_SUCCESS;
} /* end OS_DirRead_Impl */



/*----------------------------------------------------------------
 *
 * Function: OS_DirRewind_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_DirRewind_Impl(const OS_object_token_t *token)
{
    //OS_dir_internal_record_t *     dir;
    OS_impl_dir_internal_record_t* impl;

    //dir = OS_OBJECT_TABLE_GET(OS_dir_table, *token);
    impl  = OS_OBJECT_TABLE_GET(OS_impl_dir_table, *token);

    #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
        int mfs_result;
        mfs_result = mfs_dir_close(impl->device, impl->fd);
        if ( mfs_result != MFS_SUCCESS )
        {
            OS_DEBUG("Failed mfs_dir_close(). Result %d. Ignored.\n", mfs_result);
        }
        impl->fd =  mfs_dir_open(impl->device, impl->device_path);
        if (impl->fd < 0)
        {
            OS_DEBUG("Failed mfs_dir_open(). Result %d.\n", impl->fd);
            return OS_ERROR;
        }
    #else
        memset( impl->pxFindStruct, 0, sizeof( FF_FindData_t ) );
        impl->flags &= (~DIR_FLAG_GOT_FIRST);
    #endif

    return OS_SUCCESS;
} /* end OS_DirRewind_Impl */


/*----------------------------------------------------------------
 *
 * Function: OS_DirCreate_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_DirCreate_Impl(const char *local_path, uint32 access)
{
    OS_object_token_t filesys_token;
    OS_filesys_internal_record_t  *filesys;
    OS_impl_filesys_internal_record_t *filesys_impl;
    osal_status_t return_code;
    uint8_t fstype;
    #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
    int device;
    #endif

    char device_path [OS_MAX_LOCAL_PATH_LEN];

    OS_CHECK_STRING(local_path, sizeof(device_path), OS_FS_ERR_PATH_TOO_LONG);
    OS_CHECK_PATHNAME(local_path);

    return_code = OS_FreeRTOS_TranslateLocalPath(local_path, &filesys_token, device_path);
    if (return_code != OS_SUCCESS)
    {
        return return_code;
    }

    filesys = OS_OBJECT_TABLE_GET(OS_filesys_table, filesys_token);
    filesys_impl = OS_OBJECT_TABLE_GET(OS_impl_filesys_table, filesys_token);

    fstype = filesys->fstype;
    #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
    device = filesys_impl->device;
    #endif
    OS_ObjectIdRelease(&filesys_token);

    /*
     * Take action based on the type of volume
     */
    switch(fstype)
    {
        case OS_FILESYS_TYPE_VOLATILE_DISK:
            #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
                if (access == OS_READ_ONLY)
                {
                    OS_DEBUG("Warning: R/O access ignored. A R/W directory will be created.\n");
                }
                if (access == OS_WRITE_ONLY)
                {
                    OS_DEBUG("Warning: W/O access ignored. A R/W directory will be created.\n");
                }

                int mfs_result;
                // MFS uses path relative to device root
                mfs_result = mfs_create_dir(device, device_path);
                if ( mfs_result == MFS_ERROR_FAILED )
                {
                    OS_DEBUG("Failed mfs_create_dir(). Result %d.\n", mfs_result);
                    return_code = OS_ERROR;
                    break;
                }
                return_code = OS_SUCCESS;
                break;
            #else /* !OS_FILESYSTEM_RAMDISK_IS_XILMFS */
                OS_DEBUG("OS_ERR_NOT_IMPLEMENTED \n");
                return_code = OS_ERR_NOT_IMPLEMENTED;
            #endif /* !OS_FILESYSTEM_RAMDISK_IS_XILMFS */
            return_code = OS_SUCCESS;
            break;
        default:
            OS_DEBUG("OS_ERR_NOT_IMPLEMENTED \n");
            return_code = OS_ERR_NOT_IMPLEMENTED;
    }

    return return_code;

} /* end OS_DirCreate_Impl */



/*----------------------------------------------------------------
 *
 * Function: OS_DirRemove_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_DirRemove_Impl(const char *local_path)
{
    OS_object_token_t filesys_token;
    OS_filesys_internal_record_t  *filesys;
    OS_impl_filesys_internal_record_t *filesys_impl;
    osal_status_t return_code;
    uint8_t fstype;
    #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
    int device;
    int mfs_result;
    #endif

    char device_path [OS_MAX_LOCAL_PATH_LEN];

    OS_CHECK_STRING(local_path, sizeof(device_path), OS_FS_ERR_PATH_TOO_LONG);
    OS_CHECK_PATHNAME(local_path);

    return_code = OS_FreeRTOS_TranslateLocalPath(local_path, &filesys_token, device_path);
    if (return_code != OS_SUCCESS)
    {
        return return_code;
    }

    filesys = OS_OBJECT_TABLE_GET(OS_filesys_table, filesys_token);
    filesys_impl = OS_OBJECT_TABLE_GET(OS_impl_filesys_table, filesys_token);

    fstype = filesys->fstype;
    #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
    device = filesys_impl->device;
    #endif
    OS_ObjectIdRelease(&filesys_token);

    /*
     * Take action based on the type of volume
     */
    switch(fstype)
    {
        case OS_FILESYS_TYPE_VOLATILE_DISK:
            #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
                // MFS uses path relative to device root
                mfs_result = mfs_delete_dir(device, device_path);
                if ( mfs_result == MFS_ERROR_FAILED )
                {
                    return_code = OS_ERROR;
                    break;
                }
                return_code = OS_SUCCESS;
                break;
            #else /* !OS_FILESYSTEM_RAMDISK_IS_XILMFS */
                OS_DEBUG("OS_ERR_NOT_IMPLEMENTED \n");
                return_code = OS_ERR_NOT_IMPLEMENTED;
            #endif /* !OS_FILESYSTEM_RAMDISK_IS_XILMFS */
            return_code = OS_SUCCESS;
            break;
        default:
            OS_DEBUG("OS_ERR_NOT_IMPLEMENTED \n");
            return_code = OS_ERR_NOT_IMPLEMENTED;
    }

    return return_code;
} /* end OS_DirRemove_Impl */
