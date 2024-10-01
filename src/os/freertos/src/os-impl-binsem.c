#include <os-shared-globaldefs.h>
#include "os-impl-binsem.h"

// globals
OS_impl_bin_sem_internal_record_t OS_impl_bin_sem_table[OS_MAX_COUNT_SEMAPHORES];

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

    impl    = OS_OBJECT_TABLE_GET(OS_impl_bin_sem_table, *token);

    impl->xBinSem = xSemaphoreCreateBinary();

    if(impl->xBinSem == NULL){
        return OS_SEM_FAILURE;
    }

    // release the sem immediately if initial value > 0
    if(sem_initial_value >= 1){
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
    return OS_ERROR; // @FIXME NOT IMPLEMENTED
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
    int32 sem_status;

    impl    = OS_OBJECT_TABLE_GET(OS_impl_bin_sem_table, *token);

    sem_status = uxSemaphoreGetCount(impl->xBinSem);

    // "Do not delete a semaphore that has tasks blocked on it"
    // see: https://www.freertos.org/a00113.html#vSemaphoreDelete
    /* If the semaphore is a binary semaphore then 1 is returned if the semaphore is available, and 0 is returned if the semaphore is not available.*/
    if (sem_status == 0) {
        return OS_ERROR;
    }

    vSemaphoreDelete(impl->xBinSem);

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

    // TODO:
    bin_prop->value = uxSemaphoreGetCount(impl->xBinSem);
    // bin_prop->creator = ??
    // bin_prop->name = NULL;

    return OS_SUCCESS ;
}
