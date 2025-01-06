#include "os-freertos.h"
#include "os-shared-printf.h"
#include "os-shared-idmap.h"
#include "os-shared-common.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "generic_freertos_bsp_internal.h" // Used to access OS_BSP_GenericFreeRtosGlobal to register task handle
                                           // Note: this shall be removed when FreeRTOS task is replaced by OSAL task


#define OS_CONSOLE_TASK_PRIORITY  OS_UTILITYTASK_PRIORITY
#define OS_CONSOLE_TASK_STACKSIZE OS_UTILITYTASK_STACK_SIZE


typedef struct
{
    SemaphoreHandle_t console_sem;
    TaskHandle_t task_handle;
} OS_impl_console_internal_record_t;

OS_impl_console_internal_record_t OS_impl_console_table[OS_MAX_CONSOLES];


#if ( defined(OS_CONSOLE_TASK_REPORT_TASKS) || defined(OS_CONSOLE_TASK_REPORT_FILES) )
    void HLP_ReportTasksIfOnTime(void);
    void HLP_ReportFilesIfOnTime(void);
    #define OS_CONSOLE_TIMEOUT_TICKS (pdMS_TO_TICKS(10))
    #define OS_CONSOLE_TASK_FORCE_SPAWN true
#else
    #define OS_CONSOLE_TIMEOUT_TICKS (portMAX_DELAY)
    #define OS_CONSOLE_TASK_FORCE_SPAWN false
#endif


static void OS_ConsoleTask_Entry(void *pvParameters)
{
    OS_VoidPtrValueWrapper_t console_id_variant;
    osal_id_t                console_id;
    OS_object_token_t        token;

    memset(&console_id_variant, 0, sizeof(console_id_variant));
    console_id_variant.opaque_arg = pvParameters;
    console_id = console_id_variant.id;

    if (OS_ObjectIdGetById(OS_LOCK_MODE_REFCOUNT, OS_OBJECT_TYPE_OS_CONSOLE, console_id, &token) == OS_SUCCESS)
    {
        OS_impl_console_internal_record_t *impl;
        impl = OS_OBJECT_TABLE_GET(OS_impl_console_table, token);

        while (OS_SharedGlobalVars.GlobalState != OS_SHUTDOWN_MAGIC_NUMBER) 
        {
            if ( xSemaphoreTake(impl->console_sem, OS_CONSOLE_TIMEOUT_TICKS) == pdTRUE)
            {
                // @FYI: OS_ConsoleOutput_Impl is provided in os/portable/os-impl-console-bsp.c
                OS_ConsoleOutput_Impl(&token);
            }
            else
            {
                #ifdef OS_CONSOLE_TASK_REPORT_TASKS
                HLP_ReportTasksIfOnTime();
                #endif
                #ifdef OS_CONSOLE_TASK_REPORT_FILES
                HLP_ReportFilesIfOnTime();
                #endif
            }
        }

        OS_ObjectIdRelease(&token);
    }
}

/*----------------------------------------------------------------
   Function: OS_ConsoleCreate_Impl
 ------------------------------------------------------------------*/
int32 OS_ConsoleCreate_Impl(const OS_object_token_t *token)
{

    OS_impl_console_internal_record_t *impl;
    OS_console_internal_record_t *console;

    // verify this is the first and only console to be initialized
    if( OS_ObjectIndexFromToken(token) != 0 )
    {
        OS_DEBUG_LEV(1, "Multiple console devices not implemented.\n");
        return OS_ERR_NOT_IMPLEMENTED;
    }

    impl = OS_OBJECT_TABLE_GET(OS_impl_console_table, *token);
    console = OS_OBJECT_TABLE_GET(OS_console_table, *token);

    impl->console_sem = NULL;
    impl->task_handle = NULL;

    if (console->IsAsync || OS_CONSOLE_TASK_FORCE_SPAWN) 
    {
        OS_VoidPtrValueWrapper_t console_id_variant;
        BaseType_t xReturnCode;

        OS_DEBUG_LEV(1, "Starting support for asynchronous console output.\n");

        memset(&console_id_variant, 0, sizeof(console_id_variant));
        console_id_variant.id = OS_ObjectIdFromToken(token);

        impl->console_sem = xSemaphoreCreateBinary();
        if(impl->console_sem == NULL){
            OS_DEBUG_LEV(1, "Failed to create console async signal.\n")
            return OS_ERROR;
        }

        // @TODO Reimplement console as a pure OSAL task with OSAL semaphores
        xReturnCode = xTaskCreate(
            &OS_ConsoleTask_Entry,
            "console task",
            ( OS_CONSOLE_TASK_STACKSIZE / sizeof(StackType_t) ),
            console_id_variant.opaque_arg, // pvParameters
            OS_FreeRTOS_MapOsalPriority(OS_CONSOLE_TASK_PRIORITY),
            &impl->task_handle  // pxCreatedTask handle
        );

        OS_BSP_GenericFreeRtosGlobal.console_task_handle = impl->task_handle;

        if (xReturnCode != pdTRUE)
        {
            vSemaphoreDelete(impl->console_sem);
            OS_DEBUG_LEV(1, "Failed to create console task.\n");
            return OS_ERROR;
        }
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
   Function: OS_ConsoleWakeup_Impl
 ------------------------------------------------------------------*/
void OS_ConsoleWakeup_Impl(const OS_object_token_t *token)
{
    OS_impl_console_internal_record_t *impl;
    OS_console_internal_record_t *console;

    impl = OS_OBJECT_TABLE_GET(OS_impl_console_table, *token);
    console = OS_OBJECT_TABLE_GET(OS_console_table, *token);

    if (console->IsAsync = true) 
    {
        if (impl->console_sem != NULL)
        {
            xSemaphoreGive(impl->console_sem);
        }
        else
        {
            // configuration changed in runtime? anyway, cant wake-up, fall back
            OS_ConsoleOutput_Impl(token);
        }
    }
    else 
    {
        // cant wake-up, fall back
        OS_ConsoleOutput_Impl(token);
    }
}
