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
 * \file     os-impl-io.h
 * \ingroup  freertos
 * \author   joseph.p.hickey@nasa.gov
 * \author   Patrick Paul (https://github.com/pztrick)
 * \author   Fabio Benevenuti (UFRGS)
 *
 */

#ifndef INCLUDE_OS_IMPL_IO_H_
#define INCLUDE_OS_IMPL_IO_H_

#include <osconfig.h>


#ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
#else
/* freertos-plus-fat */
#include "portable/common/ff_ramdisk.h"
#include "include/ff_stdio.h"
#include "include/ff_headers.h"
#endif

typedef struct
{
    uint8_t fstype;
#ifdef OS_FILESYSTEM_RAMDISK_IS_XILMFS
    int device; // Xilinx MFS device id
    int fd;     // Xilinx MFS directory fd
#else
    FF_FILE *pxFile; // FreeRTOS+FAT File handle
#endif
} OS_impl_file_internal_record_t;

/*
 * The global file handle table.
 *
 * This table is shared across multiple units (files, sockets, etc) and they will share
 * the same file handle table from the basic file I/O.
 */
extern OS_impl_file_internal_record_t OS_impl_filehandle_table[OS_MAX_NUM_OPEN_FILES];

#endif /* INCLUDE_OS_IMPL_IO_H_ */
