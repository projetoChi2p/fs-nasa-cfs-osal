/****************************************************************************************
                                    INCLUDE FILES
 ***************************************************************************************/

#include <os-shared-globaldefs.h>
#include "os-impl-binsem.h"

/****************************************************************************************
                                     GLOBALS
 ***************************************************************************************/

OS_impl_bin_sem_internal_record_t OS_impl_bin_sem_table[OS_MAX_COUNT_SEMAPHORES];

/****************************************************************************************
                                INTERNAL FUNCTIONS
 ***************************************************************************************/

/*----------------------------------------------------------------
 * Function: OS_FreeRTOS_BinSemAPI_Impl_Init
 *-----------------------------------------------------------------*/
int32 OS_FreeRTOS_BinSemAPI_Impl_Init(void)
{
    memset(OS_impl_bin_sem_table, 0, sizeof(OS_impl_bin_sem_table));
    return OS_SUCCESS;
} /* end OS_FreeRTOS_BinSemAPI_Impl_Init */


/*----------------------------------------------------------------
   Function: OS_BinSemCreate_Impl
 ------------------------------------------------------------------*/
int32 OS_BinSemCreate_Impl(const OS_object_token_t *token, uint32 sem_initial_value, uint32 options){
    OS_impl_bin_sem_internal_record_t *impl;

    UNUSED_ARGUMENT(options);

    // verify initial value does not exceed limit
    if(sem_initial_value > OS_SEM_FULL){
        return OS_INVALID_SEM_VALUE;
    }

    impl    = OS_OBJECT_TABLE_GET(OS_impl_bin_sem_table, *token);


    impl->xBinSem = xSemaphoreCreateBinary();

    if(impl->xBinSem == NULL){
        return OS_SEM_FAILURE;
    }

    impl->initial_value = sem_initial_value;
    // release the sem immediately if initial value > 0
    if(sem_initial_value != OS_SEM_EMPTY) {
        OS_BinSemGive_Impl(token);
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
   Function: OS_BinSemGive_Impl
 ------------------------------------------------------------------*/
int32 OS_BinSemGive_Impl(const OS_object_token_t *token){
    OS_impl_bin_sem_internal_record_t *impl;

    impl    = OS_OBJECT_TABLE_GET(OS_impl_bin_sem_table, *token);

    if(xSemaphoreGive(impl->xBinSem) != pdTRUE){
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
 * Function: OS_BinSemFlush_Impl
 *-----------------------------------------------------------------*/
int32 OS_BinSemFlush_Impl(const OS_object_token_t *token)
{

    UNUSED_ARGUMENT(token);

    // See also Qin Ha https://forums.freertos.org/t/semaphore-flush-function/12770/2
    //It would be similar to, in a critical section:
    // if( listLIST_IS_EMPTY( &( pxQueue->xTasksWaitingToReceive ) ) == pdFALSE )
    // {
    //    if( xTaskRemoveFromEventList( &( pxQueue->xTasksWaitingToReceive ) ) != pdFALSE )
    //       {...}
    // }

    return OS_ERR_NOT_IMPLEMENTED; // @FIXME NOT IMPLEMENTED
} /* end OS_BinSemFlush_Impl */

/*----------------------------------------------------------------
   Function: OS_BinSemTake_Impl
 ------------------------------------------------------------------*/
int32 OS_BinSemTake_Impl(const OS_object_token_t *token){
    OS_impl_bin_sem_internal_record_t *impl;

    impl    = OS_OBJECT_TABLE_GET(OS_impl_bin_sem_table, *token);

    if(xSemaphoreTake(impl->xBinSem, portMAX_DELAY) != pdTRUE){
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
   Function: OS_BinSemTimedWait_Impl
 ------------------------------------------------------------------*/
int32 OS_BinSemTimedWait_Impl(const OS_object_token_t *token, uint32 msecs){
    TickType_t      ticks_to_wait;
    int32           status;

    OS_impl_bin_sem_internal_record_t *impl;

    impl    = OS_OBJECT_TABLE_GET(OS_impl_bin_sem_table, *token);

    if(OS_Milli2Ticks(msecs, (int *) &ticks_to_wait) != OS_SUCCESS){
        return OS_ERROR;
    }

    status = xSemaphoreTake(impl->xBinSem, ticks_to_wait);

    if(status == pdTRUE){
        return OS_SUCCESS;
    }else if(status == pdFALSE){
        return OS_SEM_TIMEOUT;
    }

    return OS_ERROR;
}

/*----------------------------------------------------------------
   Function: OS_BinSemDelete_Impl
 ------------------------------------------------------------------*/
int32 OS_BinSemDelete_Impl(const OS_object_token_t *token){
    OS_impl_bin_sem_internal_record_t *impl;
    //int32 sem_count;

    impl    = OS_OBJECT_TABLE_GET(OS_impl_bin_sem_table, *token);

    if(impl->xBinSem == NULL){
        OS_printf("OS_BinSemDelete() non-existing semaphore.\n");
        return OS_ERROR;
    }

    // @FIXME add OS_ERROR and unit test for this case:
    // "Do not delete a semaphore that has tasks blocked on it"
    // see: https://www.freertos.org/a00113.html#vSemaphoreDelete
    // We can not decide clearly if there is a blocked tasks waiting for the semaphore.
    // At least, if the semaphore was created empty, there may be or not be blocked tasks...
    // Even if the semaphore was created as non-empty, a zero count does not mean there is
    // something blocked waiting for more.
    // sem_count = uxSemaphoreGetCount(impl->xBinSem);
    // if (sem_count == 0) {
    //     OS_printf("OS_BinSemDelete() semaphore has blocks.\n");
    //     return OS_ERROR;
    // }

    vSemaphoreDelete(impl->xBinSem);

    /* Reset the table entry */
    memset(impl, 0, sizeof(*impl));

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
   Function: OS_BinSemGetInfo_Impl
 ------------------------------------------------------------------*/
int32 OS_BinSemGetInfo_Impl(const OS_object_token_t *token, OS_bin_sem_prop_t *bin_prop){
    OS_impl_bin_sem_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_bin_sem_table, *token);
    
    if (bin_prop == NULL) {
        return OS_INVALID_POINTER;
    }

    // Other properties are filled by base/shared implementation.
    bin_prop->value = uxSemaphoreGetCount(impl->xBinSem);

    return OS_SUCCESS ;
}
