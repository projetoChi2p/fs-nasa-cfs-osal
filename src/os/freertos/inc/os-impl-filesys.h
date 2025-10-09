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
 * \file     os-impl-files.h
 * \ingroup  freertos
 * \author   joseph.p.hickey@nasa.gov
 * \author   Patrick Paul (https://github.com/pztrick)
 * \author   Fabio Benevenuti (UFRGS)
 */

#ifndef INCLUDE_OS_IMPL_FILESSYS_H_
#define INCLUDE_OS_IMPL_FILESSYS_H_


#include "osconfig.h"



#define OS_FILESYS_ALLOCATION_TYPE_STATIC   1
#define OS_FILESYS_ALLOCATION_TYPE_DYNAMIC  2


typedef struct
{
    /* Used to know if the allocation of the File System space is given by the application
     *   or it needs to be allocated.
    */
    int  fs_alloc_type;

    #ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
        int device; // Xilinx MFS device id (may be negative on failure)
    #else
        FF_Disk_t *  allocated_disk;
    #endif

    FATFS fatfs;
    DIR dir;

} OS_impl_filesys_internal_record_t;

/*
 * The filesystems handle table.
 */
extern OS_impl_filesys_internal_record_t OS_impl_filesys_table[OS_MAX_FILE_SYSTEMS];

#endif /* INCLUDE_OS_IMPL_FILESSYS_H_ */
