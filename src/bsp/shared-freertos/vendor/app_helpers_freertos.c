#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <time.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "app_helpers.h"

#define EXCP_M "M"
#define EXCP_O "O"




/* We are using the FreeRTOS tick as a "high resolution" time base
 * supporting CFE_PSP_Get_Timebase() and CFE_PSP_GetTime(), which in
 * turn supports performance and benchmark metrics, while the OSAL
 * timebase, via OS_TimeBaseCreate(), feeds a lower resolution time.
 * If required, even higher accuracy could be achieved adding hardware
 * systick to tick counter.
 * CFE_PSP_Get_Timebase() supports up to 64 bits counter, but we are 
 * reusing legacy code with 32 bits, which is enough for several weeks.
 */
volatile uint32_t g_uwTick;

/* This mutex gives access to serial comunication port and print buffer.
 * It is bypassed by calls to HLP_vConsolePrintBytesBaremetal()
 */
SemaphoreHandle_t xStdioMutex;
StaticSemaphore_t xStdioMutexBuffer;
#define HLP_CONSOLE_PRINTF_BUFFER_SIZE 256
char hlp_console_printf_buffer[HLP_CONSOLE_PRINTF_BUFFER_SIZE];


void vApplicationStackOverflowHook( TaskHandle_t xTask, char * pcTaskName )
{
    /* Run time stack overflow checking is performed if
    configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */
	/* The stack space has been execeeded for a task, considering allocating more. */
    
    HLP_vConsolePrintFormattedBspUnlocked("vApplicationStackOverflowHook(%p, %s): ABORT [%s:%d]\n", xTask, pcTaskName, __FILE__, __LINE__);
    for (;;);
}


void vApplicationTickHook(void)
{
    HLP_vIncTick();
}

void HLP_vIncTick() {
    g_uwTick += 1;
}

uint32_t HLP_u32GetLoResTick(void) 
{
    return g_uwTick;
}

void HLP_vRtosBringUp() 
{
    g_uwTick = 0;
#if (defined(__i386__) && defined(__linux__))
    //HLP_vConsolePrintFormattedBspUnlocked("HLP_vRtosBringUp(), PTHREAD_STACK_MIN=%d [%s:%d]\n", PTHREAD_STACK_MIN, __FILE__, __LINE__);
#endif
}

void HLP_vConsoleInit( void ) 
{
    xStdioMutex = xSemaphoreCreateMutexStatic( &xStdioMutexBuffer );
}



void HLP_vConsolePrintFormatted( const char * Format, ... ) {
    va_list va;

    xSemaphoreTake( xStdioMutex, portMAX_DELAY );

    va_start(va, Format);
    vsnprintf(hlp_console_printf_buffer, HLP_CONSOLE_PRINTF_BUFFER_SIZE-1, Format, va);
    va_end(va);
    hlp_console_printf_buffer[HLP_CONSOLE_PRINTF_BUFFER_SIZE-1] = '\0';

    HLP_vConsolePrintBytesBaremetal( (uint8_t*)hlp_console_printf_buffer, strlen(hlp_console_printf_buffer) );

    xSemaphoreGive( xStdioMutex );
}

void HLP_vConsolePrintBytesStdioUnlocked( const uint8_t *data, int size )
{
    xSemaphoreTake( xStdioMutex, portMAX_DELAY );
    HLP_vConsolePrintBytesBaremetal( data, size );
    xSemaphoreGive( xStdioMutex );
}


/*
 * Prototypes for the standard FreeRTOS application hook (callback) functions
 * implemented within this file.  See http://www.freertos.org/a00016.html .
 */
void vApplicationMallocFailedHook( void );
void vApplicationIdleHook( void );

void vApplicationTickHook( void );
/*
void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    uint32_t * pulIdleTaskStackSize );
*/
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     uint32_t * pulTimerTaskStackSize );



/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 * used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    configSTACK_DEPTH_TYPE * pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
 * function then they must be declared static - otherwise they will be allocated on
 * the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
     * state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}


/* When configSUPPORT_STATIC_ALLOCATION is set to 1 the application writer can
 * use a callback function to optionally provide the memory required by the idle
 * and timer tasks.  This is the stack that will be used by the timer task.  It is
 * declared here, as a global, so it can be checked by a test that is implemented
 * in a different file. */
StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];


/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 * application must provide an implementation of vApplicationGetTimerTaskMemory()
 * to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     uint32_t * pulTimerTaskStackSize )
{
/* If the buffers to be provided to the Timer task are declared inside this
 * function then they must be declared static - otherwise they will be allocated on
 * the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
     * task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}


void vApplicationIdleHook( void )
{
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
     * to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
     * task.  It is essential that code added to this hook function never attempts
     * to block in any way (for example, call xQueueReceive() with a block time
     * specified, or call vTaskDelay()).  If application tasks make use of the
     * vTaskDelete() API function to delete themselves then it is also important
     * that vApplicationIdleHook() is permitted to return to its calling function,
     * because it is the responsibility of the idle task to clean up memory
     * allocated by the kernel to any task that has since deleted itself. */

    //usleep( 15000 );
}

void vApplicationDaemonTaskStartupHook( void )
{
    /* This function will be called once only, when the daemon task starts to
     * execute    (sometimes called the timer task).  This is useful if the
     * application includes initialisation code that would benefit from executing
     * after the scheduler has been started. */
}

void vAssertCalled( const char * const pcFileName,
                    unsigned long ulLine )
{
    static BaseType_t xPrinted = pdFALSE;
    volatile uint32_t ulSetToNonZeroInDebuggerToContinue = 0;

    /* Called if an assertion passed to configASSERT() fails.  See
     * https://www.FreeRTOS.org/a00110.html#configASSERT for more information. */

    /* Parameters are not used. */
    ( void ) ulLine;
    ( void ) pcFileName;


    taskENTER_CRITICAL();
    {
        /* Stop the trace recording. */
        if( xPrinted == pdFALSE )
        {
            xPrinted = pdTRUE;
            HLP_vConsolePrintFormattedBspLockUnknown("vAssertCalled() [%s:%d]\n", pcFileName, ulLine);
        }

        /* You can step out of this function to debug the assertion by using
         * the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
         * value. */
        while( ulSetToNonZeroInDebuggerToContinue == 0 )
        {
            __asm volatile ( "NOP" );
            __asm volatile ( "NOP" );
        }
    }
    taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/


void vApplicationMallocFailedHook( void )
{
    /* vApplicationMallocFailedHook() will only be called if
     * configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
     * function that will get called if a call to pvPortMalloc() fails.
     * pvPortMalloc() is called internally by the kernel whenever a task, queue,
     * timer or semaphore is created.  It is also called by various parts of the
     * demo application.  If heap_1.c, heap_2.c or heap_4.c is being used, then the
     * size of the    heap available to pvPortMalloc() is defined by
     * configTOTAL_HEAP_SIZE in FreeRTOSConfig.h, and the xPortGetFreeHeapSize()
     * API function can be used to query the size of free heap space that remains
     * (although it does not provide information on how the remaining heap might be
     * fragmented).  See http://www.freertos.org/a00111.html for more
     * information. */
    vAssertCalled( __FILE__, __LINE__ );
}
