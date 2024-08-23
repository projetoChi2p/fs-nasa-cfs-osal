/*
 * FreeRTOS
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

#ifndef __HEADER_FREERTOSCONFIG_H__
#define __HEADER_FREERTOSCONFIG_H__


#if (defined(__i386__) && defined(__linux__) && defined(__GNUC__))
#include <pthread.h> 
#include <limits.h>
/* The stack size being passed is equal to the minimum stack size needed by pthread_create(). */
#define PLATFORM_STACK_MIN PTHREAD_STACK_MIN
#define PLATFORM_HEAP_SIZE (1024 * 1024)

#else
#error Unknown/Unsupported/Untested target platform/compiler
#endif


#ifndef portINLINE
#define portINLINE         __inline
#endif


#define configPRIO_BITS         4        /* 15 priority levels */


// debugging
extern void vAssertCalled( const char * const pcFileName,
                           unsigned long ulLine );

/* It is a good idea to define configASSERT() while developing.  configASSERT()
 * uses the same semantics as the standard C assert() macro.  Don't define
 * configASSERT() when performing code coverage tests though, as it is not
 * intended to asserts() to fail, some some code is intended not to run if no
 * errors are present. */
#define configASSERT( x )    if( ( x ) == 0 ) vAssertCalled( __FILE__, __LINE__ )

#define configUSE_TRACE_FACILITY                        1

// clock
#define configTICK_RATE_HZ                              ((TickType_t) 1000)

// memory
#define configSUPPORT_DYNAMIC_ALLOCATION                1
#define configSUPPORT_STATIC_ALLOCATION                 1
#define configTOTAL_HEAP_SIZE                           PLATFORM_HEAP_SIZE
#define configMINIMAL_STACK_SIZE                        ( ( unsigned short ) PLATFORM_STACK_MIN )

#define configTIMER_TASK_STACK_DEPTH                    (configMINIMAL_STACK_SIZE * 1)

#define configCHECK_FOR_STACK_OVERFLOW                  1

// hooks
#define configUSE_DAEMON_TASK_STARTUP_HOOK              0
#define configUSE_IDLE_HOOK                             0
#define configUSE_MALLOC_FAILED_HOOK                    1  // https://www.freertos.org/a00016.html
#define configUSE_TICK_HOOK                             1

// handlers
// None for FreeRTOS over POSIX

// additional functions
#define INCLUDE_eTaskGetState                           1
#define INCLUDE_uxTaskGetStackHighWaterMark             0
#define INCLUDE_uxTaskGetStackHighWaterMark2            0
#define INCLUDE_uxTaskPriorityGet                       1
#define INCLUDE_vTaskCleanUpResources                   0
#define INCLUDE_vTaskDelay                              1
#define INCLUDE_vTaskDelayUntil                         1
#define INCLUDE_vTaskDelete                             1
#define INCLUDE_vTaskPrioritySet                        1
#define INCLUDE_vTaskSuspend                            1
#define INCLUDE_xSemaphoreGetMutexHolder                1
#define INCLUDE_xTaskAbortDelay                         0
#define INCLUDE_xTaskGetHandle                          1
#define INCLUDE_xTaskGetIdleTaskHandle                  0
#define INCLUDE_xTaskGetSchedulerState                  1
#define INCLUDE_xTimerGetTimerDaemonTaskHandle          0
#define INCLUDE_xTimerPendFunctionCall                  0

// interrupts
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         (0xf)
#define configKERNEL_INTERRUPT_PRIORITY                 ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    (5)
#define configMAX_SYSCALL_INTERRUPT_PRIORITY            ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

// other configuration
#define configIDLE_SHOULD_YIELD                         1
#define configMAX_TASK_NAME_LEN                         16
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS         3  // index 0..1 used by FreeRTOSFATConfig.h
#define configNUM_TX_DESCRIPTORS                        4
#define configQUEUE_REGISTRY_SIZE                       10
#define configSTREAM_BUFFER_TRIGGER_LEVEL_TEST_MARGIN   2
#define configTIMER_QUEUE_LENGTH                        15
#define configUSE_16_BIT_TICKS                          0
#define configUSE_CO_ROUTINES                           0
#define configUSE_COUNTING_SEMAPHORES                   1
#define configUSE_MUTEXES                               1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION         0
#define configUSE_PREEMPTION                            1
#define configUSE_RECURSIVE_MUTEXES                     1
#define configUSE_TIMERS                                1
#define configMAX_PRIORITIES                            10
#define configTIMER_TASK_PRIORITY                       (configMAX_PRIORITIES - 1)

#endif /* __HEADER_FREERTOSCONFIG_H__ */
