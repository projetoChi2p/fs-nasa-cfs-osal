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
 * \file     os-impl-dirs.h
 * \ingroup  freertos
 * \author   joseph.p.hickey@nasa.gov
 * \author   Patrick Paul (https://github.com/pztrick)
 * \author   Fabio Benevenuti (UFRGS)
 */

#ifndef OS_IMPL_DIRS_H
#define OS_IMPL_DIRS_H

#include "osconfig.h"

typedef struct
{
    uint8_t         fstype;
    char            device_path[OS_MAX_LOCAL_PATH_LEN]; // Path being browsed
#ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
    int             device; // Xilinx MFS device id
    int             fd;     // Xilinx MFS directory fd
#else
    uint8_t         flags;
    FF_FindData_t * pxFindStruct; // Structure supporting FreeRTOS+FAT directory browsing.
#endif

#ifdef OS_FILESYSTEM_NON_VOLATILES_IS_FATFS
    /* Used for Chan FatFs*/
    DIR dir;
#endif

} OS_impl_dir_internal_record_t;

/*
 * The directory handle table.
 */
extern OS_impl_dir_internal_record_t OS_impl_dir_table[OS_MAX_NUM_OPEN_DIRS];

#endif /* OS_IMPL_DIRS_H */
