#include <stdlib.h>
#include <string.h>

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
        vTaskDelete(NULL);
    #elif (defined(__i386__) && defined(__linux__))
        exit(0);
    #else
        #error Unknow target platform
    #endif
    // No action
}

void PSP_CFE_Task(void *pvParameters)
{
    // This task initializes PSP and CFE after Task Scheduler started
    OS_Application_Startup();
    OS_Application_Run();

    BSP_DEBUG("OS_Application_Run() left idle loop.\n");

    OS_BSP_Shutdown_Impl();
}

// @TODO FBV 2024-01-05 use PSP_Console_Init() prototype from header
int32 PSP_Console_Init(void);

osal_priority_t OS_MapFreeRTOSPriority(UBaseType_t priority)
{
    // TODO: finish this implementation
    osal_priority_t osal_priority;


    //osal highest priority is zero
    //osal lowest priority is OS_MAX_TASK_PRIORITY
    //freertos highest is configMAX_PRIORITIES
    //freertos lowest priority is zero (e.g. tskIDLE_PRIORITY)

    /**
     * OSAL priorities are in reverse order, and range
     * from 0 (highest; will preempt all other tasks) to
     * OS_MAX_TASK_PRIORITY (lowest; will not preempt any other task).
    */

    if (priority < 0) {
        priority = 0;
    }
    else if (priority > configMAX_PRIORITIES)
    {
        priority = configMAX_PRIORITIES;
    }

    osal_priority = OS_MAX_TASK_PRIORITY - (priority * (OS_MAX_TASK_PRIORITY / configMAX_PRIORITIES));

    return osal_priority;
}


UBaseType_t OS_FreeRTOS_MapOsalPriority(osal_priority_t priority) 
{
    UBaseType_t uxPriority;
    //osal highest priority is zero
    //osal lowest priority is OS_MAX_TASK_PRIORITY
    //freertos highest is configMAX_PRIORITIES
    //freertos lowest priority is zero (e.g. tskIDLE_PRIORITY)

    /**
     * OSAL priorities are in reverse order, and range
     * from 0 (highest; will preempt all other tasks) to
     * OS_MAX_TASK_PRIORITY (lowest; will not preempt any other task).
     */

    if (priority < 0)
    {
        priority = 0;
    }
    else if (priority > OS_MAX_TASK_PRIORITY) 
    {
        priority = OS_MAX_TASK_PRIORITY;
    }

    /* Map priority in range */
    uxPriority = ( (priority*configMAX_PRIORITIES) + OS_MAX_TASK_PRIORITY -1 ) / OS_MAX_TASK_PRIORITY;
    if (uxPriority < 0)
    {
        uxPriority = 0;
    }
    else if (uxPriority > configMAX_PRIORITIES) {
        uxPriority = configMAX_PRIORITIES;
    }

    /* Reverse order 
     * OSAL highest numeric value is lowest priority
     * FreeRTOS highest numeric value is highest priority
     */
    uxPriority = configMAX_PRIORITIES - uxPriority;
    if (uxPriority < 0)
    {
        uxPriority = 0;
    }
    else if (uxPriority > configMAX_PRIORITIES) {
        uxPriority = configMAX_PRIORITIES;
    }

    return uxPriority;
}


int main(void){

    BaseType_t xReturnCode;

    OS_BSP_GenericFreeRtosGlobal.AccessMutex = NULL;

    HLP_vSystemConfig();
    // PSP_Console_Init();

    memset(&OS_BSP_Global, 0, sizeof(OS_BSP_Global));
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
        &PSP_CFE_Task,
        "PSP_CFE_Task",
        ( PSP_CFE_TASK_STACK_SIZE_BYTES / sizeof(StackType_t) ),
        NULL,  // pvParameters
        OS_FreeRTOS_MapOsalPriority(PSP_CFE_TASK_PRIORITY),
        &(OS_BSP_GenericFreeRtosGlobal.cfe_psp_task_handle)  // pxCreatedTask handle
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
