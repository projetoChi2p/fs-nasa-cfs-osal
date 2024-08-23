#include <stdlib.h>
#include <string.h>

#include <app_helpers.h>
#include <os-shared-globaldefs.h>

#include "generic_freertos_bsp_internal.h"


/*
** Global variables
*/
OS_BSP_GenericFreeRtosGlobalData_t OS_BSP_GenericFreeRtosGlobal;

TaskHandle_t cfe_psp_task_handle;

#define PSP_CFE_TASK_STACKSIZE (MAX_CONSTANT(2048, configMINIMAL_STACK_SIZE))
/* This is a FreeRTOS priority, not an OSAL priority. */
#define PSP_CFE_TASK_PRIORITY 4

#define BSP_FREERTOS_ABEND_MESSAGE      "Abnormal scheduler termination.\r\n"
#define BSP_FREERTOS_GLOBAL_INIT_FAILED "Global initialization failed..\r\n"

// @TODO FBV 2024-01-05 use OS_DebugPrintf() prototype from header
//void OS_DebugPrintf(uint32 Level, const char *Func, uint32 Line, const char *Format, ...);

void OS_BSP_Shutdown_Impl(void){
    // Conditional compilation to handle satellite reboot when embeded or exit() with return code
    // when freertos in running posix simulation.
    #if (defined(__arm__) && !defined(__linux__))
        // OS_printf("PSP CFE Task complete. vTaskDelete(self).\n");
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
    // OS_DebugPrintf(1, __func__, __LINE__, "\r\n");
    OS_Application_Startup();
    // OS_DebugPrintf(1, __func__, __LINE__, "\r\n");
    OS_Application_Run();
    // OS_DebugPrintf(1, __func__, __LINE__, "\r\n");

    // OS_printf("PSP CFE Task complete. vTaskDelete(self).\n");

    // vTaskDelete(NULL);
    OS_BSP_Shutdown_Impl();
}

// @TODO FBV 2024-01-05 use HLP_vSystemConfig() prototype from header
void HLP_vSystemConfig(void);
// @TODO FBV 2024-01-05 use PSP_Console_Init() prototype from header
int32 PSP_Console_Init(void);


int main(void){
    OS_BSP_GenericFreeRtosGlobal.AccessMutex = NULL;

    HLP_vSystemConfig();
    // PSP_Console_Init();

    memset(&OS_BSP_Global, 0, sizeof(OS_BSP_Global));
    OS_BSP_GenericFreeRtosGlobal.AccessMutex = xSemaphoreCreateMutex();

    if(OS_BSP_GenericFreeRtosGlobal.AccessMutex == NULL){
        HLP_vConsolePrintBytesBaremetal((uint8_t *)BSP_FREERTOS_GLOBAL_INIT_FAILED, sizeof(BSP_FREERTOS_GLOBAL_INIT_FAILED));
        return OS_SEM_FAILURE;
    }

    xTaskCreate(
        &PSP_CFE_Task,
        "PSP_CFE_Task",
        PSP_CFE_TASK_STACKSIZE,
        NULL,  // pvParameters
        PSP_CFE_TASK_PRIORITY,
        &cfe_psp_task_handle  // pxCreatedTask handle
    );

    vTaskStartScheduler();
    HLP_vConsolePrintBytesBaremetal((uint8_t *)BSP_FREERTOS_ABEND_MESSAGE, sizeof(BSP_FREERTOS_ABEND_MESSAGE));

    return OS_BSP_Global.AppStatus;
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
