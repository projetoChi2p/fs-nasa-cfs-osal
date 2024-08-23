/*
 *  NASA Docket No. GSC-18,370-1, and identified as "Operating System Abstraction Layer"
 *
 *
 *  Copyright (c) 2024 Universidade Federal do Rio Grande do Sul (UFRGS)
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
 * \file     os-impl-filesys.c
 * \ingroup  freertos
 * \author   joseph.p.hickey@nasa.gov
 * \author   Patrick Paul (https://github.com/pztrick)
 * \author   Fabio Benevenuti (UFRGS)
 */

#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "common_types.h"
#include "osapi.h"
#include "os-shared-filesys.h"
#include "os-shared-idmap.h"
#include "os-shared-globaldefs.h"

/* @TODO Move ramdrive initialization to PSP or CFE layer
 * We are currently provisionally populating ramdrive file with 
 * data from CFE, namely STARTUP_SCR_DATA, which makes OSAL 
 * dependent of CFE and arguably violates API.
 * This choice for initializing CFE file here was due to minimizing
 * the changes on CFE code during this experimental implementation.
 * As OSAL for FreeRTOS gains maturity, we can then legitimate
 * changes on CFE.
 */

/* FreeRTOS plus FAT */
#include "os-freertos.h"
#include "portable/common/ff_ramdisk.h"
#include "include/ff_stdio.h"
#include "include/ff_headers.h"


/****************************************************************************************
                                   Data Types
****************************************************************************************/

typedef struct
{
    int  fs_alloc_type;
    FF_Disk_t *  allocated_disk;
} OS_impl_filesys_internal_record_t;

#define OS_FILESYS_ALLOCATION_TYPE_STATIC   1
#define OS_FILESYS_ALLOCATION_TYPE_DYNAMIC  2


/****************************************************************************************
                                   GLOBAL DATA
 ***************************************************************************************/

/*  FreeRTOS FAT RAM Disk cache size.
 *  Must be multiple of sector size, which is 512 bytes, and at least twice as big.
 */
#define FREERTOS_FAT_SECTOR_SIZE 512
#define FREERTOS_FAT_RAMDISK_CACHE_MIN_SIZE (FREERTOS_FAT_SECTOR_SIZE*5)  


/*
 * The implementation-specific file system state table.
 * This keeps record of the FreeRTOS plus FAT disks.
 */
OS_impl_filesys_internal_record_t OS_impl_filesys_table[OS_MAX_FILE_SYSTEMS];



/****************************************************************************************
                                Filesys API
 ***************************************************************************************/

/* --------------------------------------------------------------------------------------
    Name: OS_FreeRTOS_FileSysAPI_Impl_Init

    Purpose: Filesystem API global initialization

    Returns: OS_SUCCESS if success
 ---------------------------------------------------------------------------------------*/
int32 OS_FreeRTOS_FileSysAPI_Impl_Init(void)
{
    /* FBV 2024-01-10 FreeRTOS FAT ramdisk impl relies on 512 
       sectors, 1 sector per clusters and, if fit, FAT16. 
    */
    // configASSERT( CFE_PLATFORM_ES_RAM_DISK_SECTOR_SIZE == FREERTOS_FAT_SECTOR_SIZE);

    /* clear the local filesys table */
    memset(OS_impl_filesys_table, 0, sizeof(OS_impl_filesys_table));

    /* FBV 2024-01-10 Initialize known FS. 
       FreeRTOS FAT keeps a list of known FS, up to ffconfigMAX_FILE_SYS. 
       For Patrick implementation, API Init is called from inside a task. 
    */
    vTaskSuspendAll();
    FF_FS_Init();
    xTaskResumeAll( );

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 * Function: OS_FileSysStartVolume_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_FileSysStartVolume_Impl(const OS_object_token_t *token)
{
    // OS_filesys_internal_record_t *     local;
    OS_impl_filesys_internal_record_t *impl;

    impl  = OS_OBJECT_TABLE_GET(OS_impl_filesys_table, *token);
    // local = OS_OBJECT_TABLE_GET(OS_filesys_table, *token);

    memset(impl, 0, sizeof(*impl));

    // OS_DebugPrintf(1, __func__, __LINE__, "vol:'%s' dev:'%s' mnt:'%s' fs:%d%s addr:%p bsz:%d blks:%d\n", 
    // local->volume_name,
    // local->device_name, 
    // local->system_mountpt,
    // local->fstype,
    // (local->fstype == OS_FILESYS_TYPE_VOLATILE_DISK)?"(Volatile)":"",
    // local->address, local->blocksize, local->numblocks);

    return OS_SUCCESS;

} /* end OS_FileSysStartVolume_Impl */


/*----------------------------------------------------------------
 *
 * Function: OS_FileSysFormatVolume_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_FileSysFormatVolume_Impl(const OS_object_token_t *token)
{
    OS_filesys_internal_record_t *     local;
    OS_impl_filesys_internal_record_t *impl;
    int32                              return_code;

    impl  = OS_OBJECT_TABLE_GET(OS_impl_filesys_table, *token);
    local = OS_OBJECT_TABLE_GET(OS_filesys_table, *token);

    memset(impl, 0, sizeof(*impl));

    // OS_DebugPrintf(1, __func__, __LINE__, "vol:'%s' dev:'%s' mnt:'%s' fs:%d%s addr:%p bsz:%d blks:%d\n", 
    // local->volume_name,
    // local->device_name, 
    // local->system_mountpt,
    // local->fstype,
    // (local->fstype == OS_FILESYS_TYPE_VOLATILE_DISK)?"(Volatile)":"",
    // local->address, local->blocksize, local->numblocks);

    /*
     * Take action based on the type of volume
     */
    switch(local->fstype) 
    {
        case OS_FILESYS_TYPE_VOLATILE_DISK:
            if ( local->blocksize != FREERTOS_FAT_SECTOR_SIZE ) 
            {
                OS_DebugPrintf(1, __func__, __LINE__, "Unsupported block size %u\n", (unsigned int)(local->blocksize));
                return_code = OS_ERROR;
                break;
            }

            //OS_DEBUG("No RAMDISK available at address %p\n", local->address);


            if (local->address == NULL) {
                impl->fs_alloc_type  = OS_FILESYS_ALLOCATION_TYPE_DYNAMIC;
                local->address = pvPortMalloc(local->numblocks * local->blocksize);
            }

            impl->allocated_disk = FF_RAMDiskInit(
                /*(char *)*/    local->device_name, // Discarding const qualifier!
                (uint8_t *) local->address,
                local->numblocks,
                FREERTOS_FAT_RAMDISK_CACHE_MIN_SIZE
            );

            if (impl->allocated_disk == NULL)
            {
                //OS_DEBUG("FF_RAMDiskInit() failed\n");
                return_code = OS_INVALID_POINTER;
                break;
            }

            if (impl->allocated_disk->pxIOManager->xPartition.ucType != FF_T_FAT16)
            {
                //OS_DEBUG("FF_RAMDiskInit(%s) unexpected filesystem type: %d.\n", 
                //    local->device_name,
                //    impl->allocated_disk->pxIOManager->xPartition.ucType);
                return_code = OS_INVALID_POINTER;
                break;
            }

            // name the volume
            sprintf(
                impl->allocated_disk->pxIOManager->xPartition.pcVolumeLabel,
                local->volume_name
            );

            //OS_DebugPrintf(1, __func__, __LINE__, "RAM Disk partition:\n");
            //FF_RAMDiskShowPartition(impl->allocated_disk);


            //impl->mount_fstype  = local->fstype;
            //impl->mount_options = RTEMS_FILESYSTEM_READ_WRITE;
            //snprintf(impl->blockdev_name, sizeof(impl->blockdev_name), "%s%c", RAMDISK_DEVICE_BASE_NAME,
            //         (int)OS_ObjectIndexFromToken(token) + 'a');
            //snprintf(impl->blockdev_name, sizeof(impl->blockdev_name), local->device_name);

            //OS_DEBUG("RAM disk initialized: volume=%s device=%s address=0x%08lX\n", local->volume_name,
            //         /*impl->blockdev_name*/ local->device_name, (unsigned long)local->address);

            return_code = OS_SUCCESS;
            break;
        
        default:
            return_code = OS_ERR_NOT_IMPLEMENTED;
            break;
    }

    /*
     * If the operation was generally successful but a (real) FS
     * mount point was not supplied, then generate one now.
     *
     * The path will be simply /<VOLNAME>
     */
    if (return_code == OS_SUCCESS && local->system_mountpt[0] == 0) 
    {
        if (strlen(local->device_name) >= sizeof(local->system_mountpt))
        {
            return_code = OS_FS_ERR_PATH_TOO_LONG;
        }
        else {
            //snprintf(local->system_mountpt, sizeof(local->system_mountpt), "%s", local->device_name);
            strncpy(local->system_mountpt, local->device_name, sizeof(local->system_mountpt));
            //OS_DEBUG("OSAL: using mount point %s for dev:%s vol:%s\n", local->system_mountpt, local->device_name, local->volume_name);
        }
    }

    return return_code;
} /* end OS_FileSysFormatVolume_Impl */

/*----------------------------------------------------------------
 *
 * Function: OS_FileSysMountVolume_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_FileSysMountVolume_Impl(const OS_object_token_t *token)
{

    OS_filesys_internal_record_t *     local;
    OS_impl_filesys_internal_record_t *impl;
    //struct stat                        stat_buf;

    impl  = OS_OBJECT_TABLE_GET(OS_impl_filesys_table, *token);
    local = OS_OBJECT_TABLE_GET(OS_filesys_table, *token);

    // OS_DebugPrintf(1, __func__, __LINE__, "vol:'%s' dev:'%s' mnt:'%s' fs:%d%s addr:%p bsz:%d blks:%d\n", 
    // local->volume_name,
    // local->device_name, 
    // local->system_mountpt,
    // local->fstype,
    // (local->fstype == OS_FILESYS_TYPE_VOLATILE_DISK)?"(Volatile)":"",
    // local->address, local->blocksize, local->numblocks);

    if (impl->allocated_disk == NULL)
    {
        OS_DEBUG("mount(%s) DEVICE NOT READY/FORMATTED\n", local->device_name);
        return OS_ERROR;
    }

    if ( strcmp(local->device_name, local->system_mountpt) != 0 ) {
        OS_DEBUG("mount(%s != %s) ERROR\n", local->device_name, local->system_mountpt);
        return OS_ERR_NOT_IMPLEMENTED;
    }

    /* No action.
     * This implementation relies on FreeRTOS FAT stdio features,
     * which keeps registry of filesystem path prefixes (device name)
     * and select the respective I/O manager to handle files and
     * directories.
     */

    // OS_DebugPrintf(1, __func__, __LINE__, "RAM Disk partition:\n");
    // FF_RAMDiskShowPartition(impl->allocated_disk);

    return OS_SUCCESS;

} /* end OS_FileSysMountVolume_Impl */



/****************************************************************************************
                                Experimental
 ***************************************************************************************/
/*----------------------------------------------------------------
 *
 * Function: OS_FileSysUnmountVolume_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_FileSysUnmountVolume_Impl(const OS_object_token_t *token)
{
    OS_filesys_internal_record_t *     local;
    OS_impl_filesys_internal_record_t* impl;

    local = OS_OBJECT_TABLE_GET(OS_filesys_table, *token);
    impl  = OS_OBJECT_TABLE_GET(OS_impl_filesys_table, *token);

    if (impl->fs_alloc_type == OS_FILESYS_ALLOCATION_TYPE_DYNAMIC)
    {
        vPortFree(local->address);
    }

    local->address = NULL;

    return OS_SUCCESS;
} /* end OS_FileSysUnmountVolume_Impl */




int32 devel_breakpoint(void){
    OS_DebugPrintf(1, __func__, __LINE__, "return OS_SUCCESS\n");
    return OS_SUCCESS;
}


// @FIXME not implemented and returning OS_SUCCESS
int32 xxxxxxOS_FileAPI_Init(void){
    OS_DebugPrintf(1, __func__, __LINE__, "OS_FileAPI_Init()\n");
    return devel_breakpoint();
}
int32 xxxxOS_DirAPI_Init(void){
    OS_DebugPrintf(1, __func__, __LINE__, "OS_DirAPI_Init()\n");
    return devel_breakpoint();
}

#if 0
int32 xxxxOS_FileSysAPI_Init(void){
    g_pxDiskRAM = NULL;
    OS_DebugPrintf(1, __func__, __LINE__, "OS_FileSysAPI_Init()\n");
    return devel_breakpoint();
}
int32 xxxxOS_mkfs(char *address, const char *devname, const char *volname, size_t blocksize, osal_blockcount_t numblocks){
    if ( strcmp(devname, WELL_KNOWN_DEVICE_NAME) != 0 ) {
        OS_DebugPrintf(1, __func__, __LINE__, "Unsupported device %s\n", devname);
        return OS_ERROR;
    }
    if ( strcmp(volname, WELL_KNOWN_VOLUME_NAME) != 0 ) {
        OS_DebugPrintf(1, __func__, __LINE__, "Unsupported volume %s\n", volname);
        return OS_ERROR;
    }
    if ( blocksize != FREERTOS_FAT_SECTOR_SIZE ) {
        OS_DebugPrintf(1, __func__, __LINE__, "Unsupported block size %u\n", (unsigned int)blocksize);
        return OS_ERROR;
    }
    g_pxDiskRAM = FF_RAMDiskInit(
        (char *)devname, // Discarding const qualifier!
        (uint8_t *) address,
        numblocks,
        RAMDISK_CACHE_SIZE
    );
    configASSERT(g_pxDiskRAM);
    configASSERT(g_pxDiskRAM->pxIOManager->xPartition.ucType == FF_T_FAT16);
    // name the volume
    sprintf(
        g_pxDiskRAM->pxIOManager->xPartition.pcVolumeLabel,
        volname
    );

    FF_RAMDiskShowPartition(g_pxDiskRAM);

    {
        OS_DebugPrintf(1, __func__, __LINE__, "STARTUP_SCR_DATA[%d]:\n%s\n", STARTUP_SCR_SIZE, STARTUP_SCR_DATA);
        FF_FILE *pxDestFile;
        size_t written_done;
        const char sz_scr[] = "/ramdev0/cfe_es_startup.scr";
        pxDestFile = ff_fopen( sz_scr, "w" );
        if ( pxDestFile != NULL) {
            written_done = ff_fwrite( STARTUP_SCR_DATA, sizeof(STARTUP_SCR_DATA[0]), STARTUP_SCR_SIZE, pxDestFile );
            if ( written_done != STARTUP_SCR_SIZE ) {
                OS_DebugPrintf(1, __func__, __LINE__, "Failed to written %d!=%d ERRNO %d\n", written_done, STARTUP_SCR_SIZE, stdioGET_ERRNO());
            }
            ff_fclose(pxDestFile);
        }
        else {
            OS_DebugPrintf(1, __func__, __LINE__, "ff_fopen(%s) failed: %d\n", sz_scr, stdioGET_ERRNO());
        }
    }

    return OS_SUCCESS;
}
int32 xxxxOS_initfs(char *address, const char *devname, const char *volname, size_t blocksize, osal_blockcount_t numblocks){
    OS_DebugPrintf(1, __func__, __LINE__, "OS_initfs(a=%u,d=%s,v=%s,b=%u,n=%u)\n", (unsigned int)address, devname, volname, (unsigned int)blocksize, (unsigned int)numblocks);
    return devel_breakpoint();
}
int32 xxxxOS_mount(const char *devname, const char *mountpoint){
    if ( strcmp(devname, WELL_KNOWN_DEVICE_NAME) != 0 ) {
        OS_DebugPrintf(1, __func__, __LINE__, "Unsupported device %s\n", devname);
        return OS_ERROR;
    }
    if ( strcmp(mountpoint, WELL_KNOWN_MOUNT_POINT) != 0 ) {
        OS_DebugPrintf(1, __func__, __LINE__, "Unsupported mount point %s\n", mountpoint);
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

int32 xxxxOS_rmfs(const char *devname){
    OS_DebugPrintf(1, __func__, __LINE__, "OS_rmfs(d=%s)\n", devname);
    return devel_breakpoint();
}
int32 xxxxOS_unmount(const char *mountpoint){
    OS_DebugPrintf(1, __func__, __LINE__, "OS_unmount(d=%s)\n", mountpoint);
    return devel_breakpoint();
}

// @FIXME not implemented and returning OS_ERROR
int32 OS_DirectoryClose(osal_id_t dir_id){
    OS_DebugPrintf(1, __func__, __LINE__, "OS_DirectoryClose() OS_ERROR\n");
    return OS_ERROR;
}

//OS_FileSysAddFixedMap
//OS_TranslatePath

int32 xxxxOS_OpenCreate(osal_id_t *filedes, const char *path, int32 flags, int32 access){
    const char *   basename_ptr;
    size_t         fullpath_len;
    size_t         path_len;
    size_t         mountpoint_len;

//flags

    /*
    ** Check to see if the path pointers are NULL
    */
    if (filedes == NULL || path == NULL)
    {
        OS_DebugPrintf(1, __func__, __LINE__, "OS_OpenCreate(p=%s) OS_INVALID_POINTER\n", path);
        return OS_INVALID_POINTER;
    }
    /*
    ** Check length
    */
    fullpath_len = strlen(path);
    if (fullpath_len >= OS_MAX_PATH_LEN)
    {
        OS_DebugPrintf(1, __func__, __LINE__, "OS_OpenCreate(p=%s) OS_FS_ERR_PATH_TOO_LONG\n", path);
        return OS_FS_ERR_PATH_TOO_LONG;
    }

    /*
    ** All valid full paths must start with a '/' character
    */
    if (path[0] != '/')
    {
        OS_DebugPrintf(1, __func__, __LINE__, "OS_OpenCreate(p=%s) OS_FS_ERR_PATH_INVALID\n", path);
        return OS_FS_ERR_PATH_INVALID;
    }

    mountpoint_len = strlen(WELL_KNOWN_MOUNT_POINT);
    if ( strncmp(WELL_KNOWN_MOUNT_POINT, path, mountpoint_len) != 0 ) {
        OS_DebugPrintf(1, __func__, __LINE__, "OS_OpenCreate(p=%s != %s) OS_ERROR\n", path, WELL_KNOWN_MOUNT_POINT);
        return OS_ERROR;
    }

    /* checks to see if there is a '/' somewhere in the path */
    basename_ptr = strrchr(path, '/');
    if (basename_ptr == NULL)
    {
        OS_DebugPrintf(1, __func__, __LINE__, "OS_OpenCreate(p=%s) OS_FS_ERR_PATH_INVALID\n", path);
        return OS_FS_ERR_PATH_INVALID;
    }

    path_len = (basename_ptr-path);
    if (path_len != mountpoint_len) {
        OS_DebugPrintf(1, __func__, __LINE__, "OS_OpenCreate(l=%u != %u) OS_ERROR\n", path_len, mountpoint_len);
        return OS_ERROR;
    }

    /* strrchr returns a pointer to the last '/' char, so we advance one char */
    basename_ptr = basename_ptr + 1;
    if (strlen(basename_ptr) >= OS_MAX_FILE_NAME)
    {
        OS_DebugPrintf(1, __func__, __LINE__, "OS_OpenCreate(n=%s) OS_FS_ERR_NAME_TOO_LONG\n", basename_ptr);
        return OS_FS_ERR_NAME_TOO_LONG;
    }

    OS_DebugPrintf(1, __func__, __LINE__, "OS_OpenCreate(p=%s) OS_ERROR\n", path);
    return OS_ERROR;
}
int32 xxxxxOS_close(osal_id_t filedes){
    OS_DebugPrintf(1, __func__, __LINE__, "OS_close() OS_ERROR\n");
    return OS_ERROR;
}
int32 xxxxxxOS_fsBlocksFree(const char *name){
    OS_DebugPrintf(1, __func__, __LINE__, "OS_fsBlocksFree() OS_ERROR\n");
    return OS_ERROR;
}
int32 xxxxOS_lseek(osal_id_t filedes, int32 offset, uint32 whence){
    OS_DebugPrintf(1, __func__, __LINE__, "OS_lseek() OS_ERROR\n");
    return OS_ERROR;
}
int32 xxxxxOS_read(osal_id_t filedes, void *buffer, size_t nbytes){
    OS_DebugPrintf(1, __func__, __LINE__, "OS_read() OS_ERROR\n");
    return OS_ERROR;
}
int32 xxxxxOS_remove(const char *path){
    OS_DebugPrintf(1, __func__, __LINE__, "OS_remove() OS_ERROR\n");
    return OS_ERROR;
}
int32 xxxxxOS_stat(const char *path, os_fstat_t *filestats){
    OS_DebugPrintf(1, __func__, __LINE__, "OS_stat() OS_ERROR\n");
    return OS_ERROR;
}
int32 xxxxxOS_write(osal_id_t filedes, const void *buffer, size_t nbytes){
    OS_DebugPrintf(1, __func__, __LINE__, "OS_write() OS_ERROR\n");
    return OS_ERROR;
}

#endif

/*----------------------------------------------------------------
 *
 * Function: OS_FileSysStatVolume_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_FileSysStatVolume_Impl(const OS_object_token_t *token, OS_statvfs_t *result)
{
    OS_DebugPrintf(1, __func__, __LINE__, "OS_ERR_NOT_IMPLEMENTED \n");    
    return OS_ERR_NOT_IMPLEMENTED;
} /* end OS_FileSysStatVolume_Impl */



/*----------------------------------------------------------------
 *
 * Function: OS_FileSysCheckVolume_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_FileSysCheckVolume_Impl(const OS_object_token_t *token, bool repair)
{
    OS_DebugPrintf(1, __func__, __LINE__, "OS_ERR_NOT_IMPLEMENTED \n");    
    return OS_ERR_NOT_IMPLEMENTED;
} /* end OS_FileSysCheckVolume_Impl */



/*----------------------------------------------------------------
 *
 * Function: OS_FileSysStopVolume_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_FileSysStopVolume_Impl(const OS_object_token_t *token)
{
    OS_DebugPrintf(1, __func__, __LINE__, "OS_ERR_NOT_IMPLEMENTED \n");
    return OS_ERR_NOT_IMPLEMENTED;

} /* end OS_FileSysStopVolume_Impl */


/*----------------------------------------------------------------
 *
 * Function: OS_FileRename_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_FileRename_Impl(const char *old_path, const char *new_path)
{
    OS_DebugPrintf(1, __func__, __LINE__, "OS_ERR_NOT_IMPLEMENTED \n");    
    return OS_ERR_NOT_IMPLEMENTED;

} /* end OS_FileRename_Impl */


/*----------------------------------------------------------------
 *
 * Function: OS_FileRemove_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_FileRemove_Impl(const char *local_path)
{
    OS_DebugPrintf(1, __func__, __LINE__, "OS_ERR_NOT_IMPLEMENTED \n");
    return OS_ERR_NOT_IMPLEMENTED;
} /* end OS_FileRemove_Impl */


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
    OS_DebugPrintf(1, __func__, __LINE__, "OS_ERR_NOT_IMPLEMENTED \n");
    return OS_ERR_NOT_IMPLEMENTED;

} /* end OS_GenericSeek_Impl */


/*----------------------------------------------------------------
 *
 * Function: OS_FileStat_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_FileStat_Impl(const char *local_path, os_fstat_t *FileStats)
{
    OS_DebugPrintf(1, __func__, __LINE__, "OS_ERR_NOT_IMPLEMENTED \n");
    return OS_ERR_NOT_IMPLEMENTED;

} /* end OS_FileStat_Impl */


/*----------------------------------------------------------------
 *
 * Function: OS_FileChmod_Impl
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_FileChmod_Impl(const char *local_path, uint32 access)
{
    OS_DebugPrintf(1, __func__, __LINE__, "OS_ERR_NOT_IMPLEMENTED \n");
    return OS_ERR_NOT_IMPLEMENTED;

} /* end OS_FileChmod_Impl */


#if 0

// @TODO @FIXME
int32 OS_FreeRTOS_FileSysAPI_Impl_Init(void){
    return OS_SUCCESS;
}
int32 OS_FreeRTOS_DirAPI_Impl_Init(void){
    return OS_SUCCESS;
}
int32 OS_FileAPI_Init(void){
    return OS_SUCCESS;
}
int32 OS_FileSysAPI_Init(void){
    return OS_SUCCESS;
}
int32 OS_DirAPI_Init(void){
    return OS_SUCCESS;
}
int32 OS_DirectoryClose(osal_id_t dir_id){
    return -1;
}
int32 OS_OpenCreate(osal_id_t *filedes, const char *path, int32 flags, int32 access){
    return -1;
}
int32 OS_close(osal_id_t filedes){
    return -1;
}
int32 OS_fsBlocksFree(const char *name){
    return -1;
}
int32 OS_initfs(char *address, const char *devname, const char *volname, size_t blocksize, osal_blockcount_t numblocks){
    return -1;
}
int32 OS_lseek(osal_id_t filedes, int32 offset, uint32 whence){
    return -1;
}
int32 OS_mkfs(char *address, const char *devname, const char *volname, size_t blocksize, osal_blockcount_t numblocks){
    return -1;
}
int32 OS_mount(const char *devname, const char *mountpoint){
    return -1;
}
int32 OS_read(osal_id_t filedes, void *buffer, size_t nbytes){
    return -1;
}
int32 OS_remove(const char *path){
    return -1;
}
int32 OS_rmfs(const char *devname){
    return -1;
}
int32 OS_stat(const char *path, os_fstat_t *filestats){
    return -1;
}
int32 OS_unmount(const char *mountpoint){
    return -1;
}
int32 OS_write(osal_id_t filedes, const void *buffer, size_t nbytes){
    return -1;
}
#endif