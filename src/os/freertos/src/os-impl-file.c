/*
 *  NASA Docket No. GSC-18,370-1, and identified as "Operating System Abstraction Layer"
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
 * \file     os-impl-files.c
 * \ingroup  freertos
 * \author   joseph.p.hickey@nasa.gov
 * \author   Patrick Paul (https://github.com/pztrick)
 * \author   Fabio Benevenuti (UFRGS)
 *
 */

/****************************************************************************************
                                    INCLUDE FILES
****************************************************************************************/

#include <string.h>
#include <unistd.h>
#include <sys/types.h>


#include "common_types.h"
#include "osapi.h"
#include "os-shared-file.h"
#include "os-shared-filesys.h"
#include "os-shared-idmap.h"
#include "os-shared-globaldefs.h"

#include "os-freertos.h"
#include "os-impl-files.h"
#include "os-impl-filesys.h"

#ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
/* xilinx memory filesystem */
#include "xilmfs.h"
#else
/* freertos-plus-fat */
#include "portable/common/ff_ramdisk.h"
#include "include/ff_stdio.h"
#include "include/ff_headers.h"
#endif

/* Chan FatFS */
// #include "ff.h"


/****************************************************************************************
                                   GLOBAL DATA
 ***************************************************************************************/


/*
 * The global file handle table.
 *
 * This is shared by all OSAL entities that perform low-level I/O.
 */
OS_impl_file_internal_record_t OS_impl_filehandle_table[OS_MAX_NUM_OPEN_FILES];


/****************************************************************************************
                                        File API
 ***************************************************************************************/


/*----------------------------------------------------------------
 *
 * Function: OS_FreeRTOS_StreamAPI_Impl_Init
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *
 *-----------------------------------------------------------------*/
int32 OS_FreeRTOS_StreamAPI_Impl_Init(void)
{
    osal_index_t local_id;

    memset(OS_impl_filehandle_table, 0, sizeof(OS_impl_filehandle_table));

    /*
     * init all filehandles to -1, which is always invalid.
     * this isn't strictly necessary but helps when debugging.
     */
    for (local_id = 0; local_id < OS_MAX_NUM_OPEN_FILES; ++local_id)
    {
        #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
            OS_impl_filehandle_table[local_id].device     = -1;
            OS_impl_filehandle_table[local_id].fd         = -1;
        #else
            OS_impl_filehandle_table[local_id].pxFile     = NULL;
        #endif
    }

    return OS_SUCCESS;
} /* end OS_FreeRTOS_StreamAPI_Impl_Init */


/****************************************************************************************
                                 Named File API
 ***************************************************************************************/



/*----------------------------------------------------------------
 *
 * Function: OS_FileOpen_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_FileOpen_Impl(const OS_object_token_t *token, const char *local_path, int32 flags, int32 access)
{
    OS_object_token_t filesys_token;
    OS_filesys_internal_record_t  *filesys;
    OS_impl_filesys_internal_record_t *filesys_impl;
    osal_status_t status;

    OS_impl_file_internal_record_t *impl;
    char device_path [OS_MAX_LOCAL_PATH_LEN];
    //uint8 fstype;

    #ifdef OS_FILESYSTEM_NON_VOLATILE_IS_FATFS
        /* Used only for Chan FatFS */
        BYTE mode;
        FRESULT result;
    #endif

    status = OS_FreeRTOS_TranslateLocalPath(local_path, &filesys_token, device_path);
    if (status != OS_SUCCESS)
    {
        return status;
    }

    filesys = OS_OBJECT_TABLE_GET(OS_filesys_table, filesys_token);
    filesys_impl = OS_OBJECT_TABLE_GET(OS_impl_filesys_table, filesys_token);

    impl = OS_OBJECT_TABLE_GET(OS_impl_filehandle_table, *token);

    impl->fstype = filesys->fstype;
    #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
    impl->device = filesys_impl->device;
    #endif
    OS_ObjectIdRelease(&filesys_token);

    // TODO review the mappings from access and flags to the underlaying filesystem mode or permissions
    // TODO verify proper ftell() and fstat() after file open for all the possible modes (e.g read, write, append, truncate,...)

    if (impl->fstype == OS_FILESYS_TYPE_VOLATILE_DISK)
    {

        #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
            int mode;

            // localpath was translated to from virtual to system mount point
            // e.g.  /RAM1/cfestrup.scr -> /cfestrup.scr

            /*
            ** Check for a valid access mode
            ** For creating a file, OS_READ_ONLY does not make sense
            **
            ** See also FreeRTOS FAT FF_GetModeBits()
            */
            switch (access)
            {
                case OS_WRITE_ONLY:
                    if (flags & OS_FILE_FLAG_TRUNCATE)
                    {
                        mfs_delete_file(impl->device, device_path);
                        mode = MFS_MODE_CREATE;
                    }
                    else {
                        mode = MFS_MODE_WRITE;
                    }
                    break;
                case OS_READ_ONLY:
                    mode = MFS_MODE_READ;
                    break;
                case OS_READ_WRITE:
                    if ( (flags & OS_FILE_FLAG_CREATE) && (flags & OS_FILE_FLAG_TRUNCATE) )
                    {
                        mfs_delete_file(impl->device, device_path);
                        mode = MFS_MODE_CREATE;
                    }
                    else if (flags & OS_FILE_FLAG_CREATE)
                    {
                        mode = MFS_MODE_WRITE;
                    }
                    else if (flags & OS_FILE_FLAG_TRUNCATE)
                    {
                        mfs_delete_file(impl->device, device_path);
                        mode = MFS_MODE_CREATE;
                    }
                    else
                    {
                        mode = MFS_MODE_WRITE;
                    }
                    break;
                default:
                    return OS_ERR_FILE;
            }

            impl->fd = mfs_file_open(impl->device, device_path, mode);
            if ( impl->fd < 0)
            {
                return OS_ERROR;
            }

            return OS_SUCCESS;
        #else
            char os_perm_sz[5];
            /*
            ** Check for a valid access mode
            ** For creating a file, OS_READ_ONLY does not make sense
            **
            ** See also FreeRTOS FAT FF_GetModeBits()
            */
            switch (access)
            {
                case OS_WRITE_ONLY:
                    if (flags & OS_FILE_FLAG_TRUNCATE)
                    {
                        strcpy(os_perm_sz,"w");
                    }
                    else {
                        strcpy(os_perm_sz,"a");
                    }
                    break;
                case OS_READ_ONLY:
                    strcpy(os_perm_sz,"r");
                    break;
                case OS_READ_WRITE:
                    if ( (flags & OS_FILE_FLAG_CREATE) && (flags & OS_FILE_FLAG_TRUNCATE) )
                    {
                        strcpy(os_perm_sz,"rw");
                    }
                    else if (flags & OS_FILE_FLAG_CREATE)
                    {
                        strcpy(os_perm_sz,"ra");
                    }
                    else if (flags & OS_FILE_FLAG_TRUNCATE)
                    {
                        return OS_ERR_FILE;
                    }
                    else
                    {
                        strcpy(os_perm_sz,"r+");
                    }
                    break;
                default:
                    return OS_ERR_FILE;
            }

            /* This implementation relies on FreeRTOS FAT stdio features,
             * which keeps registry of filesystem path prefixes (device name or
             * mount point) and select the respective I/O manager to handle
             * files and directories.
             */

            impl->pxFile = ff_fopen( local_path, os_perm_sz );
            if ( impl->pxFile == NULL)
            {
                OS_DEBUG("OS_FileOpen_Impl(%s,\"%s\"): ERRNO %d\n", local_path, os_perm_sz, stdioGET_ERRNO());
                return OS_ERROR;
            }
        #endif

    }
    #ifdef OS_FILESYSTEM_NON_VOLATILE_IS_FATFS
    else if (impl->fstype == OS_FILESYS_TYPE_FS_BASED)
    {
        mode = 0;

        switch (access)
        {
        case OS_READ_ONLY:
            mode |= FA_READ;
            break;
        case OS_WRITE_ONLY:
            mode = FA_CREATE_ALWAYS | FA_WRITE;
            break;
        case OS_READ_WRITE:
            mode = FA_WRITE | FA_READ;
            break;
        default:
            break;
        }

        result = f_open(&impl->fp, device_path, mode);

        if (result != FR_OK)
        {
            return OS_ERROR;
        }
    }
    #endif /* end #ifdef OS_FILESYSTEM_NON_VOLATILE_IS_FATFS */
    else
    {
        OS_DebugPrintf(1, __func__, __LINE__, "OS_ERR_NOT_IMPLEMENTED \n");
        return OS_ERR_NOT_IMPLEMENTED;
    }

    return OS_SUCCESS;
} /* end OS_FileOpen_Impl */



/*----------------------------------------------------------------
 *
 * Function: OS_GenericRead_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_GenericRead_Impl(const OS_object_token_t *token, void *buffer, size_t nbytes, int32 timeout)
{
    OS_impl_file_internal_record_t *impl;

    #ifdef OS_FILESYSTEM_NON_VOLATILE_IS_FATFS
        /* Used for Chan FatFs*/
        FRESULT result;
        UINT br;
    #endif

    impl = OS_OBJECT_TABLE_GET(OS_impl_filehandle_table, *token);

    if (timeout != OS_PEND) {
        OS_DEBUG("read: timed read not supported.\n");
        return OS_ERR_NOT_IMPLEMENTED;
    }

    if (nbytes > 0)
    {
        if (impl->fstype == OS_FILESYS_TYPE_VOLATILE_DISK)
        {
            #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
                // OSAL API expects 0 if at end of file/stream data, negative if error
                // MFS does not distinguish between error or end of file, hence we always return success
                int nread = mfs_file_read(impl->device, impl->fd, buffer, nbytes);
                return (int32)nread;
            #else
                size_t result = ff_fread(buffer, 1, nbytes, impl->pxFile);
                if ( stdioGET_ERRNO( ) != pdFREERTOS_ERRNO_NONE )
                {
                    OS_DEBUG("read: %s\n", strerror(stdioGET_ERRNO( )));
                    return OS_ERROR;
                }
                else
                {
                    /* type conversion from size_t to int32 for return */
                    return (int32)result;
                }
            #endif
        }
        #ifdef OS_FILESYSTEM_NON_VOLATILE_IS_FATFS
        else if (impl->fstype == OS_FILESYS_TYPE_FS_BASED)
        {
            result = f_read(&impl->fp, buffer, nbytes, &br);

            if (result == FR_OK)
            {
                /*
                * The file read/write pointer of the file object advances number
                * of bytes read. After the function succeeded, *br should be checked 
                * to detect the end of file. In case of *br is less than btr, it 
                * means the read/write pointer reached end of the file during read operation.
                */
                if (br < nbytes)
                {
                    return br;
                }
            }
            else
            {
                return OS_ERROR;
            }
        }
        #endif /*end #ifdef OS_FILESYS_NON_VOLATILE_IS_FATFS */
        else
        {
            OS_DEBUG("OS_ERR_NOT_IMPLEMENTED \n");
            return OS_ERR_NOT_IMPLEMENTED;
        }
    }

    return nbytes;
} /* end OS_GenericRead_Impl */



/*----------------------------------------------------------------
 *
 * Function: OS_GenericWrite_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_GenericWrite_Impl(const OS_object_token_t *token, const void *buffer, size_t nbytes, int32 timeout)
{
    OS_impl_file_internal_record_t* impl;

    UNUSED_ARGUMENT(timeout);

    impl = OS_OBJECT_TABLE_GET(OS_impl_filehandle_table, *token);

    if (impl->fstype == OS_FILESYS_TYPE_VOLATILE_DISK)
    {
        #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
            int result = mfs_file_write(impl->device, impl->fd, buffer, nbytes);
            if ( result == MFS_SUCCESS )
            {
                return nbytes;
            }
            else
            {
                OS_DEBUG("%d->%d\n", nbytes, result);
                return OS_ERR_FILE;
            }
        #else
            size_t result = ff_fwrite(buffer, 1 ,nbytes, impl->pxFile);
            if (result == nbytes)
            {
                return nbytes;
            }
            else
            {
                OS_DEBUG("%d!=%d %s\n", nbytes, result, strerror(stdioGET_ERRNO( )));
                return OS_ERR_FILE;
            }
        #endif

    }
    #ifdef OS_FILESYSTEM_NON_VOLATILE_IS_FATFS
    else if (impl->fstype == OS_FILESYS_TYPE_FS_BASED)
    {
        FRESULT result;
        UINT bytes_written;

        result = f_write(&impl->fp, buffer, nbytes, &bytes_written);

        if (result == FR_OK)
        {
            if (bytes_written == nbytes)
            {
                return OS_SUCCESS;
            }
            else
            {
                return OS_ERR_FILE;
            }
        }
        else
        {
            return OS_ERROR;
        }
    }
    #endif /* end #ifdef OS_FILESYSTEM_NON_VOLATILE_IS_FATFS */
    else
    {
        OS_DebugPrintf(1, __func__, __LINE__, "OS_ERR_NOT_IMPLEMENTED \n");
        return OS_ERR_NOT_IMPLEMENTED;
    }

    return OS_ERR_FILE;

} /* end OS_GenericWrite_Impl */



/*----------------------------------------------------------------
 *
 * Function: OS_GenericClose_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_GenericClose_Impl(const OS_object_token_t *token)
{
    OS_impl_file_internal_record_t *impl;

    #ifdef OS_FILESYSTEM_NON_VOLATILE_IS_FATFS
        /* Chan FatFs */
        FRESULT result;
    #endif

    impl = OS_OBJECT_TABLE_GET(OS_impl_filehandle_table, *token);

    if (impl->fstype == OS_FILESYS_TYPE_VOLATILE_DISK)
    {
        // TODO: do a better error propagation.
        #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
            if ( mfs_file_close(impl->device, impl->fd) != MFS_SUCCESS )
            {
                OS_DEBUG("close failed. ignore. continue.\n");
            }
        #else
            int result;
            result = ff_fclose(impl->pxFile);
            if (result < 0)
            {
                /*
                * close() can technically fail for various reasons, but
                * there isn't much recourse if this call fails.  Just log
                * the failure for debugging.
                */
                OS_DEBUG("close: %s\n", strerror(stdioGET_ERRNO( )));
            }
        #endif
    }
    #ifdef OS_FILESYSTEM_NON_VOLATILE_IS_FATFS
    else if (impl->fstype == OS_FILESYS_TYPE_FS_BASED)
    {
        result = f_close(&impl->fp);

        if (result == FR_OK)
        {
            return OS_SUCCESS;
        }
        else
        {
            return OS_ERROR;
        }
    }
    #endif
    else
    {
        OS_DebugPrintf(1, __func__, __LINE__, "OS_ERR_NOT_IMPLEMENTED \n");
        return OS_ERR_NOT_IMPLEMENTED;
    }

    /* Reset the table entry */
    memset(impl, 0, sizeof(*impl));

    return OS_SUCCESS;
} /* end OS_GenericClose_Impl */


/*----------------------------------------------------------------
 *
 * Function: OS_GenericSeek_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_GenericSeek_Impl(const OS_object_token_t *token, int32 offset, uint32 whence)
{
    OS_impl_file_internal_record_t* impl;

    #ifdef OS_FILESYSTEM_NON_VOLATILE_IS_FATFS
        /* Chan FatFs */
        FRESULT result;
    #endif

    impl = OS_OBJECT_TABLE_GET(OS_impl_filehandle_table, *token);

    if (impl->fstype == OS_FILESYS_TYPE_VOLATILE_DISK)
    {
        #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
            long mfs_tell;

            // OSAL API expects file position (non-negative) on success, or relevant error code (negative)
            switch (whence)
            {
                case OS_SEEK_SET:
                    mfs_tell = mfs_file_lseek(impl->device, impl->fd, offset, MFS_SEEK_SET);
                    break;
                case OS_SEEK_CUR:
                    mfs_tell = mfs_file_lseek(impl->device, impl->fd, offset, MFS_SEEK_CUR);
                    break;
                case OS_SEEK_END:
                    mfs_tell = mfs_file_lseek(impl->device, impl->fd, offset, MFS_SEEK_END);
                    break;
                default:
                    return OS_ERROR;
            }
            // MFS fails with seek == file size, but it could be valid...
            if (mfs_tell < 0)
            {
                return OS_ERR_FILE;
            }
            return mfs_tell;

        #else
            OS_DEBUG("OS_ERR_NOT_IMPLEMENTED \n");
            return OS_ERR_NOT_IMPLEMENTED;
        #endif

    }
    #ifdef OS_FILESYSTEM_NON_VOLATILE_IS_FATFS
    else if (impl->fstype == OS_FILESYS_TYPE_FS_BASED)
    {
        result = f_lseek(&impl->fp, offset);

        if (result == FR_OK)
        {
            return OS_SUCCESS;
        }
        else
        {
            return OS_ERROR;
        }
    }
    #endif
    else
    {
        OS_DEBUG("OS_ERR_NOT_IMPLEMENTED \n");
        return OS_ERR_NOT_IMPLEMENTED;
    }

    return OS_ERR_FILE;
} /* end OS_GenericSeek_Impl */
