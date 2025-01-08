#include <os-shared-globaldefs.h>
#include "os-impl-task.h"

// OS_ObjectIdToInteger() returns unsigned long
#define TASKID_STRINGIFY_FORMAT "%lu"

// globals definitions
OS_impl_task_internal_record_t OS_impl_task_table[OS_MAX_TASKS];

/*----------------------------------------------------------------
 * Function: OS_FreeRTOS_TaskAPI_Impl_Init
 *-----------------------------------------------------------------*/
int32 OS_FreeRTOS_TaskAPI_Impl_Init(void)
{
    memset(OS_impl_task_table, 0, sizeof(OS_impl_task_table));
    return (OS_SUCCESS);
} /* end OS_FreeRTOS_TaskAPI_Impl_Init */


static void OS_FreeRTOS_TaskEntryPoint(void *pvParameters)
{
    uint32 obj_id = *(uint32 *) pvParameters;
    OS_TaskEntryPoint(OS_ObjectIdFromInteger(obj_id));
}

/*----------------------------------------------------------------
   Function: OS_TaskMatch_Impl
 ------------------------------------------------------------------*/
int32 OS_TaskMatch_Impl(const OS_object_token_t *token){
    // OS_task_internal_record_t *task;
    OS_impl_task_internal_record_t *impl;
    TaskHandle_t current_task;

    // task = OS_OBJECT_TABLE_GET(OS_task_table, *token);
    impl = OS_OBJECT_TABLE_GET(OS_impl_task_table, *token);

    current_task = xTaskGetCurrentTaskHandle();

    if(impl->xTask == current_task){
      return OS_SUCCESS;
    }

    return OS_ERROR;
}

/*----------------------------------------------------------------
   Function: OS_TaskCreate_Impl
 ------------------------------------------------------------------*/
int32 OS_TaskCreate_Impl(const OS_object_token_t *token, uint32 flags)
{
    OS_impl_task_internal_record_t *impl;
    OS_task_internal_record_t *task;
    BaseType_t xReturnCode;

    impl = OS_OBJECT_TABLE_GET(OS_impl_task_table, *token);
    task = OS_OBJECT_TABLE_GET(OS_task_table, *token);

    impl->xTaskBuffer = NULL;
    impl->obj_id = OS_ObjectIdToInteger(OS_ObjectIdFromToken(token));
    sprintf(impl->obj_id_str, TASKID_STRINGIFY_FORMAT, impl->obj_id);
    // OS_DebugPrintf(1, __func__, __LINE__, "TASK: %s STACK: %d\n", impl->obj_id_str, task->stack_size);


    /* OSAL stack pointer is void*
     * OSAL API documentation for OS_TaskCreate does not inform the unit of stack size.
     * Unit tests pass sizeof() as size for OS_TaskCreate(), hence bytes.
     * Pthreads use bytes.
     * FreeRTOS uses count of StackType_t, internally multiplied by sizeof() for malloc().
     */

    //  create task
    //  xTaskCreate allocates from FreeRTOS heap
    //  xTaskCreateStatic is allocated ahead of calling OS_TaskCreate (with task->stack_pointer)
    if (task->stack_pointer == NULL) {
        xReturnCode = xTaskCreate(
            OS_FreeRTOS_TaskEntryPoint,
            impl->obj_id_str,
            ( task->stack_size / sizeof(StackType_t) ),
            &impl->obj_id,  // pvParameters
            OS_FreeRTOS_MapOsalPriority(task->priority),
            (TaskHandle_t *) &impl->xTask  // pxCreatedTask handle
        );
    }
    else
    {
        // Task stack is already allocated, so it uses StaticTask to do not reallocated the stack.
        // This requires to allocate xTaskBuffer, and by consequence, desalocate when delete the task.
        impl->xTaskBuffer = (StaticTask_t *)pvPortMalloc(sizeof(StaticTask_t));
        impl->xTask = xTaskCreateStatic(
            OS_FreeRTOS_TaskEntryPoint,         
            impl->obj_id_str,                  
            ( task->stack_size / sizeof(StackType_t) ),
            &impl->obj_id, // pvParameters
            OS_FreeRTOS_MapOsalPriority(task->priority),
            task->stack_pointer, 
            impl->xTaskBuffer
        );

        /*
        If neither puxStackBuffer or pxTaskBuffer are NULL then the task will be created, and the task's handle is returned.
        If either puxStackBuffer or pxTaskBuffer is NULL then the task will not be created and NULL will be returned.
        */
        if (impl->xTask == NULL) {
            xReturnCode = pdFALSE;
        }
        else {
            xReturnCode = pdTRUE;
        }
    }

    if (xReturnCode != pdTRUE){
        OS_printf("xTaskCreate %s failed.\n", task->task_name);
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
   Function: OS_TaskDelete_Impl
 ------------------------------------------------------------------*/
int32 OS_TaskDelete_Impl(const OS_object_token_t *token){
    OS_impl_task_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_task_table, *token);

    if (! impl->task_called_exit ) {
        vTaskDelete(impl->xTask);
    }

    // xTaskBuffer diferent than NULL means that xTaskCreateStatic was used instead xTaskCreate.
    if (impl->xTaskBuffer != NULL)  {
        vPortFree(impl->xTaskBuffer);
        impl->xTaskBuffer = NULL;
    }

    /* Reset the table entry */
    memset(impl, 0, sizeof(*impl));

    return OS_SUCCESS;
}


/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_TaskDetach_Impl(const OS_object_token_t *token)
{
    /* No-op on FreeRTOS */
    return OS_SUCCESS;
}


/*----------------------------------------------------------------
   Function: OS_TaskExit_Impl
 ------------------------------------------------------------------*/
void OS_TaskExit_Impl(void){
    
    OS_object_token_t token;
    osal_id_t task_id;
    
    task_id = OS_TaskGetId_Impl();

    if (OS_ObjectIdGetById(OS_LOCK_MODE_NONE, OS_OBJECT_TYPE_OS_TASK, task_id, &token) == OS_SUCCESS)
    {
        OS_impl_task_internal_record_t *impl;

        impl = OS_OBJECT_TABLE_GET(OS_impl_task_table, token);

        impl->task_called_exit = 1;
        vTaskDelete(NULL);
    }
    else
    {
        // Inconsistent state, minimize collateral.
        vTaskSuspend(NULL);
    }
}

/*----------------------------------------------------------------
   Function: OS_TaskDelay_Impl
 ------------------------------------------------------------------*/
int32 OS_TaskDelay_Impl(uint32 millisecond){
    vTaskDelay(millisecond / portTICK_PERIOD_MS);
    return OS_SUCCESS;
}

/*----------------------------------------------------------------
   Function: OS_TaskSetPriority_Impl
 ------------------------------------------------------------------*/
int32 OS_TaskSetPriority_Impl(const OS_object_token_t *token, osal_priority_t new_priority){

    if ( INCLUDE_vTaskPrioritySet == 1) 
    {
        OS_impl_task_internal_record_t *impl;

        impl = OS_OBJECT_TABLE_GET(OS_impl_task_table, *token);
        vTaskPrioritySet(
            impl->xTask,
            OS_FreeRTOS_MapOsalPriority(new_priority)
        );
    }
    else 
    {
        return OS_ERR_NOT_IMPLEMENTED;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
   Function: OS_TaskGetId_Impl
 ------------------------------------------------------------------*/
osal_id_t OS_TaskGetId_Impl(void){

    unsigned long obj_id;
    osal_id_t global_task_id;
    char *task_name;

    task_name = pcTaskGetName(NULL);
    obj_id = atol(task_name);
    global_task_id = OS_ObjectIdFromInteger(obj_id);

    return global_task_id;
}

/*----------------------------------------------------------------
   Function: OS_TaskGetInfo_Impl
 ------------------------------------------------------------------*/
int32 OS_TaskGetInfo_Impl(const OS_object_token_t *token, OS_task_prop_t *task_prop){
    OS_impl_task_internal_record_t *impl;

    /*
     * The task_prop comes partially filled from common OSAL.
     * There is no information override here.
     * We are only doing some additional checking on impl table.
     */
    if (task_prop == NULL) {
        return OS_INVALID_POINTER;
    }

    impl = OS_OBJECT_TABLE_GET(OS_impl_task_table, *token);
    
    if ( impl->obj_id != OS_ObjectIdToInteger(OS_ObjectIdFromToken(token)) )
    {
        return OS_ERR_INVALID_ID;
    }

    if (impl->xTask == NULL)
    {
        return OS_ERR_NAME_NOT_FOUND;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
   Function: OS_TaskRegister_Impl
 ------------------------------------------------------------------*/
int32 OS_TaskRegister_Impl(osal_id_t global_task_id){
    // we already save impl->obj_id in OS_TaskCreate_Impl()
    // @FIXME we could construct a hash table here to
    // look up global_task_id faster in OS_TaskGetId_Impl()
    return OS_SUCCESS;
}

/*----------------------------------------------------------------
   Function: OS_TaskIdMatchSystemData_Impl
 ------------------------------------------------------------------*/
bool OS_TaskIdMatchSystemData_Impl(void *ref, const OS_object_token_t *token, const OS_common_record_t *obj){
    return OS_ERROR; // @FIXME
}

/*----------------------------------------------------------------
   Function: OS_TaskValidateSystemData_Impl
 ------------------------------------------------------------------*/
int32 OS_TaskValidateSystemData_Impl(const void *sysdata, size_t sysdata_size){
    return OS_ERROR; // @FIXME
}

