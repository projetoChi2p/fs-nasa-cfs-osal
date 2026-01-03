#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <app_helpers.h>
#include <os-shared-globaldefs.h>

#include "generic_freertos_bsp_internal.h"


/*
** Global variables
*/
OS_BSP_GenericFreeRtosGlobalData_t OS_BSP_GenericFreeRtosGlobal;


#define BSP_FREERTOS_ABEND_MESSAGE      "Abnormal scheduler termination.\r\n"
#define BSP_FREERTOS_GLOBAL_INIT_FAILED "Global initialization failed.\r\n"
#define BSP_FREERTOS_TASK_INIT_FAILED   "Main task initialization failed.\r\n"

// @TODO FBV 2024-01-05 use OS_DebugPrintf() prototype from header
//void OS_DebugPrintf(uint32 Level, const char *Func, uint32 Line, const char *Format, ...);

void OS_BSP_Shutdown_Impl(void){
    // Currently we are not rebooting spacecraft.
    // TODO Conditional compilation to handle satellite reboot when embeded
    // or exit() with return code when FreeRTOS in running POSIX simulation.
    #if (defined(__arm__) && !defined(__linux__))
        vTaskDelete(NULL);
    #elif (defined(__riscv) && !defined(__linux__))
        while(1) {
            __asm__ __volatile__("nop");
        }
        vTaskDelete(NULL);
    #elif (defined(__i386__) && defined(__linux__))
        exit(0);
    #else
        #error Unknow target platform
    #endif
    // No action
}

void OS_BSP_Main_Task(void *pvParameters)
{
    UNUSED_ARGUMENT(pvParameters);

    // This task initializes PSP and CFE after Task Scheduler started
    OS_Application_Startup();
    #ifdef FREERTOS_TRACE_ENABLED
        extern void HLP_ReportTasksIfComplete();
        HLP_ReportTasksIfComplete();
    #endif
    OS_Application_Run();

    BSP_DEBUG("OS_Application_Run() left idle loop.\n");

    OS_BSP_Shutdown_Impl();
}

/* This OS_FreeRTOS_MapOsalPriority() function would fit better
 * inside os-impl-task.c, but that file/module is not compiled
 * in the case of OSAL's coverage tests, failing to satisfy 
 * linkage for main() below. Consequently, OS_FreeRTOS_MapOsalPriority()
 * is here in BSP, instead of in OS.
 */
UBaseType_t OS_FreeRTOS_MapOsalPriority(osal_priority_t priority) 
{
    UBaseType_t uxPriority;

    /**
     * OSAL priorities are in reverse order w.r.t. FreeRTOS, and
     * range from 0 (highest; will preempt all other tasks) to
     * OS_MAX_TASK_PRIORITY (lowest; will not preempt any other task).
     * - osal priority is unsigned
     * - osal highest priority is zero
     * - osal lowest priority is OS_MAX_TASK_PRIORITY
     * - freertos priority is unsigned
     * - freertos highest is configMAX_PRIORITIES
     * - freertos lowest priority is zero (e.g. tskIDLE_PRIORITY)
     */

    assert(sizeof(osal_priority_t)==1);                    /* priority is byte     */
    assert((osal_priority_t)(0) < (osal_priority_t)(-1));  /* priority is unsigned */
    assert(OS_MAX_TASK_PRIORITY == (osal_priority_t)(-1)); /* priority is unsigned */

    /* Map priority in range */
    uxPriority = ( (priority*configMAX_PRIORITIES) + OS_MAX_TASK_PRIORITY -1 ) / OS_MAX_TASK_PRIORITY;
    if (uxPriority > configMAX_PRIORITIES)
    {
        uxPriority = configMAX_PRIORITIES;
    }

    /* Reverse order 
     * OSAL highest numeric value is lowest priority
     * FreeRTOS highest numeric value is highest priority
     */
    uxPriority = configMAX_PRIORITIES - uxPriority;
    if (uxPriority > configMAX_PRIORITIES) {
        uxPriority = configMAX_PRIORITIES;
    }

    return uxPriority;
}


int main(int argc, char *argv[])
{

    BaseType_t xReturnCode;

    /*
     * Initially clear the global objects
     */
    memset(&OS_BSP_Global, 0, sizeof(OS_BSP_Global));
    memset(&OS_BSP_GenericFreeRtosGlobal, 0, sizeof(OS_BSP_GenericFreeRtosGlobal));

    OS_BSP_Global.ArgC = argc;
    OS_BSP_Global.ArgV = argv;

    HLP_vSystemConfig();
    // PSP_Console_Init();

    OS_BSP_GenericFreeRtosGlobal.AccessMutex = xSemaphoreCreateMutex();

    if(OS_BSP_GenericFreeRtosGlobal.AccessMutex == NULL){
        HLP_vConsolePrintBytesBaremetal((uint8_t *)BSP_FREERTOS_GLOBAL_INIT_FAILED, sizeof(BSP_FREERTOS_GLOBAL_INIT_FAILED));
        return OS_SEM_FAILURE;
    }

    /* OSAL is not brought-up at this point, hence we cannot 
     * rely on pure OSAL tasks and semaphores.
     * Still, while initializing OSAL, we may rely on FreeRTOS
     * features, like critical sections, for instance in 
     * filesystems initialization.
     * Hence, it is safer to bring-up OSAL from within FreeRTOS 
     * task.
     */
    xReturnCode = xTaskCreate(
        &OS_BSP_Main_Task,
        "BSP_Main",
        ( BSP_MAIN_TASK_STACK_SIZE_BYTES / sizeof(StackType_t) ),
        NULL,  // pvParameters
        OS_FreeRTOS_MapOsalPriority(BSP_MAIN_TASK_PRIORITY),
        &(OS_BSP_GenericFreeRtosGlobal.bsp_main_task_handle)  // pxCreatedTask handle
    );

    if (xReturnCode != pdTRUE)
    {
        HLP_vConsolePrintBytesBaremetal((uint8_t *)BSP_FREERTOS_TASK_INIT_FAILED, sizeof(BSP_FREERTOS_TASK_INIT_FAILED));
        return EXIT_FAILURE;
    }


    vTaskStartScheduler();
    HLP_vConsolePrintBytesBaremetal((uint8_t *)BSP_FREERTOS_ABEND_MESSAGE, sizeof(BSP_FREERTOS_ABEND_MESSAGE));

    return EXIT_FAILURE;
}


/*----------------------------------------------------------------
   OS_BSP_Lock_Impl
   See full description in header
 ------------------------------------------------------------------*/
void OS_BSP_Lock_Impl(void)
{
    if (OS_BSP_GenericFreeRtosGlobal.AccessMutex != NULL) {
        if(xSemaphoreTake(OS_BSP_GenericFreeRtosGlobal.AccessMutex, portMAX_DELAY) != pdTRUE){
            BSP_DEBUG("xSemaphoreTake(AccessMutex) failed.\n");
        }
    }
}

/*----------------------------------------------------------------
   OS_BSP_Unlock_Impl
   See full description in header
 ------------------------------------------------------------------*/
void OS_BSP_Unlock_Impl(void)
{
    if (OS_BSP_GenericFreeRtosGlobal.AccessMutex != NULL) {
        if(xSemaphoreGive(OS_BSP_GenericFreeRtosGlobal.AccessMutex) != pdTRUE){
            BSP_DEBUG("xSemaphoreGive(AccessMutex) failed.\n");
        }
    }
}
