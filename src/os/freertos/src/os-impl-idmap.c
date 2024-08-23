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

#include <os-shared-globaldefs.h>

#include "os-freertos.h"

// globals
SemaphoreHandle_t OS_task_table_sem;
SemaphoreHandle_t OS_queue_table_sem;
SemaphoreHandle_t OS_bin_sem_table_sem;
SemaphoreHandle_t OS_mutex_table_sem;
SemaphoreHandle_t OS_count_sem_table_sem;
SemaphoreHandle_t OS_stream_table_mut;
SemaphoreHandle_t OS_dir_table_mut;
SemaphoreHandle_t OS_timebase_table_mut;
SemaphoreHandle_t OS_timecb_table_mut;
SemaphoreHandle_t OS_module_table_mut;
SemaphoreHandle_t OS_filesys_table_mut;
SemaphoreHandle_t OS_console_mut;

static SemaphoreHandle_t *const MUTEX_TABLE[] = {
    [OS_OBJECT_TYPE_UNDEFINED]   = NULL,
    [OS_OBJECT_TYPE_OS_TASK]     = &OS_task_table_sem,
    [OS_OBJECT_TYPE_OS_QUEUE]    = &OS_queue_table_sem,
    [OS_OBJECT_TYPE_OS_COUNTSEM] = &OS_count_sem_table_sem,
    [OS_OBJECT_TYPE_OS_BINSEM]   = &OS_bin_sem_table_sem,
    [OS_OBJECT_TYPE_OS_MUTEX]    = &OS_mutex_table_sem,
    [OS_OBJECT_TYPE_OS_STREAM]   = &OS_stream_table_mut,
    [OS_OBJECT_TYPE_OS_DIR]      = &OS_dir_table_mut,
    [OS_OBJECT_TYPE_OS_TIMEBASE] = &OS_timebase_table_mut,
    [OS_OBJECT_TYPE_OS_TIMECB]   = &OS_timecb_table_mut,
    [OS_OBJECT_TYPE_OS_MODULE]   = &OS_module_table_mut,
    [OS_OBJECT_TYPE_OS_FILESYS]  = &OS_filesys_table_mut,
    [OS_OBJECT_TYPE_OS_CONSOLE]  = &OS_console_mut,
};

enum
{
    MUTEX_TABLE_SIZE = (sizeof(MUTEX_TABLE) / sizeof(MUTEX_TABLE[0]))
};

/*----------------------------------------------------------------
    Function: OS_Lock_Global_Impl
 ------------------------------------------------------------------*/
int32 OS_Lock_Global_Impl(osal_objtype_t idtype){
    SemaphoreHandle_t *mut;

    if(idtype < MUTEX_TABLE_SIZE){
        mut = MUTEX_TABLE[idtype];
    }else{
        mut = NULL;
    }

    if(mut == NULL){
        return OS_ERROR;
    }

    if(xSemaphoreTake(*mut, portMAX_DELAY) != pdTRUE){
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
    Function: OS_Unlock_Global_Impl
 ------------------------------------------------------------------*/
int32 OS_Unlock_Global_Impl(osal_objtype_t idtype){
    SemaphoreHandle_t *mut;

    if(idtype < MUTEX_TABLE_SIZE){
        mut = MUTEX_TABLE[idtype];
    }else{
        mut = NULL;
    }

    if(mut == NULL){
        return OS_ERROR;
    }

    if(xSemaphoreGive(*mut) != pdTRUE){
        return OS_ERROR;
    }

    return OS_SUCCESS;
}


/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
void OS_WaitForStateChange_Impl(osal_objtype_t idtype, uint32 attempts)
{
    TickType_t wait_ticks;

    if (attempts <= 10)
    {
        wait_ticks = attempts * attempts;
    }
    else
    {
        wait_ticks = 100;
    }

    OS_Unlock_Global_Impl(idtype);
    vTaskDelay(wait_ticks);
    OS_Lock_Global_Impl(idtype);
}



/*---------------------------------------------------------------------------------------
    Name: OS_FreeRTOS_TableMutex_Init
---------------------------------------------------------------------------------------*/
int32 OS_FreeRTOS_TableMutex_Init(osal_objtype_t idtype)
{
    int32 return_code = OS_SUCCESS;

    if(idtype < MUTEX_TABLE_SIZE && MUTEX_TABLE[idtype] != NULL){
        // @FYI Consider using xSemaphoreCreateMutexStatic to define specific RAM address
        // @FYI Consider using Binary Semaphore in place of Mutex if interrupts will be able to access mutex
        //      See: https://www.freertos.org/Embedded-RTOS-Binary-Semaphores.html

        // Create mutex
        *MUTEX_TABLE[idtype] = xSemaphoreCreateMutex();
        if(*MUTEX_TABLE[idtype] == NULL){
            OS_DEBUG("Error: xSemaphoreCreateMutex failed: %d\n", idtype);
            return OS_ERROR;
        }
        // Take the mutex
        if(xSemaphoreTake(*MUTEX_TABLE[idtype], (TickType_t) 0) == pdTRUE){
            // Release the mutex
            if(xSemaphoreGive(*MUTEX_TABLE[idtype]) != pdTRUE){
                return OS_ERROR;
            }
        }else {
            return OS_ERROR;
        }
    }

    return return_code;
}
