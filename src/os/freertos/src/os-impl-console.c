#include "os-freertos.h"
#include "os-shared-printf.h"
#include "os-shared-idmap.h"
#include "os-shared-common.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define OS_CONSOLE_ASYNC          false
#define OS_CONSOLE_TASK_PRIORITY  2
#define OS_CONSOLE_TASK_STACKSIZE (MAX_CONSTANT(1024, configMINIMAL_STACK_SIZE))

typedef struct
{
    bool is_async;
    SemaphoreHandle_t console_sem;
    TaskHandle_t task_handle;
} OS_impl_console_internal_record_t;

OS_impl_console_internal_record_t OS_impl_console_table[OS_MAX_CONSOLES];

// @FYI: OS_ConsoleOutput_Impl is provided in os/portable/os-impl-console-bsp.c

static void OS_ConsoleTask_Entry(void *pvParameters)
{
    // @FIXME we pass `token->obj_id` as `arg` to match the RTEMS impl.
    // but should consider just passing token around as `pvParameters` param
    int32 arg = (int32) pvParameters;

    OS_object_token_t token;
    OS_impl_console_internal_record_t *local;

    if(OS_ObjectIdGetById(OS_LOCK_MODE_REFCOUNT, OS_OBJECT_TYPE_OS_CONSOLE, OS_ObjectIdFromInteger(arg), &token) ==
        OS_SUCCESS)
    {
        local = OS_OBJECT_TABLE_GET(OS_impl_console_table, token);

        while (OS_SharedGlobalVars.GlobalState != OS_SHUTDOWN_MAGIC_NUMBER) {
            OS_ConsoleOutput_Impl(&token);
            xSemaphoreTake(local->console_sem, portMAX_DELAY);
        }

        OS_ObjectIdRelease(&token);
    }
}

/*----------------------------------------------------------------
   Function: OS_ConsoleCreate_Impl
 ------------------------------------------------------------------*/
int32 OS_ConsoleCreate_Impl(const OS_object_token_t *token){
    int32 xReturnCode;
    OS_impl_console_internal_record_t *local;

    // verify this is the first and only console to be initialized
    if(OS_ObjectIndexFromToken(token) != 0){
        return OS_ERROR;
    }

    local = OS_OBJECT_TABLE_GET(OS_impl_console_table, *token);
    local->is_async = OS_CONSOLE_ASYNC;

    // initialize BSP console support
    // PSP_Console_Init();

    // create semaphore and release semaphore; must be done at semaphore creation
    local->console_sem = xSemaphoreCreateBinary();
    xSemaphoreGive(local->console_sem);

    // create task
    xReturnCode = xTaskCreate(
        &OS_ConsoleTask_Entry,
        "console task",
        OS_CONSOLE_TASK_STACKSIZE,
        (void*) token->obj_id,  // pvParameters
        OS_CONSOLE_TASK_PRIORITY,
        &local->task_handle  // pxCreatedTask handle
    );

    // @FYI: FreeRTOS often returns 1 (pdTRUE) whereas OSAL returns 0 for OS_SUCCESS
    if(xReturnCode != pdPASS){
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
   Function: OS_ConsoleWakeup_Impl
 ------------------------------------------------------------------*/
void OS_ConsoleWakeup_Impl(const OS_object_token_t *token){
    OS_impl_console_internal_record_t *local;

    local = OS_OBJECT_TABLE_GET(OS_impl_console_table, *token);

    if(local->is_async){
        xSemaphoreGive(local->console_sem);
    }else{
        OS_ConsoleOutput_Impl(token);
    }
}