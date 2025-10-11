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

#include "ff.h"
#include "os-impl-filesys.h"


/****************************************************************************************
                                   Data Types
****************************************************************************************/


/****************************************************************************************
                                   GLOBAL DATA
 ***************************************************************************************/

#ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS

#else
/*  FreeRTOS FAT RAM Disk cache size.
 *  Must be multiple of sector size, which is 512 bytes, and at least twice as big.
 */
#define FREERTOS_FAT_SECTOR_SIZE 512
#define FREERTOS_FAT_RAMDISK_CACHE_MIN_SIZE (FREERTOS_FAT_SECTOR_SIZE*5)
#endif

/*
 * The implementation-specific file system state table.
 * This keeps record of the low level filesystems and drivers, e.g. Xilinx Memory Filesystem or FreeRTOS+FAT disks.
 */
OS_impl_filesys_internal_record_t OS_impl_filesys_table[OS_MAX_FILE_SYSTEMS];



/****************************************************************************************
                                Filesys API
 ***************************************************************************************/

//        d8888 8888888b. 8888888
//       d88888 888   Y88b  888
//      d88P888 888    888  888
//     d88P 888 888   d88P  888
//    d88P  888 8888888P"   888
//   d88P   888 888         888
//  d8888888888 888         888
// d88P     888 888       8888888

/* --------------------------------------------------------------------------------------
    Name: OS_FreeRTOS_FileSysAPI_Impl_Init

    Purpose: Filesystem API global initialization

    Returns: OS_SUCCESS if success
 ---------------------------------------------------------------------------------------*/
int32 OS_FreeRTOS_FileSysAPI_Impl_Init(void)
{
    /* clear the local filesys table */
    memset(OS_impl_filesys_table, 0, sizeof(OS_impl_filesys_table));

    #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
        //Check consistency between OSAL and Xilinx MFS limits
        if ( MFS_MAX_FILENAME_LENGTH < OS_MAX_FILE_NAME)
        {
            OS_DEBUG("Bad configuration: MFS_MAX_FILENAME_LENGTH %d < OS_MAX_FILE_NAME %d\n",
            MFS_MAX_FILENAME_LENGTH, OS_MAX_FILE_NAME);
            return OS_ERR_INVALID_SIZE;
        }

        if ( MFS_MAX_FILESYSTEM < OS_MAX_FILE_SYSTEMS)
        {
            OS_DEBUG("Bad configuration: MFS_MAX_FILESYSTEM %d < OS_MAX_FILE_SYSTEMS %d\n",
            MFS_MAX_FILESYSTEM, OS_MAX_FILE_SYSTEMS);
            return OS_ERR_INVALID_SIZE;
        }

        //Check consistency between OSAL and Xilinx MFS limits
        if (MFS_MAX_OPEN_FILES < (OS_MAX_NUM_OPEN_FILES+OS_MAX_NUM_OPEN_DIRS))
        {
            OS_DEBUG("Bad configuration: MFS_MAX_OPEN_FILES %d < OS_MAX_NUM_OPEN_FILES %d + OS_MAX_NUM_OPEN_DIRS %d\n",
            MFS_MAX_OPEN_FILES, OS_MAX_NUM_OPEN_FILES, OS_MAX_NUM_OPEN_DIRS);
            return OS_ERR_INVALID_SIZE;
        }

        mfs_init();
    #else
        /* Initialize known filesystems. FreeRTOS+FAT keeps a list of known FSs,
           up to ffconfigMAX_FILE_SYS.
           There will always be 1 FreeRTOS+FAT filesystem representing the system
           root "/", which will take one position in FreeRTOS+FAT known filesystems table.
        */
        //Check consistency between OSAL and FreeRTOS+FAT limits
        if ( ffconfigMAX_FILE_SYS < (OS_MAX_FILE_SYSTEMS+1) )
        {
            OS_DEBUG("Number os OSAL supported filesystems too large for current configuration (%d+1>%d).\n", OS_MAX_FILE_SYSTEMS, ffconfigMAX_FILE_SYS);
            return OS_ERROR;
        }

        FF_FS_Init();
    #endif

    return OS_SUCCESS;
}

//  .d8888b.  888                     888           d88P  .d8888b.  888
// d88P  Y88b 888                     888          d88P  d88P  Y88b 888
// Y88b.      888                     888         d88P   Y88b.      888
//  "Y888b.   888888  8888b.  888d888 888888     d88P     "Y888b.   888888 .d88b.  88888b.
//     "Y88b. 888        "88b 888P"   888       d88P         "Y88b. 888   d88""88b 888 "88b
//       "888 888    .d888888 888     888      d88P            "888 888   888  888 888  888
// Y88b  d88P Y88b.  888  888 888     Y88b.   d88P       Y88b  d88P Y88b. Y88..88P 888 d88P
//  "Y8888P"   "Y888 "Y888888 888      "Y888 d88P         "Y8888P"   "Y888 "Y88P"  88888P"
//                                                                                 888
//                                                                                 888
//                                                                                 888

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
    OS_filesys_internal_record_t*      filesys;
    OS_impl_filesys_internal_record_t* impl;
    int32                              return_code;

    impl  = OS_OBJECT_TABLE_GET(OS_impl_filesys_table, *token);
    filesys = OS_OBJECT_TABLE_GET(OS_filesys_table, *token);


    // From OS_FileSysAddFixedMap(), comes with
    // filesys->fstype = OS_FILESYS_TYPE_FS_BASED;
    // filesys->flags  = OS_FILESYS_FLAG_IS_FIXED;

    // From OS_FileSys_Initialize()
    // if vol_name ~ OS_FILESYS_RAMDISK_VOLNAME_PREFIX ("RAM"):
    // => filesys->fstype = OS_FILESYS_TYPE_VOLATILE_DISK;
    // => filesys->flags  = 0
    // otherwise:
    // => filesys->fstype = OS_FILESYS_TYPE_UNKNOWN;
    // => filesys->flags  = 0
    // We can change to OS_FILESYS_TYPE_NORMAL_DISK etc on start

    // From unit tests:
    // => filesys->fstype = any
    // => filesys->flags = ?

    memset(impl, 0, sizeof(*impl));

    /*
     * Take action based on the type of volume
     */
    switch(filesys->fstype)
    {
        case OS_FILESYS_TYPE_VOLATILE_DISK:
            #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
                if ( filesys->blocksize != MFS_BLOCK_DATA_SIZE )
                {
                    OS_DEBUG("Unsupported block size %u != %u\n", (unsigned int)(filesys->blocksize), (unsigned int)(MFS_BLOCK_DATA_SIZE));
                    return_code = OS_ERR_INVALID_SIZE;
                    break;
                }
                impl->device = -1;
            #else /* OS_FILESYSTEM_RAMDISK_IS_XILMFS */
                if ( filesys->blocksize != FREERTOS_FAT_SECTOR_SIZE )
                {
                    OS_DebugPrintf(1, __func__, __LINE__, "Unsupported block size %u\n", (unsigned int)(filesys->blocksize));
                    return_code = OS_ERR_INVALID_SIZE;
                    break;
                }
            #endif /* !OS_FILESYSTEM_RAMDISK_IS_XILMFS */

            if (filesys->address == NULL) {
                filesys->address = pvPortMalloc(filesys->numblocks * filesys->blocksize);
                if (filesys->address == NULL)
                {
                    OS_DEBUG("Memory allocation failed.\n");
                    return_code = OS_INVALID_POINTER;
                    break;
                }
                impl->fs_alloc_type  = OS_FILESYS_ALLOCATION_TYPE_DYNAMIC;
            }

            return_code = OS_SUCCESS;
            break;

        case OS_FILESYS_TYPE_FS_BASED:
        case OS_FILESYS_TYPE_NORMAL_DISK:
            /* No op */
            return_code = OS_SUCCESS;
            break;
        default:

            OS_DEBUG("v:%s d:%s m:%s v:%s a:%p bs:%lu blks:%lu flags:%lx t:%lx (%s)\n",
                filesys->volume_name,
                filesys->device_name,
                filesys->system_mountpt,
                filesys->virtual_mountpt,
                filesys->address,
                (unsigned long)filesys->blocksize,
                (unsigned long)filesys->numblocks,
                (unsigned long)filesys->flags,
                (unsigned long)filesys->fstype,

                (filesys->fstype==OS_FILESYS_TYPE_VOLATILE_DISK)?"Volatile":"?"

                // OS_FILESYS_TYPE_UNKNOWN = 0,   /**< Unspecified or unknown file system type */
                // OS_FILESYS_TYPE_FS_BASED,      /**< An emulated virtual file system that maps to another file system location */
                // OS_FILESYS_TYPE_NORMAL_DISK,   /**< A traditional disk drive or something that emulates one */
                // OS_FILESYS_TYPE_VOLATILE_DISK, /**< A temporary/volatile file system or RAM disk */
                // OS_FILESYS_TYPE_MTD,           /**< A "memory technology device" such as FLASH or EEPROM */
            );

            return_code = OS_ERR_NOT_IMPLEMENTED;
            break;
    }

    /*
     * If the operation was generally successful but a (real) FS
     * mount point was not supplied, then generate one now.
     *
     * Will use /volname as system mount point.
     */
    if (return_code == OS_SUCCESS && filesys->system_mountpt[0] == 0)
    {
        if (strlen(filesys->volume_name) >= ( sizeof(filesys->system_mountpt)-2) ) // 2 bytes for '/' and '\0'
        {
            return_code = OS_FS_ERR_PATH_TOO_LONG;
        }
        else
        {
            filesys->system_mountpt[0] = '/';
            strncpy(&filesys->system_mountpt[1], filesys->volume_name, sizeof(filesys->system_mountpt) - 2);
            filesys->system_mountpt[sizeof(filesys->system_mountpt) - 1] = 0;
            OS_DEBUG("OSAL: using mount point %s for volume %s\n", filesys->system_mountpt, filesys->volume_name);
        }
    }

    return return_code;
} /* end OS_FileSysStartVolume_Impl */


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
    OS_filesys_internal_record_t*      filesys;
    OS_impl_filesys_internal_record_t* impl;
    int32                              return_code;

    impl  = OS_OBJECT_TABLE_GET(OS_impl_filesys_table, *token);
    filesys = OS_OBJECT_TABLE_GET(OS_filesys_table, *token);


    // are all files in the filesystem closed?
    // are all dirs in the filesystem closed?
    // is the filesystem unmounted?

    /*
     * Take action based on the type of volume
     */
    switch(filesys->fstype)
    {
        case OS_FILESYS_TYPE_VOLATILE_DISK:

            if (filesys->address == NULL)
            {
                return_code = OS_INVALID_POINTER;
                break;
            }

            #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
                if ( impl->device >= 0 )
                {
                    OS_DEBUG("File system '%s' still mounted as %d?\n", filesys->system_mountpt, impl->device);
                    return_code = OS_ERR_FILE;
                    break;
                }
            #else /* !OS_FILESYSTEM_RAMDISK_IS_XILMFS */
                OS_DEBUG("OS_ERR_NOT_IMPLEMENTED \n");
                return_code = OS_ERR_NOT_IMPLEMENTED;
                break;
            #endif /* !OS_FILESYSTEM_RAMDISK_IS_XILMFS */

            if (impl->fs_alloc_type == OS_FILESYS_ALLOCATION_TYPE_DYNAMIC)
            {
                if (filesys->address == NULL)
                {
                    return_code = OS_INVALID_POINTER;
                    break;
                }

                vPortFree(filesys->address);
                impl->fs_alloc_type &= (~OS_FILESYS_ALLOCATION_TYPE_DYNAMIC);
                filesys->address = NULL;
            }
            else
            {
                // no action
            }

            return_code = OS_SUCCESS;
            break;

        case OS_FILESYS_TYPE_FS_BASED:
        case OS_FILESYS_TYPE_NORMAL_DISK:
            /* No op */
            return_code = OS_SUCCESS;
            break;
        default:

            OS_DEBUG("vol:%s d:%s m:%s vm:%s a:%p bs:%lu blks:%lu flags:%lx t:%lx (%s)\n",
                filesys->volume_name,
                filesys->device_name,
                filesys->system_mountpt,
                filesys->virtual_mountpt,
                filesys->address,
                (unsigned long)filesys->blocksize,
                (unsigned long)filesys->numblocks,
                (unsigned long)filesys->flags,
                (unsigned long)filesys->fstype,
                (filesys->fstype==OS_FILESYS_TYPE_VOLATILE_DISK)?"Volatile":"?"
                // OS_FILESYS_TYPE_UNKNOWN = 0,   /**< Unspecified or unknown file system type */
                // OS_FILESYS_TYPE_FS_BASED,      /**< An emulated virtual file system that maps to another file system location */
                // OS_FILESYS_TYPE_NORMAL_DISK,   /**< A traditional disk drive or something that emulates one */
                // OS_FILESYS_TYPE_VOLATILE_DISK, /**< A temporary/volatile file system or RAM disk */
                // OS_FILESYS_TYPE_MTD,           /**< A "memory technology device" such as FLASH or EEPROM */
            );

            return_code = OS_ERR_NOT_IMPLEMENTED;
            break;
    }

    return return_code;

} /* end OS_FileSysStopVolume_Impl */


//  .d8888b.  888                        888             d88P 8888888888                                      888
// d88P  Y88b 888                        888            d88P  888                                             888
// 888    888 888                        888           d88P   888                                             888
// 888        88888b.   .d88b.   .d8888b 888  888     d88P    8888888  .d88b.  888d888 88888b.d88b.   8888b.  888888
// 888        888 "88b d8P  Y8b d88P"    888 .88P    d88P     888     d88""88b 888P"   888 "888 "88b     "88b 888
// 888    888 888  888 88888888 888      888888K    d88P      888     888  888 888     888  888  888 .d888888 888
// Y88b  d88P 888  888 Y8b.     Y88b.    888 "88b  d88P       888     Y88..88P 888     888  888  888 888  888 Y88b.
//  "Y8888P"  888  888  "Y8888   "Y8888P 888  888 d88P        888      "Y88P"  888     888  888  888 "Y888888  "Y888

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
    UNUSED_ARGUMENT(token);
    UNUSED_ARGUMENT(repair);

    OS_DebugPrintf(1, __func__, __LINE__, "OS_ERR_NOT_IMPLEMENTED \n");
    return OS_ERR_NOT_IMPLEMENTED;
} /* end OS_FileSysCheckVolume_Impl */


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
    OS_filesys_internal_record_t*      filesys;
#ifndef OS_FILESYSTEM_RAMDISK_IS_XILMFS
    OS_impl_filesys_internal_record_t* impl;
#endif
    int32                              return_code;

    filesys = OS_OBJECT_TABLE_GET(OS_filesys_table, *token);
#ifndef OS_FILESYSTEM_RAMDISK_IS_XILMFS
    impl    = OS_OBJECT_TABLE_GET(OS_impl_filesys_table, *token);
#endif

    /*
     * Take action based on the type of volume
     */
    switch(filesys->fstype)
    {
        case OS_FILESYS_TYPE_VOLATILE_DISK:

            if (filesys->address == NULL)
            {
                return_code = OS_INVALID_POINTER;
                break;
            }

            #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
                int mfs_result;
                mfs_result = mfs_init_fs(
                    filesys->numblocks * filesys->blocksize, // number of bytes allocated or reserved for this file system
                    filesys->address,                        // starting address of the memory block
                    MFSINIT_NEW                              // creating empty read/write filesystem
                );

                if (mfs_result < 0)
                {
                    OS_DEBUG("mfs_init_fs(%s) failed: %d.\n", filesys->system_mountpt, mfs_result);
                    return_code = OS_ERR_FILE;
                    break;
                }

                /* For now, we are only formatting... */
                mfs_result = mfs_fs_close(mfs_result);
                if (mfs_result < 0)
                {
                    OS_DEBUG("mfs_fs_close(%s) failed: %d.\n", filesys->system_mountpt, mfs_result);
                    return_code = OS_ERR_FILE;
                    break;
                }

                return_code = OS_SUCCESS;
                break;

            #else /* !OS_FILESYSTEM_RAMDISK_IS_XILMFS */

                if ( filesys->system_mountpt[0] == 0 )
                {
                    OS_DEBUG("The system mount point should be a non-empty string.\n");
                    return_code = OS_FS_ERR_PATH_INVALID;
                    break;
                }

                // NOTE: FF_RAMDiskInit() always mount the RAM filesystem!

                // TODO FF_RAMDiskInit() always clear RAM, how to preserve filesystem between boot?
                impl->allocated_disk = FF_RAMDiskInit(
                    /*(char *)*/ filesys->system_mountpt, // Discarding const qualifier!
                    (uint8_t *) filesys->address,
                    filesys->numblocks,
                    FREERTOS_FAT_RAMDISK_CACHE_MIN_SIZE
                );

                if (impl->allocated_disk == NULL)
                {
                    OS_DEBUG("FF_RAMDiskInit() failed\n");
                    return_code = OS_INVALID_POINTER;
                    break;
                }

                if ( (impl->allocated_disk->pxIOManager->xPartition.ucType != FF_T_FAT12) &&
                    (impl->allocated_disk->pxIOManager->xPartition.ucType != FF_T_FAT16) )
                {
                    OS_DEBUG("FF_RAMDiskInit(%s) unexpected filesystem type: %d.\n",
                    filesys->device_name,
                    impl->allocated_disk->pxIOManager->xPartition.ucType);
                    return_code = OS_INVALID_POINTER;
                    break;
                }

                return_code = OS_SUCCESS;

                // name the volume
                strncpy(
                    impl->allocated_disk->pxIOManager->xPartition.pcVolumeLabel,
                    filesys->volume_name,
                    sizeof(impl->allocated_disk->pxIOManager->xPartition.pcVolumeLabel)-1
                );
                impl->allocated_disk->pxIOManager->xPartition.pcVolumeLabel[sizeof(impl->allocated_disk->pxIOManager->xPartition.pcVolumeLabel)-1] = 0;

            #endif /* !OS_FILESYSTEM_RAMDISK_IS_XILMFS */

            break;
        case OS_FILESYS_TYPE_FS_BASED:
            /* No op */
            return_code = OS_SUCCESS;
            break;
        default:

            OS_DEBUG("vol:%s d:%s m:%s vm:%s a:%p bs:%lu blks:%lu flags:%lx t:%lx (%s)\n",
                filesys->volume_name,
                filesys->device_name,
                filesys->system_mountpt,
                filesys->virtual_mountpt,
                filesys->address,
                (unsigned long)filesys->blocksize,
                (unsigned long)filesys->numblocks,
                (unsigned long)filesys->flags,
                (unsigned long)filesys->fstype,

                (filesys->fstype==OS_FILESYS_TYPE_VOLATILE_DISK)?"Volatile":"?"

                // OS_FILESYS_TYPE_UNKNOWN = 0,   /**< Unspecified or unknown file system type */
                // OS_FILESYS_TYPE_FS_BASED,      /**< An emulated virtual file system that maps to another file system location */
                // OS_FILESYS_TYPE_NORMAL_DISK,   /**< A traditional disk drive or something that emulates one */
                // OS_FILESYS_TYPE_VOLATILE_DISK, /**< A temporary/volatile file system or RAM disk */
                // OS_FILESYS_TYPE_MTD,           /**< A "memory technology device" such as FLASH or EEPROM */
            );

            return_code = OS_ERR_NOT_IMPLEMENTED;
            break;
    }

    return return_code;
} /* end OS_FileSysFormatVolume_Impl */


// 888b     d888                            888           d88P 888     888                                                   888
// 8888b   d8888                            888          d88P  888     888                                                   888
// 88888b.d88888                            888         d88P   888     888                                                   888
// 888Y88888P888  .d88b.  888  888 88888b.  888888     d88P    888     888 88888b.  88888b.d88b.   .d88b.  888  888 88888b.  888888
// 888 Y888P 888 d88""88b 888  888 888 "88b 888       d88P     888     888 888 "88b 888 "888 "88b d88""88b 888  888 888 "88b 888
// 888  Y8P  888 888  888 888  888 888  888 888      d88P      888     888 888  888 888  888  888 888  888 888  888 888  888 888
// 888   "   888 Y88..88P Y88b 888 888  888 Y88b.   d88P       Y88b. .d88P 888  888 888  888  888 Y88..88P Y88b 888 888  888 Y88b.
// 888       888  "Y88P"   "Y88888 888  888  "Y888 d88P         "Y88888P"  888  888 888  888  888  "Y88P"   "Y88888 888  888  "Y888

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

    OS_filesys_internal_record_t *      filesys;
    OS_impl_filesys_internal_record_t * impl;
    int32                               return_code;

    /* Used for Chan FatFs*/
    FRESULT result;

    impl    = OS_OBJECT_TABLE_GET(OS_impl_filesys_table, *token);
    filesys = OS_OBJECT_TABLE_GET(OS_filesys_table, *token);

    /*
     * Take action based on the type of volume
     */
    switch(filesys->fstype)
    {
        case OS_FILESYS_TYPE_VOLATILE_DISK:
            // Sanity check
            if (filesys->address == NULL)
            {
                OS_DEBUG("mount(%s) DEVICE NOT READY/FORMATTED\n", filesys->device_name);
                return_code = OS_INVALID_POINTER;
                break;
            }

            #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
                impl->device = mfs_init_fs(
                    filesys->numblocks * filesys->blocksize, // number of bytes allocated or reserved for this file system
                    filesys->address,                        // starting address of the memory block
                    MFSINIT_IMAGE                            // mounting a pre-formmated read/write filesystem
                );

                if (impl->device < 0)
                {
                    OS_DEBUG("mfs_init_fs(%s) failed: %d.\n", filesys->system_mountpt, impl->device);
                    return_code = OS_ERR_FILE;
                    break;
                }

                return_code = OS_SUCCESS;
                break;

            #else /* !OS_FILESYSTEM_RAMDISK_IS_XILMFS */
                /* No action: FreeRTOS+FAT FF_RAMDiskInit() left RAM filesystem mounted
                 * after formatting.
                 */
                /* This implementation relies on FreeRTOS+FAT stdio features,
                 * which keeps registry of filesystem path prefixes (device name or
                 * mount point) and select the respective I/O manager to handle
                 * files and directories.
                 */
                if (impl->allocated_disk == NULL)
                {
                    OS_DEBUG("mount(%s) DEVICE NOT READY/FORMATTED\n", filesys->device_name);
                    return_code = OS_INVALID_POINTER;
                    break;
                }

            #endif /* !OS_FILESYSTEM_RAMDISK_IS_XILMFS */

            return_code = OS_SUCCESS;
            break;

        case OS_FILESYS_TYPE_FS_BASED:
            result = f_mount(0, &impl->fatfs);

            if (result == FR_OK)
            {
                return_code = OS_SUCCESS;
            }
            else
            {
                OS_DEBUG("OSAL: Error mouting Chan FATFS ec: %x", return_code)
                return_code = OS_ERROR;
            }

            break;
        default:

            OS_DEBUG("vol:%s d:%s m:%s vm:%s a:%p bs:%lu blks:%lu flags:%lx t:%lx (%s)\n",
                filesys->volume_name,
                filesys->device_name,
                filesys->system_mountpt,
                filesys->virtual_mountpt,
                filesys->address,
                (unsigned long)filesys->blocksize,
                (unsigned long)filesys->numblocks,
                (unsigned long)filesys->flags,
                (unsigned long)filesys->fstype,

                (filesys->fstype==OS_FILESYS_TYPE_VOLATILE_DISK)?"Volatile":"?"

                // OS_FILESYS_TYPE_UNKNOWN = 0,   /**< Unspecified or unknown file system type */
                // OS_FILESYS_TYPE_FS_BASED,      /**< An emulated virtual file system that maps to another file system location */
                // OS_FILESYS_TYPE_NORMAL_DISK,   /**< A traditional disk drive or something that emulates one */
                // OS_FILESYS_TYPE_VOLATILE_DISK, /**< A temporary/volatile file system or RAM disk */
                // OS_FILESYS_TYPE_MTD,           /**< A "memory technology device" such as FLASH or EEPROM */
            );
            OS_DebugPrintf(1, __func__, __LINE__, "OS_ERR_NOT_IMPLEMENTED \n");
            return_code = OS_ERR_NOT_IMPLEMENTED;
            break;
    }

    return return_code;

} /* end OS_FileSysMountVolume_Impl */


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
    OS_filesys_internal_record_t *     filesys;
    OS_impl_filesys_internal_record_t* impl;
    int32                              return_code;

    /* Used for Chan FatFS*/
    FRESULT                            result;

    filesys = OS_OBJECT_TABLE_GET(OS_filesys_table, *token);
    impl  = OS_OBJECT_TABLE_GET(OS_impl_filesys_table, *token);

    /*
     * Take action based on the type of volume
     */
    switch(filesys->fstype)
    {
        case OS_FILESYS_TYPE_VOLATILE_DISK:
            // Sanity check
            if (filesys->address == NULL)
            {
                OS_DEBUG("mount(%s) DEVICE NOT READY/FORMATTED\n", filesys->device_name);
                return_code = OS_INVALID_POINTER;
                break;
            }

            #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
                int mfs_result;

                if (impl->device < 0)
                {
                    OS_DEBUG("bad device for '%s': %d.\n", filesys->system_mountpt, impl->device);
                    return_code = OS_ERR_FILE;
                    break;
                }

                mfs_result = mfs_fs_close(impl->device);
                if (mfs_result < 0)
                {
                    OS_DEBUG("mfs_fs_close(%s,%d) failed: %d.\n", filesys->system_mountpt, impl->device, mfs_result);
                    return_code = OS_ERR_FILE;
                    break;
                }
                impl->device = -1;

                return_code = OS_SUCCESS;
                break;

            #else /* !OS_FILESYSTEM_RAMDISK_IS_XILMFS */
                OS_DEBUG("OS_ERR_NOT_IMPLEMENTED \n");
                return_code = OS_ERR_NOT_IMPLEMENTED;
                break;
            #endif /* !OS_FILESYSTEM_RAMDISK_IS_XILMFS */

            return_code = OS_SUCCESS;
            break;

            case OS_FILESYS_TYPE_NORMAL_DISK:
                /* To unmount the Filesystem it need to pass NULL as a parameter to f_mount */
                result = f_mount(0, NULL);

                if (result != FR_OK)
                {
                    return_code = OS_ERROR;
                }
                else
                {
                    return_code = OS_SUCCESS;
                }

            break;
        default:

            OS_DEBUG("vol:%s d:%s m:%s vm:%s a:%p bs:%lu blks:%lu flags:%lx t:%lx (%s)\n",
                filesys->volume_name,
                filesys->device_name,
                filesys->system_mountpt,
                filesys->virtual_mountpt,
                filesys->address,
                (unsigned long)filesys->blocksize,
                (unsigned long)filesys->numblocks,
                (unsigned long)filesys->flags,
                (unsigned long)filesys->fstype,

                (filesys->fstype==OS_FILESYS_TYPE_VOLATILE_DISK)?"Volatile":"?"

                // OS_FILESYS_TYPE_UNKNOWN = 0,   /**< Unspecified or unknown file system type */
                // OS_FILESYS_TYPE_FS_BASED,      /**< An emulated virtual file system that maps to another file system location */
                // OS_FILESYS_TYPE_NORMAL_DISK,   /**< A traditional disk drive or something that emulates one */
                // OS_FILESYS_TYPE_VOLATILE_DISK, /**< A temporary/volatile file system or RAM disk */
                // OS_FILESYS_TYPE_MTD,           /**< A "memory technology device" such as FLASH or EEPROM */
            );
            OS_DebugPrintf(1, __func__, __LINE__, "OS_ERR_NOT_IMPLEMENTED \n");
            return_code = OS_ERR_NOT_IMPLEMENTED;
            break;
    }

    return return_code;
} /* end OS_FileSysUnmountVolume_Impl */


//  .d8888b.  888             888
// d88P  Y88b 888             888
// Y88b.      888             888
//  "Y888b.   888888  8888b.  888888
//     "Y88b. 888        "88b 888
//       "888 888    .d888888 888
// Y88b  d88P Y88b.  888  888 Y88b.
//  "Y8888P"   "Y888 "Y888888  "Y888

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

    OS_filesys_internal_record_t *filesys;
    OS_impl_filesys_internal_record_t *impl;
    osal_status_t return_code;

    /* Variables for volatile Filesystem */
    #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
    int mfs_status;
    int blocks_used;
    int blocks_free;
    #endif

    /* Used for Chan FatFs */
    FATFS   *fs;
    DWORD   free_clusters;
    DWORD   free_sectors;
    DWORD   total_sectors;
    FRESULT fs_result;

    filesys = OS_OBJECT_TABLE_GET(OS_filesys_table, *token);
    impl    = OS_OBJECT_TABLE_GET(OS_impl_filesys_table, *token);

    /*
     * Take action based on the type of volume
     */
    switch(filesys->fstype)
    {
        case OS_FILESYS_TYPE_VOLATILE_DISK:

            #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
                // In MFS, the number of blocks (filesys->numblocks) and block size
                // (filesys->blocksize) informed by the upper application layer are not
                // the actual values for the MFS filesystem.
                // An MFS block embeds metadata, including flags and indexes in the a double
                // linked list of blocks storing the same file. For instance, for a raw data
                // block of 128 bytes an MFS block has 148 bytes.
                // In the example above, the total RAM of size (filesys->numblocks * 128) is
                // repurposed in MFS blocks of 148 bytes, hence the actual data capacity is at
                // least 14% smaller than expected by the application. Additional overhead
                // comes from directory tree, taking other MFS blocks of 148 bytes.
                // Here, we opted to inform to the application layer the size of block storing
                // raw file data, which is the (filesys->blocksize).
                // This conveys the semantic that a file of N bytes will take (N/filesys->blocksize)
                // blocks to store raw data, and not (N/MFS block size)
                // Notwithstanding, the answer about number of blocks and free blocks will be the
                // effective number of MFS blocks.

                mfs_status = mfs_get_usage(impl->device, &blocks_used, & blocks_free);

                if ( mfs_status != MFS_SUCCESS )
                {
                    OS_DEBUG("mfs_get_usage(dev=%d) failed, result %d.\n", impl->device, mfs_status);
                    return_code = OS_ERROR;
                    break;
                }

                result->block_size   = OSAL_SIZE_C(filesys->blocksize);
                result->total_blocks = OSAL_BLOCKCOUNT_C( blocks_used+blocks_free );
                result->blocks_free  = OSAL_BLOCKCOUNT_C ( blocks_free );

            #else /* !OS_FILESYSTEM_RAMDISK_IS_XILMFS */

                if (impl->allocated_disk == NULL)
                {
                    OS_DEBUG("mount(%s) DEVICE NOT READY/FORMATTED\n", filesys->device_name);
                    return_code = OS_ERR_INCORRECT_OBJ_STATE;
                    break;
                }

                int blocks_free;

                // device_name was registered previlously through FF_RAMDiskInit()
                // now ff_diskfree() uses device_name with FF_FS_Find() to retrieve information
                blocks_free = ff_diskfree(filesys->system_mountpt, NULL /* pxSectorCount */);
                if ( stdioGET_ERRNO( ) != pdFREERTOS_ERRNO_NONE )
                {
                    OS_DEBUG("diskfree(v:%s d:%s m:%s v:%s): %s\n",
                        filesys->volume_name,
                        filesys->device_name,
                        filesys->system_mountpt,
                        filesys->virtual_mountpt,
                        strerror(stdioGET_ERRNO( )));
                    return_code = OS_ERROR;
                    break;
                }

                result->block_size   = OSAL_SIZE_C(filesys->blocksize);
                result->total_blocks = OSAL_BLOCKCOUNT_C(filesys->numblocks);
                result->blocks_free  = OSAL_BLOCKCOUNT_C ( blocks_free );

            #endif /* !OS_FILESYSTEM_RAMDISK_IS_XILMFS */

            return_code = OS_SUCCESS;
            break;
        case OS_FILESYS_TYPE_FS_BASED:
        case OS_FILESYS_TYPE_NORMAL_DISK:
            /* Get volume information and free clusters */
            fs_result = f_getfree("0:", &free_clusters, &fs);

            if (fs_result != FR_OK)
            {
                return_code = OS_ERROR;
            }
            else
            {
                /*
                * - Cluster: The unit for file allocation, composed of sectors.
                * - Sector:  The physical unit on disk.
                * - Block:   The logical unit for this application, equal to one sector.
                */

                total_sectors = (impl->fatfs.n_fatent - 2) * fs->csize;
                free_sectors = free_clusters * fs->csize;

                /* Block size is the Maximum sector size of the Filesystem */
                result->block_size = _MAX_SS;
                result->blocks_free = free_sectors;
                result->total_blocks = total_sectors;

                return_code = OS_SUCCESS;
            }

            break;
        default:

            OS_DEBUG("vol:%s d:%s m:%s vm:%s a:%p bs:%lu blks:%lu flags:%lx t:%lx (%s)\n",
                filesys->volume_name,
                filesys->device_name,
                filesys->system_mountpt,
                filesys->virtual_mountpt,
                filesys->address,
                (unsigned long)filesys->blocksize,
                (unsigned long)filesys->numblocks,
                (unsigned long)filesys->flags,
                (unsigned long)filesys->fstype,

                (filesys->fstype==OS_FILESYS_TYPE_VOLATILE_DISK)?"Volatile":"?"

                // OS_FILESYS_TYPE_UNKNOWN = 0,   /**< Unspecified or unknown file system type */
                // OS_FILESYS_TYPE_FS_BASED,      /**< An emulated virtual file system that maps to another file system location */
                // OS_FILESYS_TYPE_NORMAL_DISK,   /**< A traditional disk drive or something that emulates one */
                // OS_FILESYS_TYPE_VOLATILE_DISK, /**< A temporary/volatile file system or RAM disk */
                // OS_FILESYS_TYPE_MTD,           /**< A "memory technology device" such as FLASH or EEPROM */
            );

            return_code = OS_ERR_NOT_IMPLEMENTED;
            break;
    }

    return return_code;

} /* end OS_FileSysStatVolume_Impl */


// 8888888888 d8b 888
// 888        Y8P 888
// 888            888
// 8888888    888 888  .d88b.  .d8888b
// 888        888 888 d8P  Y8b 88K
// 888        888 888 88888888 "Y8888b.
// 888        888 888 Y8b.          X88
// 888        888 888  "Y8888   88888P'



/*----------------------------------------------------------------
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *           Checks if the filesys table index matches the "system_mountpt" field prefix.
 *           Function is Compatible with the Search object lookup routine
 *           This routine is based on OSAL OS_FileSys_FindVirtMountPoint()
 *
 *  Returns: true if the entry matches prefix, false if it does not match
 *
 *-----------------------------------------------------------------*/
static bool findMountPoint(void *ref, const OS_object_token_t *token, const OS_common_record_t *obj)
{
    OS_filesys_internal_record_t *filesys;
    const char *                  target = (const char *)ref;
    size_t                        mplen;

    UNUSED_ARGUMENT(obj);

    filesys = OS_OBJECT_TABLE_GET(OS_filesys_table, *token);

    if ((filesys->flags & OS_FILESYS_FLAG_IS_MOUNTED_VIRTUAL) == 0)
    {
        return false;
    }

    mplen = OS_strnlen(filesys->system_mountpt, sizeof(filesys->system_mountpt));

    /*
     * The system_mountpt member should be a substring of the search target.
     * If this matches a basic substring check then it may be match
     */
    if (mplen == 0 || mplen >= sizeof(filesys->system_mountpt) ||
        strncmp(target, filesys->system_mountpt, mplen) != 0)
    {
        /* not a substring, so not a match */
        return false;
    }

    /*
     * Confirm that the substring ends at either a directory separator
     * or the end of string  (so exact mount points also match).
     *
     * For instance consider a system_mountpt of /mnt/abc and searching
     * for target=/mnt/abcd - this should return false in that case.
     */
    return (target[mplen] == '/' || target[mplen] == 0);
}


/*----------------------------------------------------------------
 *
 *  Function: OS_FreeRTOS_TranslateLocalPath
 *
 *  Attention: This routine returns a locked token!
 *
 *  Purpose: Support FreeRTOS file systems
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_FreeRTOS_TranslateLocalPath(const char *LocalPath, OS_object_token_t *FileSystem, char *DevicePath)
{
    int32                         return_code;
    OS_filesys_internal_record_t *filesys;
    size_t LocalPathLen;
    size_t SysMountPointLen;
    size_t DevicePathLen;

    /* Check parameters */
    OS_CHECK_PATHNAME(LocalPath);
    OS_CHECK_POINTER(DevicePath);
    OS_CHECK_POINTER(FileSystem);

    /*
    ** Check length
    */
    LocalPathLen = OS_strnlen(LocalPath, OS_MAX_PATH_LEN);
    if (LocalPathLen >= OS_MAX_PATH_LEN)
    {
        return OS_FS_ERR_PATH_TOO_LONG;
    }

    /* Get a global lock. */
    return_code = OS_ObjectIdGetBySearch(
        OS_LOCK_MODE_GLOBAL, // could be OS_LOCK_MODE_EXCLUSIVE?
        OS_OBJECT_TYPE_OS_FILESYS,
        findMountPoint,
        (void *)LocalPath,
        FileSystem
        );

    if (return_code == OS_SUCCESS)
    {
        filesys = OS_OBJECT_TABLE_GET(OS_filesys_table, *FileSystem);
        SysMountPointLen = OS_strnlen(filesys->system_mountpt, sizeof(filesys->system_mountpt));

        if (filesys->fstype == OS_FILESYS_TYPE_VOLATILE_DISK)
        {
            // /RAM1
            // /RAM1/
            // /RAM1/abc
            if ( LocalPath[SysMountPointLen] == '\0' )
            {
                DevicePath[0] = '/';
                DevicePath[1] = '\0';
            }
            else
            {
                // findMountPoint() checks for local path for having a delimiter '/' immeditate to mount point prefix
                DevicePathLen = LocalPathLen - SysMountPointLen;

                memcpy(DevicePath, &LocalPath[SysMountPointLen], DevicePathLen);
                DevicePath[DevicePathLen + 2] = '\0';
            }
        }
        else if (filesys->fstype == OS_FILESYS_TYPE_FS_BASED)
        {
            // findMountPoint() checks for local path for having a delimiter '/' immeditate to mount point prefix
            // strcpy(DevicePath, "0:");
            // strcat(DevicePath, filesys->system_mountpt);
            strcpy(DevicePath, filesys->system_mountpt);
            strcat(DevicePath, &LocalPath[SysMountPointLen]);
        }
        else
        {
            return_code = OS_ERR_NOT_IMPLEMENTED;
        }

        //OS_ObjectIdRelease(&token);
    }
    else
    {
        return_code = OS_ERR_NAME_NOT_FOUND;
    }

    return return_code;
}


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
    OS_object_token_t filesys_token;
    OS_filesys_internal_record_t  *filesys;
    OS_impl_filesys_internal_record_t *filesys_impl;
    osal_status_t return_code;

    char device_path [OS_MAX_LOCAL_PATH_LEN];
    uint8 fstype;
    #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
    int device;
    #else
    int32_t iRcFat;
    FF_Stat_t xStat;
    #endif

    /* Used for Chan FatFs */
    FILINFO info;
    FRESULT fs_result;

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

                memset(&FileStats->FileTime, 0, sizeof(FileStats->FileTime));
                int mfs_status;
                int file_size;
                mfs_status = mfs_get_file_size(device, device_path, &file_size);
                if (mfs_status == 1)
                {
                    // it is a file
                    FileStats->FileSize = file_size;
                    FileStats->FileModeBits = ( OS_FILESTAT_MODE_WRITE |
                                                OS_FILESTAT_MODE_READ );
                }
                else if (mfs_status == 2)
                {
                    // it is a directory
                    FileStats->FileSize = 0;
                    FileStats->FileModeBits = ( OS_FILESTAT_MODE_WRITE |
                                                OS_FILESTAT_MODE_READ  |
                                                OS_FILESTAT_MODE_EXEC  |
                                                OS_FILESTAT_MODE_DIR );
                }
                else
                {
                    // not found or error
                    return_code = OS_ERROR;
                    break;
                }

            #else


                #if ( ffconfigTIME_SUPPORT != 0 )
                    // FreeRTOS+FAT time is fed by FreeRTOS_time() macro
                    // See also OS_SetLocalTime()
                    // FF_TimeStruct_t tmStruct;
                    // time_t secs = xStat.st_mtime;
                    // entry_time = OS_TimeFromTotalSeconds(?);
                    OS_DEBUG("OS_ERR_NOT_IMPLEMENTED \n");
                    return_code = OS_ERR_NOT_IMPLEMENTED;
                    break;
                #endif
                memset(&FileStats->FileTime, 0, sizeof(FileStats->FileTime));

                iRcFat = ff_stat( local_path, &xStat );
                if ( iRcFat == FF_ERR_NONE )
                {
                    FileStats->FileSize = xStat.st_size;

                    if ( ( xStat.st_mode & FF_IFDIR ) || ( xStat.st_mode & FF_IFREG ) )
                    {
                        FileStats->FileModeBits = ( OS_FILESTAT_MODE_WRITE | OS_FILESTAT_MODE_READ );
                    }
                    if ( xStat.st_mode & FF_IFDIR )
                    {
                        FileStats->FileModeBits |= (OS_FILESTAT_MODE_DIR | OS_FILESTAT_MODE_EXEC) ;
                    }
                }
                else if( iRcFat < 0 )
                {
                    // FreeRTOS+FAT fails with ENOENT 2 (No such file or directory) when stat on device root
                    // Here we check if it is device root and report as directory
                    FF_DirHandler_t xHandler;
                    memset(&xHandler, 0, sizeof(xHandler));
                    if( FF_FS_Find( local_path, &xHandler ) == pdFALSE )
                    {
                        OS_DEBUG("Failed FF_FS_Find() for '%s'. Result %ld\n", local_path, stdioGET_ERRNO() );
                        return_code = OS_ERR_FILE;
                        break;
                    }

                    if ( FF_Mounted( xHandler.pxManager ) )
                    {
                        if (
                            ( OS_strnlen(xHandler.pcPath, 2) == 1 && (strcmp(xHandler.pcPath, "/")==0) ) ||
                            ( OS_strnlen(xHandler.pcPath, 3) == 2 && (strcmp(xHandler.pcPath, "/.")==0) ) ||
                            ( OS_strnlen(xHandler.pcPath, 4) == 3 && (strcmp(xHandler.pcPath, "/..")==0) ) )
                        {
                            FileStats->FileModeBits = ( OS_FILESTAT_MODE_WRITE | OS_FILESTAT_MODE_READ |
                                                        OS_FILESTAT_MODE_DIR | OS_FILESTAT_MODE_EXEC );
                            FileStats->FileSize = 0;
                        }
                        else
                        {
                            OS_DEBUG("Failed ff_stat() for '%s'. Result %ld\n", local_path, stdioGET_ERRNO() );
                            return_code = OS_ERR_FILE;
                            break;
                        }
                    }
                    else
                    {
                        OS_DEBUG("Not a mounted device '%s'.\n", local_path);
                        return_code = OS_ERR_FILE;
                        break;
                    }
                }
                else
                {
                    OS_DEBUG("Failed ff_stat() for '%s'. Result %ld\n", local_path, stdioGET_ERRNO() );
                    return_code = OS_ERR_NAME_NOT_FOUND;
                    break;
                }

            #endif

            return_code = OS_SUCCESS;
            break;
        case OS_FILESYS_TYPE_FS_BASED:
        case OS_FILESYS_TYPE_NORMAL_DISK:
            fs_result = f_stat(device_path, &info);

            if (fs_result == FR_OK)
            {
                FileStats->FileSize = info.fsize;
                FileStats->FileTime.ticks = info.ftime;

                if (info.fattrib & AM_DIR)
                {
                    FileStats->FileModeBits |= OS_FILESTAT_MODE_DIR;
                }
                if (info.fattrib & AM_RDO)
                {
                    FileStats->FileModeBits |= OS_FILESTAT_MODE_READ;
                }
                if (info.fattrib & AM_ARC)
                {
                    FileStats->FileModeBits |= OS_FILESTAT_MODE_WRITE;
                }

                return_code = OS_SUCCESS;
            }
            else
            {
                return_code = OS_ERROR;
            }

            break;
        default:
            OS_DEBUG("OS_ERR_NOT_IMPLEMENTED \n");
            return_code = OS_ERR_NOT_IMPLEMENTED;
    }

    return return_code;

} /* end OS_FileStat_Impl */


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
    OS_object_token_t filesys_token;
    OS_filesys_internal_record_t  *filesys;
    OS_impl_filesys_internal_record_t *filesys_impl;
    osal_status_t return_code;

    char device_path [OS_MAX_LOCAL_PATH_LEN];
    uint8 fstype;
    #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
    int device;
    int mfs_result;
    #endif

    /* Used for Chan FatFs */
    FRESULT result;

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
                mfs_result = mfs_delete_file(device, device_path);
                if ( mfs_result != MFS_SUCCESS )
                {
                    return_code = OS_ERROR;
                    break;
                }
                return_code = OS_SUCCESS;
                break;
            #else
                OS_DEBUG("OS_ERR_NOT_IMPLEMENTED \n");
                return_code = OS_ERR_NOT_IMPLEMENTED;
                break;
            #endif

            return_code = OS_SUCCESS;
            break;
        case OS_FILESYS_TYPE_FS_BASED:
        case OS_FILESYS_TYPE_NORMAL_DISK:
            result = f_unlink(device_path);

            if (result == FR_OK)
            {
                return_code = OS_SUCCESS;
            }
            else
            {
                return_code = OS_ERROR;
            }

            break;
        default:
            OS_DEBUG("OS_ERR_NOT_IMPLEMENTED \n");
            return_code = OS_ERR_NOT_IMPLEMENTED;
    }

    return return_code;

} /* end OS_FileRemove_Impl */



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

    OS_object_token_t filesys_token;
    OS_filesys_internal_record_t  *filesys;
    OS_impl_filesys_internal_record_t *filesys_impl;
    osal_status_t return_code;

    /* Used for Chan FatFs*/
    FRESULT fs_result;

    unsigned long old_fs_id; // see OS_ObjectIdToInteger()
    unsigned long new_fs_id; // see OS_ObjectIdToInteger()

    char device_path_old [OS_MAX_LOCAL_PATH_LEN];
    char device_path_new [OS_MAX_LOCAL_PATH_LEN];
    uint8 fstype;
    #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
    int device;
    int mfs_result;
    #endif

    /* First translate old name */
    return_code = OS_FreeRTOS_TranslateLocalPath(old_path, &filesys_token, device_path_old);
    if (return_code != OS_SUCCESS)
    {
        return return_code;
    }

    old_fs_id = OS_ObjectIdToInteger(OS_ObjectIdFromToken(&filesys_token));
    filesys = OS_OBJECT_TABLE_GET(OS_filesys_table, filesys_token);
    filesys_impl = OS_OBJECT_TABLE_GET(OS_impl_filesys_table, filesys_token);

    fstype = filesys->fstype;
    #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
    device = filesys_impl->device;
    #endif
    OS_ObjectIdRelease(&filesys_token);

    /* Now translate new name */
    return_code = OS_FreeRTOS_TranslateLocalPath(new_path, &filesys_token, device_path_new);
    if (return_code != OS_SUCCESS)
    {
        return return_code;
    }

    new_fs_id = OS_ObjectIdToInteger(OS_ObjectIdFromToken(&filesys_token));

    OS_ObjectIdRelease(&filesys_token);

    if ( old_fs_id != new_fs_id )
    {
        return OS_ERROR;
    }

    /*
     * Take action based on the type of volume
     */
    switch(fstype)
    {
        case OS_FILESYS_TYPE_VOLATILE_DISK:
            #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
                // MFS uses path relative to device root

                mfs_result = mfs_rename_file(device, device_path_old, device_path_new);
                if ( mfs_result != MFS_SUCCESS )
                {
                    return_code = OS_ERROR;
                    break;
                }
                return_code = OS_SUCCESS;
                break;
            #else
                OS_DEBUG("OS_ERR_NOT_IMPLEMENTED \n");
                return_code = OS_ERR_NOT_IMPLEMENTED;
                break;
            #endif

            return_code = OS_SUCCESS;
            break;
        case OS_FILESYS_TYPE_FS_BASED:
        case OS_FILESYS_TYPE_NORMAL_DISK:
            fs_result = f_rename(device_path_old, device_path_new);

            if (fs_result == FR_OK)
            {
                return_code = OS_SUCCESS;
            }
            else
            {
                return_code = OS_ERROR;
            }

            break;
        default:
            OS_DEBUG("OS_ERR_NOT_IMPLEMENTED \n");
            return_code = OS_ERR_NOT_IMPLEMENTED;
    }

    return return_code;

} /* end OS_FileRename_Impl */




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
    UNUSED_ARGUMENT(local_path);
    UNUSED_ARGUMENT(access);

    OS_DebugPrintf(1, __func__, __LINE__, "OS_ERR_NOT_IMPLEMENTED \n");
    return OS_ERR_NOT_IMPLEMENTED;

} /* end OS_FileChmod_Impl */

