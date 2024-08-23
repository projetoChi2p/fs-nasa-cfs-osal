#ifndef INCLUDE_OS_IMPL_TIMEBASE_H
#define INCLUDE_OS_IMPL_TIMEBASE_H

#include <osconfig.h>
#include "os-freertos.h"
#include "os-shared-idmap.h"
#include "os-shared-mutex.h"

typedef struct
{
    uint8             internal_sync;                   /**< Flag indicating if sync function is simulated locally instead of an external sync provided by user. */

    TaskHandle_t      handler_task;                    /**< A FreeRTOS task handle for timer handler task. */

    SemaphoreHandle_t handler_mutex;                   /**< A FreeRTOS mutex handle for the timebase handler task. */
    StaticSemaphore_t handler_mutex_buffer;            /**< Static memory for FreeRTOS mutex. */
    SemaphoreHandle_t tick_binary_sem;                 /**< A FreeRTOS binary semaphor handle to be posted by FreeRTOS timer callback. */
    StaticSemaphore_t tick_binary_sem_buffer;          /**< Static memory for FreeRTOS semaphore. */

    uint32            configured_start_time_usecs;     /**< Timer start time (first event). */
    uint32            configured_interval_time_usecs;  /**< Timer tick event interval, in microseconds, if reload after first event. */
    TickType_t        interval_time_ticks;             /**< Timer tick event interval, in OS ticks, if reload after first event. */

    TimerHandle_t     timer_handle;                    /**< A handle to the FreeRTOS timer feeding the time base. */
    StaticTimer_t     timer_buffer;                    /**< Static memory for FreeRTOS timer. */
    uint8             reset_first_fired;               /**< Flag indicating if the timer is first fired (delayed start time) or not (elapsed interval time). */
} OS_impl_timebase_internal_record_t;

extern OS_impl_timebase_internal_record_t OS_impl_timebase_table[OS_MAX_TIMEBASES];

#endif /* INCLUDE_OS_IMPL_TIMEBASE_H */
