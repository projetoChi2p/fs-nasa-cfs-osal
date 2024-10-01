#include <os-shared-globaldefs.h>
#include "os-impl-task.h"

// globals definitions
OS_impl_task_internal_record_t OS_impl_task_table[OS_MAX_TASKS];

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
    int obj_id = *(int *) pvParameters;
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
    sprintf(impl->obj_id_str, "%lu", impl->obj_id);

    //OS_DebugPrintf(1, __func__, __LINE__, "TASK: %s STACK: %d\n", impl->obj_id_str, task->stack_size);

    //  create task
    //  xTaskCreate allocates from FreeRTOS heap
    //  xTaskCreateStatic is allocated ahead of calling OS_TaskCreate (with task->stack_pointer)
    if (task->stack_pointer == NULL) {
        xReturnCode = xTaskCreate(
            OS_FreeRTOS_TaskEntryPoint,
            impl->obj_id_str,
            task->stack_size,
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
            task->stack_size / sizeof(StackType_t),
            &impl->obj_id,
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

    // xTaskBuffer diferent than NULL means that xTaskCreateStatic was used instead xTaskCreate.
    if (impl->xTaskBuffer != NULL)  {
        vPortFree(impl->xTaskBuffer);
        impl->xTaskBuffer = NULL;
    }

    vTaskDelete(impl->xTask);

    // @FIXME null the impl table

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
    vTaskDelete(NULL);

    // @FIXME null the impl table
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
    //#if ( INCLUDE_vTaskPrioritySet == 1 )
    // map priority
    //void vTaskPrioritySet( TaskHandle_t xTask, UBaseType_t uxNewPriority )

    return OS_ERROR; // @FIXME
}

/*----------------------------------------------------------------
   Function: OS_TaskGetId_Impl
 ------------------------------------------------------------------*/
osal_id_t OS_TaskGetId_Impl(void){
    osal_id_t global_task_id;
    char *task_name;

    task_name = pcTaskGetName(NULL);

    if (sscanf(task_name, "%lu", &global_task_id) == 1){
        return global_task_id;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
   Function: OS_TaskGetInfo_Impl
 ------------------------------------------------------------------*/
int32 OS_TaskGetInfo_Impl(const OS_object_token_t *token, OS_task_prop_t *task_prop){
    OS_impl_task_internal_record_t *impl;
    TaskStatus_t xTaskDetails;

    if (task_prop == NULL) {
        return OS_INVALID_POINTER;
    }

    impl = OS_OBJECT_TABLE_GET(OS_impl_task_table, *token);

    vTaskGetInfo(impl->xTask, &xTaskDetails, pdTRUE, eInvalid);

    // TODO
    strcpy(task_prop->name, xTaskDetails.pcTaskName);
    // task_prop->creator = NULL;
    // task_prop->stack_size = OS_MapFreeRTOSPriority(xTaskDetails.uxBasePriority);

    return OS_ERROR;
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

