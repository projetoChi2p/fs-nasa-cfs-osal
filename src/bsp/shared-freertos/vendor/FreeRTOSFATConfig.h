/*
 * FreeRTOS+FAT
 * 
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright (C) 2021 Patrick Paul
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 * SPDX-License-Identifier: MIT
 *
 */

#ifndef __HEADER_FREERTOS_PLUS_FAT_CONFIG_H__
#define __HEADER_FREERTOS_PLUS_FAT_CONFIG_H__

// Send FF_PRINTF to BSP_Console_Write
//void OS_BSP_DEBUG_FF_PRINTF( const char* format, ...);
//#define FF_PRINTF OS_BSP_DEBUG_FF_PRINTF

#define ffconfigMIN_CLUSTERS_FAT16 32  // defaults to 4096

// Documentation:
// https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_FAT/Embedded_File_System_Configuration.html

#define ffconfigBYTE_ORDER pdFREERTOS_LITTLE_ENDIAN

#define ffconfigHAS_CWD 0  // no cwd, use full paths

#define ffconfigCWD_THREAD_LOCAL_INDEX 0

#define ffconfigLFN_SUPPORT 1

#define ffconfigINCLUDE_SHORT_NAME 0

#define ffconfigSHORTNAME_CASE 1

#define ffconfigUNICODE_UTF16_SUPPORT 0

#define ffconfigUNICODE_UTF8_SUPPORT 1

#define ffconfigFAT12_SUPPORT 0

#define ffconfigOPTIMISE_UNALIGNED_ACCESS 1

#define ffconfigCACHE_WRITE_THROUGH 1

#define ffconfigWRITE_BOTH_FATS 1

#define ffconfigWRITE_FREE_COUNT 1

#define ffconfigTIME_SUPPORT 0  // @FIXME consider enabling mtime, atime, ctime

#define ffconfigREMOVABLE_MEDIA 0

#define ffconfigMOUNT_FIND_FREE 1

#define ffconfigFSINFO_TRUSTED 0

#define ffconfigPATH_CACHE 1

#define ffconfigPATH_CACHE_DEPTH 4

#define ffconfigHASH_CACHE 0

#define ffconfigHASH_FUNCTION CRC8

#define ffconfigMKDIR_RECURSIVE 0
        
#define ffconfigMALLOC(size) pvPortMalloc(size)  // use same memory allocator as FreeRTOS

#define ffconfigFREE(size) vPortFree(size)  // use same memory free fn as FreeRTOS

#define ffconfig64_NUM_SUPPORT 0  // 0 = 32-bit

#define ffconfigMAX_PARTITIONS 1

#define ffconfigMAX_FILE_SYS 2

#define ffconfigDRIVER_BUSY_SLEEP_MS 100  // how long to wait when disk driver is busy

#define ffconfigFPRINTF_SUPPORT 0  // no ff_fprintf; provide own.

#define ffconfigFPRINTF_BUFFER_LENGTH 100

#define ffconfigINLINE_MEMORY_ACCESS 0

#define ffconfigFAT_CHECK 1

#define ffconfigMAX_FILENAME 32

#define ffconfigDEV_SUPPORT 1

#define ffconfigDEV_PATH "/"

#endif /* __HEADER_FREERTOS_PLUS_FAT_CONFIG_H__ */
