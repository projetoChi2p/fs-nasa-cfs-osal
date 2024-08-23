/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 *
 *
 * Copyright (c) 2024 Universidade Federal do Rio Grande do Sul (UFRGS)
 *
 *
 * Copyright (c) 2021 Patrick Paul
 * SPDX-License-Identifier: MIT-0
 *
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * \file
 * \ingroup  freertos
 * \author   joseph.p.hickey@nasa.gov
 * \author   Patrick Paul (https://github.com/pztrick)
 * \author   Fabio Benevenuti (UFRGS)
 *
 */


#include <os-shared-globaldefs.h>
#include "os-impl-timebase.h"
#include "os-impl-mutex.h"
#include "os-freertos.h"

/****************************************************************************************
                                     GLOBALS
 ***************************************************************************************/

OS_impl_timebase_internal_record_t OS_impl_timebase_table[OS_MAX_TIMEBASES];

/****************************************************************************************
                                INTERNAL FUNCTIONS
 ***************************************************************************************/


/*----------------------------------------------------------------
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *           A callback to FreeRTOS timer, which in turn
 *           posts a semaphore so the user task can execute
 *
 *-----------------------------------------------------------------*/
static void OS_FreeRTOS_TimeBase_TimerCallback(TimerHandle_t xInternalHandle)
{
    OS_VoidPtrValueWrapper_t            timebase_id_variant;
    OS_object_token_t                   token;
    OS_impl_timebase_internal_record_t *local;
    osal_id_t                           timebase_id;
    configASSERT( xInternalHandle != NULL );

    memset(&timebase_id_variant, 0, sizeof(timebase_id_variant));
    timebase_id_variant.opaque_arg = pvTimerGetTimerID(xInternalHandle);
    timebase_id = timebase_id_variant.id;

    if (OS_ObjectIdGetById(OS_LOCK_MODE_NONE, OS_OBJECT_TYPE_OS_TIMEBASE, timebase_id, &token) == OS_SUCCESS)
    {
        local = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, token);

        /* Feed user task
         */
        if(xSemaphoreGive(local->tick_binary_sem) != pdTRUE){
            OS_DEBUG("OS_BinSemGive_Impl xSemaphoreGive error\n");
            //return OS_ERROR;
        }

    }
}

/*----------------------------------------------------------------
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *  This function must have the same interface as an external sync
 *  function, which is OS_TimerSync_t.
 *-----------------------------------------------------------------*/
static uint32 OS_FreeRTOS_TimeBase_SemWaitImpl(osal_id_t timebase_id)
{
    OS_object_token_t                   token;
    OS_impl_timebase_internal_record_t *impl;
    uint32                              tick_time_usecs;

    /* The returned tick_time should be nonzero.  If the sync function
     * returns zero, then it means something went wrong and it is not
     * known how many ticks have elapsed.
     */
    tick_time_usecs = 0;

    if (OS_ObjectIdGetById(OS_LOCK_MODE_NONE, OS_OBJECT_TYPE_OS_TIMEBASE, timebase_id, &token) == OS_SUCCESS)
    {
        impl = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, token);

        while (xSemaphoreTake(impl->tick_binary_sem, portMAX_DELAY) != pdTRUE){
            //return OS_ERROR;
        }
        /*
         * Pend for the tick arrival
         */
        //if (ret != 0)
        //{
        //    /*
        //     * the sigwait call failed.
        //     * returning 0 will cause the process to repeat.
        //     */
        //}

        /*
         * Determine how long this tick was.
         * Note that there are plenty of ways this become wrong if the timer
         * is reset right around the time a tick comes in.  However, it is
         * impossible to guarantee the behavior of a reset if the timer is running.
         * (This is not an expected use-case anyway; the timer should be set and forget)
         */
        if (impl->reset_first_fired == 0)
        {
            tick_time_usecs = impl->configured_interval_time_usecs;
        }
        else
        {
            /*
             * Reset the timer and make it auto-reload, but only if first fire and an interval was selected.
             * We create dormant timer without auto-reload because we don't known start and interval times, which may be different, upon creation.
             * During timer set we keep without auto-reload and set start time as period.
             * Finally, here, after start time, we set the interval time as period and enable auto-reload.
             * 
             * There is two sources of delay/uncertainty in this transition from start time to interval time:
             * - First, there is a delay between the instant when OS_FreeRTOS_TimeBase_TimerCallback() and xSemaphoreGive() and
             *   the instant this OS_FreeRTOS_TimeBase_SemWaitImpl() passed xSemaphoreTake().
             * - Second, xTimerChangePeriod() only enqueues the change operation, which will be effective only later
             *   when FreeRTOS timer prvTimerTask() calls prvProcessReceivedCommands().
             */
            if (impl->interval_time_ticks > 0)
            {
                vTimerSetReloadMode(impl->timer_handle, pdTRUE);
                if ( xTimerChangePeriod(impl->timer_handle, impl->interval_time_ticks, portMAX_DELAY) != pdPASS ) {
                    OS_DEBUG("ERROR: xTimerChangePeriod().\n")
                }
            }

            tick_time_usecs = impl->configured_start_time_usecs;
            impl->reset_first_fired = 0;
        }
    }

    return tick_time_usecs;
}

/*----------------------------------------------------------------
 *
 *  Purpose:  Convert Microseconds to a number of ticks.
 *
 *-----------------------------------------------------------------*/
void OS_FreeRTOS_UsecsToTicks(uint32 usecs, TickType_t *ticks)
{
    uint32 result;

    result = usecs / (1000000/configTICK_RATE_HZ);

    if (result == 0)
    {
        result = 1;
    }

    *ticks = (TickType_t)result;
}


/****************************************************************************************
                      Entry point for helper thread
****************************************************************************************/

/*----------------------------------------------------------------
 *
 *  Purpose: Local helper routine, not part of OSAL API.
 *
 *-----------------------------------------------------------------*/
static void OS_FreeRTOS_TimeBaseTaskEntryPoint(void *pvParameters)
{
    OS_VoidPtrValueWrapper_t            timebase_id_variant;
    osal_id_t                           timebase_id;

    configASSERT( pvParameters != NULL );

    memset(&timebase_id_variant, 0, sizeof(timebase_id_variant));
    timebase_id_variant.opaque_arg = pvParameters;
    timebase_id = timebase_id_variant.id;

    OS_TimeBase_CallbackThread(timebase_id);

}


/****************************************************************************************
                                INITIALIZATION FUNCTION
 ***************************************************************************************/

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_FreeRTOS_TimeBaseAPI_Impl_Init(void)
{
    if( (INCLUDE_vTaskDelay != 1) || (!configUSE_TIMERS) ) {
        OS_DEBUG("ERROR: this implementation requires FreeRTOSConfig.h INCLUDE_vTaskDelay, configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS to be enabled.\n")
        return OS_ERROR;
    }    

    OS_SharedGlobalVars.TicksPerSecond = (int32) configTICK_RATE_HZ;

    OS_SharedGlobalVars.MicroSecPerTick = 1000000 / configTICK_RATE_HZ;

    return OS_SUCCESS;
} 

/****************************************************************************************
                                   Time Base API
 ***************************************************************************************/


/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
void OS_TimeBaseLock_Impl(const OS_object_token_t *token)
{
    OS_impl_timebase_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);

    /* A block time of portMAX_DELAY can be used to block indefinitely */
    if(xSemaphoreTake(impl->handler_mutex, portMAX_DELAY) != pdTRUE){
        OS_DEBUG("xSemaphoreTake(handler_mutex) failed.\n");
    }

    return;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
void OS_TimeBaseUnlock_Impl(const OS_object_token_t *token)
{
    OS_impl_timebase_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);

    if(xSemaphoreGive(impl->handler_mutex) != pdTRUE){
        OS_DEBUG("xSemaphoreGive(handler_mutex) failed.\n");
    }

    return;
}


/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TimeBaseGetInfo_Impl(const OS_object_token_t *token, OS_timebase_prop_t *timer_prop)
{
    /* no additional information to be filled/overriden into timer_prop
     * see also basic information populated in shared OS_TimeBaseGetInfo()
     */
    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TimeBaseCreate_Impl(const OS_object_token_t *token)
{
    int32                               return_code;
    OS_impl_timebase_internal_record_t *local;
    OS_timebase_internal_record_t *     timebase;
    OS_VoidPtrValueWrapper_t            timebase_id_variant;
    TickType_t                          default_dummy_interval_ticks;

    return_code = OS_SUCCESS;
    local       = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);
    timebase    = OS_OBJECT_TABLE_GET(OS_timebase_table, *token);

    memset(&timebase_id_variant, 0, sizeof(timebase_id_variant));
    timebase_id_variant.id = OS_ObjectIdFromToken(token);
    // OS_DEBUG("Info: OS_TimeBaseCreate() timebase osal_id: %lu.\n", OS_ObjectIdToInteger(timebase_id_variant.id));

    /*
     * Check if an external sync function is provided. 
     * If not, then will simulate using a software timer as sync function.
     */
    local->internal_sync                  = (timebase->external_sync == NULL);

    local->timer_handle                   = NULL;
    local->handler_mutex                  = NULL;
    local->handler_task                   = NULL;
    local->tick_binary_sem                = NULL;
    local->configured_start_time_usecs    = 0;
    local->configured_interval_time_usecs = 0;
    local->interval_time_ticks            = 0;
    local->reset_first_fired              = 1;

    /*
     * Set up the necessary OS constructs for timer, only is no external sync 
     * function is provided.
     */
    if (local->internal_sync)
    {
        timebase->external_sync = OS_FreeRTOS_TimeBase_SemWaitImpl;

        /*
         * The tick_sem is a simple semaphore posted by FreeRTOS timer callback function
         * and taken by the timebase helper task (created later).
         */

        local->tick_binary_sem = xSemaphoreCreateBinaryStatic(&local->tick_binary_sem_buffer);

        if(local->tick_binary_sem == NULL){
            OS_DEBUG("Error: Tick Sem could not be created.\n");
            return_code = OS_TIMER_ERR_INTERNAL;
        }

        /*
         * Binary semaphore which includes priority, the handler_mutex 
         * deals with access to the callback list for this timebase.
         */
        local->handler_mutex = xSemaphoreCreateMutexStatic(&local->handler_mutex_buffer);

        if (local->handler_mutex == NULL)
        {
            OS_DEBUG("Error: Handler Mutex could not be created.\n");
            /* It is safe to delete semaphore here because there is no task blocked on it. */
            vSemaphoreDelete(local->tick_binary_sem);
            return_code = OS_TIMER_ERR_INTERNAL;
        }


        /*
         * FreeRTOS timers are created in the dormant state and can be later activated
         * with xTimerStart(), xTimerChangePeriod(), etc.
         *
         * Start/Interval time in unknown at this point, hence we are providing a 
         * non-zero dummy value to be overriden in OS_TimeBaseSet_Impl().
         */
        OS_FreeRTOS_UsecsToTicks(1000000, &default_dummy_interval_ticks);

        local->timer_handle = xTimerCreateStatic(
            "OS_TIMEBASE",
            default_dummy_interval_ticks,       /*TimerPeriodInTicks*/
            pdFALSE,                            /* AutoReload */
            timebase_id_variant.opaque_arg,     /* pvTimerID */
            OS_FreeRTOS_TimeBase_TimerCallback,
            &local->timer_buffer );

        if( local->timer_handle == NULL )
        {
            OS_DEBUG("Error: xTimerCreateStatic() for timer failed.\n");
            /* It is safe to delete semaphore here because there is no task blocked on it. */
            vSemaphoreDelete(local->handler_mutex);
            vSemaphoreDelete(local->tick_binary_sem);
            return_code = OS_TIMER_ERR_UNAVAILABLE;
        }
    }

    /*
     * Spawn a dedicated time base handler thread
     *
     * This alleviates the need to handle expiration in the context of a signal handler -
     * The handler thread can call a BSP synchronized delay implementation as well as the
     * application callback function.  It should run with elevated priority to reduce latency.
     *
     * Note the thread will not actually start running until this function exits and releases
     * the global table lock.
     */
    if (return_code == OS_SUCCESS)
    {
        /* In FreeRTOS, highest numeric value means highest priority.
         */

        if ( xTaskCreate(
                    OS_FreeRTOS_TimeBaseTaskEntryPoint, /* task entry point */
                    "OS_TIMEBASE",                      /* task name */
                    configMINIMAL_STACK_SIZE,           /* task stack size to be dynamically allocated */
                    timebase_id_variant.opaque_arg,     /* pvParameters */
                    configTIMER_TASK_PRIORITY,
                    &local->handler_task ) != pdTRUE )
        {
            /* Provide some feedback as to why this failed */
            OS_printf("xTaskCreate() failed.\n");
            return_code = OS_TIMER_ERR_INTERNAL;
        }

        if (return_code != OS_SUCCESS)
        {
            /* Also delete the resources we allocated earlier */

            /* Post delete on timer command queue. */
            if ( xTimerDelete( local->timer_handle, portMAX_DELAY ) != pdPASS ) {
                OS_printf("Failed to post timer delete command .\n");
            };
            /* It is safe to delete semaphore here because there is no task blocked on it. */
            vSemaphoreDelete(local->handler_mutex);
            vSemaphoreDelete(local->tick_binary_sem);
            return return_code;
        }
    }

    return return_code;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TimeBaseSet_Impl(const OS_object_token_t *token, uint32 start_time_usecs, uint32 interval_time_usecs)
{
    int32                               return_code;
    TickType_t                          start_time_ticks;

    OS_impl_timebase_internal_record_t *local;
    OS_timebase_internal_record_t *     timebase;

    local       = OS_OBJECT_TABLE_GET(OS_impl_timebase_table, *token);
    timebase    = OS_OBJECT_TABLE_GET(OS_timebase_table, *token);
    return_code = OS_SUCCESS;

    /* There is only something to do here if we are generating a simulated tick */
    if (local->internal_sync)
    {
        /*
        ** Note that UsecsToTicks() already protects against intervals
        ** less than os_clock_accuracy -- no need for extra checks which
        ** would actually possibly make it less accurate.
        **
        ** Still want to preserve zero, since that has a special meaning.
        */

        if (start_time_usecs <= 0)
        {
            interval_time_usecs = 0; /* cannot have interval without start */
        }

        if (interval_time_usecs <= 0)
        {
            local->interval_time_ticks = 0;
        }
        else
        {
            OS_FreeRTOS_UsecsToTicks(interval_time_usecs, &local->interval_time_ticks);
        }

        /*
        ** The defined behavior is to not arm the timer if the start time is zero
        ** If the interval time is zero, then the timer will not be re-armed.
        */
        if (start_time_usecs > 0)
        {
            /*
            ** Convert from Microseconds to the timeout
            */
            OS_FreeRTOS_UsecsToTicks(start_time_usecs, &start_time_ticks);

            /* We are starting with auto-reload disabled, but later, when
             * start time is triggered, if interval time is non-zero, 
             * the sync function will rearm timer with interval time 
             * and make it auto-reload.
             */
            vTimerSetReloadMode(local->timer_handle, pdFALSE);
            if ( xTimerChangePeriod(local->timer_handle, start_time_ticks, portMAX_DELAY) != pdPASS ) {
                OS_DEBUG("ERROR: xTimerChangePeriod().\n")
                return_code = OS_TIMER_ERR_INTERNAL;
            }
            else
            {
                local->configured_start_time_usecs    = (10000 * start_time_ticks) / OS_SharedGlobalVars.TicksPerSecond;
                local->configured_interval_time_usecs = (10000 * local->interval_time_ticks) / OS_SharedGlobalVars.TicksPerSecond;
                local->configured_start_time_usecs *= 100;
                local->configured_interval_time_usecs *= 100;

                if (local->configured_start_time_usecs != start_time_usecs)
                {
                    OS_DEBUG("WARNING: timer %lu start_time requested=%luus, configured=%luus\n",
                             OS_ObjectIdToInteger(OS_ObjectIdFromToken(token)), (unsigned long)start_time_usecs,
                             (unsigned long)local->configured_start_time_usecs);
                }
                if (local->configured_interval_time_usecs != interval_time_usecs)
                {
                    OS_DEBUG("WARNING: timer %lu interval_time requested=%luus, configured=%luus\n",
                             OS_ObjectIdToInteger(OS_ObjectIdFromToken(token)), (unsigned long)interval_time_usecs,
                             (unsigned long)local->configured_interval_time_usecs);
                }

                if (local->interval_time_ticks > 0)
                {
                    timebase->accuracy_usec = local->configured_interval_time_usecs;
                }
                else
                {
                    timebase->accuracy_usec = local->configured_start_time_usecs;
                }
            }
        }
    }

    if (local->reset_first_fired == 0 && return_code == OS_SUCCESS)
    {
        local->reset_first_fired = 1;
    }

    return return_code;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TimeBaseDelete_Impl(const OS_object_token_t *token)
{
    // @FIXME
    OS_DEBUG("OS_ERR_NOT_IMPLEMENTED.\n");
    return OS_ERR_NOT_IMPLEMENTED;
}

