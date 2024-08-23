/*
 * Copyright 2021 Patrick Paul
 * SPDX-License-Identifier: MIT-0
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "osapi.h"
#include "osapi-common.h"
#include "os-shared-common.h"

#include "generic_freertos_bsp_internal.h"
#include "bsp-impl.h"


#include "app_helpers.h"

char os_bsp_debug_printf_buffer[256];
#define OS_BSP_DEBUG_PRINTF_BUFFER_SIZE (sizeof(os_bsp_debug_printf_buffer)/sizeof(os_bsp_debug_printf_buffer[0]))


// OS_BSP_ConsoleOutput_Impl() is called from 
// OS_ConsoleOutput_Impl() inside OS_BSP_Lock_Impl()/OS_BSP_Unlock_Impl() block
void OS_BSP_ConsoleOutput_Impl(const char *Str, size_t DataLen)
{
    HLP_vConsolePrintBytesBspLocked((uint8_t *)Str, DataLen);
}

void OS_BSP_ConsoleSetMode_Impl(uint32 ModeBits)
{
    // no-op
}


// @ TODO replace OS_BSP_FF_PRINTF() with OS_printf()
void OS_BSP_DEBUG_FF_PRINTF(const char *Format, ...)
{
    va_list va;

    OS_BSP_Lock_Impl();

    va_start(va, Format);
    vsnprintf(os_bsp_debug_printf_buffer, OS_BSP_DEBUG_PRINTF_BUFFER_SIZE-1, Format, va);
    va_end(va);
    os_bsp_debug_printf_buffer[OS_BSP_DEBUG_PRINTF_BUFFER_SIZE-1] = '\0';
    OS_BSP_ConsoleOutput_Impl(os_bsp_debug_printf_buffer, strlen(os_bsp_debug_printf_buffer));

    OS_BSP_Unlock_Impl();
}

// FBV 2024-01-04 As per osal/src/bsp/generic-freertos/build_options.cmake, osapi-debug.c is explicitly discarded
// @ TODO replace direct call to OS_BSP_ConsoleOutput_Impl() with OS_printf()
/*
void OS_DebugPrintf(uint32 Level, const char *Func, uint32 Line, const char *Format, ...)
{
    va_list va;
    if(OS_SharedGlobalVars.DebugLevel >= Level){

        OS_BSP_Lock_Impl();

        snprintf(os_bsp_debug_printf_buffer, OS_BSP_DEBUG_PRINTF_BUFFER_SIZE-1, "%s():%lu:", Func, (unsigned long)Line);
        os_bsp_debug_printf_buffer[OS_BSP_DEBUG_PRINTF_BUFFER_SIZE-1] = '\0';
        // OS_BSP_ConsoleOutput_Impl(os_bsp_debug_printf_buffer, strlen(os_bsp_debug_printf_buffer));
        OS_printf("%s", os_bsp_debug_printf_buffer);
        
        va_start(va, Format);
        vsnprintf(os_bsp_debug_printf_buffer, OS_BSP_DEBUG_PRINTF_BUFFER_SIZE-1, Format, va);
        va_end(va);
        os_bsp_debug_printf_buffer[OS_BSP_DEBUG_PRINTF_BUFFER_SIZE-1] = '\0';
        // OS_BSP_ConsoleOutput_Impl(os_bsp_debug_printf_buffer, strlen(os_bsp_debug_printf_buffer));
        OS_printf("%s\n", os_bsp_debug_printf_buffer);

        OS_BSP_Unlock_Impl();
    }
}
*/

