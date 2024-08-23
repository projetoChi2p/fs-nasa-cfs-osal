#include <os-shared-globaldefs.h>
#include "os-impl-queue.h"
#include "os-freertos.h"

// globals
OS_impl_queue_internal_record_t OS_impl_queue_table[OS_MAX_QUEUES];

/*----------------------------------------------------------------
 * Function: OS_FreeRTOS_QueueAPI_Impl_Init
 *-----------------------------------------------------------------*/
int32 OS_FreeRTOS_QueueAPI_Impl_Init(void)
{
    memset(OS_impl_queue_table, 0, sizeof(OS_impl_queue_table));
    return OS_SUCCESS;
} /* end OS_FreeRTOS_QueueAPI_Impl_Init */

/*----------------------------------------------------------------
 * Function: OS_QueueCreate_Impl
 *-----------------------------------------------------------------*/
int32 OS_QueueCreate_Impl(const OS_object_token_t *token, uint32 flags)
{
    OS_impl_queue_internal_record_t *impl;
    OS_queue_internal_record_t *queue;

    impl = OS_OBJECT_TABLE_GET(OS_impl_queue_table, *token);
    queue = OS_OBJECT_TABLE_GET(OS_queue_table, *token);

    impl->xQueue = xQueueCreate(
        queue->max_depth, 
        queue->max_size
    );

    if(impl->xQueue == NULL){
        OS_printf("xQueueCreate failed\n");
        return OS_ERROR;
    }

    return OS_SUCCESS;
} /* end OS_QueueCreate_Impl */

/*----------------------------------------------------------------
 * Function: OS_QueueDelete_Impl
 *-----------------------------------------------------------------*/
int32 OS_QueueDelete_Impl(const OS_object_token_t *token)
{
    OS_impl_queue_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_queue_table, *token);

    vQueueDelete(impl->xQueue);

    return OS_SUCCESS;

} /* end OS_QueueDelete_Impl */

/*----------------------------------------------------------------
 * Function: OS_QueueGet_Impl
 *-----------------------------------------------------------------*/
int32 OS_QueueGet_Impl(const OS_object_token_t *token, void *data, size_t size, size_t *size_copied, int32 timeout)
{
    int32 return_code;
    int32 status;
    TickType_t ticks_to_wait;

    // @FYI: int32 timeout
    //      = -1 (OS_PEND) -- wait indefinitely
    //      =  0 (OS_CHECK) -- immediate (null if empty)
    //      = >0 -- time to wait in milliseconds

    OS_impl_queue_internal_record_t *impl;
    OS_queue_internal_record_t *queue;

    impl = OS_OBJECT_TABLE_GET(OS_impl_queue_table, *token);
    queue = OS_OBJECT_TABLE_GET(OS_queue_table, *token);

    if(timeout == OS_PEND){
        ticks_to_wait = portMAX_DELAY;
    }else if(timeout == OS_CHECK){
        ticks_to_wait = 0;
    }else{
        // convert from milliseconds to system ticks
        if(OS_Milli2Ticks(timeout, (int *) &ticks_to_wait) != OS_SUCCESS){
            return OS_ERROR;
        }
    }

    status = xQueueReceive(
        impl->xQueue,
        data,
        ticks_to_wait
    );

    // did we receive a queue message?
    if(status == pdTRUE){
        return_code = OS_SUCCESS;
    }else if(timeout == OS_CHECK){
        return_code = OS_QUEUE_EMPTY;
    }else{
        return_code = OS_QUEUE_TIMEOUT;
    }

    // is the message the correct size?
    // @FIXME not implemented
    //OS_DEBUG("WARNING: Indicating xQueueReceive buffer size of queue->max_size (may not be accurate)\n");
    *size_copied = queue->max_size;

    return return_code;
} /* end OS_QueueGet_Impl */

/*----------------------------------------------------------------
 * Function: OS_QueuePut_Impl
 *-----------------------------------------------------------------*/
int32 OS_QueuePut_Impl(const OS_object_token_t *token, const void *data, size_t size, uint32 flags)
{
    int32               status;

    OS_impl_queue_internal_record_t *impl;
    // OS_queue_internal_record_t *queue;

    impl = OS_OBJECT_TABLE_GET(OS_impl_queue_table, *token);
    // queue = OS_OBJECT_TABLE_GET(OS_queue_table, *token);

    // FreeRTOS supports waiting for full queue to become available.
    int ticks_to_wait = 0;

    status = xQueueSend(
        impl->xQueue,
        data,
        ticks_to_wait
    );

    if(status == errQUEUE_FULL){
        return OS_QUEUE_FULL;
    }else if(status != pdTRUE){
        OS_DEBUG("xQueueSend error: %d\n", status);
        return OS_ERROR;
    }

    return OS_SUCCESS;
} /* end OS_QueuePut_Impl */

/*----------------------------------------------------------------
 * Function: OS_QueueGetInfo_Impl
 *-----------------------------------------------------------------*/
int32 OS_QueueGetInfo_Impl(const OS_object_token_t *token, OS_queue_prop_t *queue_prop)
{
    return OS_SUCCESS;

} /* end OS_QueueGetInfo_Impl */
