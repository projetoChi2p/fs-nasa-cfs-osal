#include <os-shared-globaldefs.h>
#include "os-impl-countsem.h"

// globals
OS_impl_count_sem_internal_record_t OS_impl_count_sem_table[OS_MAX_COUNT_SEMAPHORES];

/*----------------------------------------------------------------
 * Function: OS_FreeRTOS_CountSemAPI_Impl_Init
 *-----------------------------------------------------------------*/
int32 OS_FreeRTOS_CountSemAPI_Impl_Init(void)
{
    memset(OS_impl_count_sem_table, 0, sizeof(OS_impl_count_sem_table));
    return OS_SUCCESS;
} /* end OS_FreeRTOS_CountSemAPI_Impl_Init */


/*----------------------------------------------------------------
   Function: OS_CountSemCreate_Impl
 ------------------------------------------------------------------*/
int32 OS_CountSemCreate_Impl(const OS_object_token_t *token, uint32 sem_initial_value, uint32 options){
    OS_impl_count_sem_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

    // verify initial value does not exceed limit
    if(sem_initial_value > MAX_SEM_VALUE){
        return OS_INVALID_SEM_VALUE;
    }

    impl->xCountSem = xSemaphoreCreateCounting(
        MAX_SEM_VALUE,
        sem_initial_value
    );

    if(impl->xCountSem == NULL){
        OS_printf("xSemaphoreCreateCounting failed\n");
        return OS_SEM_FAILURE;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
   Function: OS_CountSemGive_Impl
 ------------------------------------------------------------------*/
int32 OS_CountSemGive_Impl(const OS_object_token_t *token){
    OS_impl_count_sem_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

    if(xSemaphoreGive(impl->xCountSem) != pdTRUE){
        OS_DEBUG("OS_CountSemGive_Impl xSemaphoreGive error");
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
   Function: OS_CountSemTake_Impl
 ------------------------------------------------------------------*/
int32 OS_CountSemTake_Impl(const OS_object_token_t *token){
    OS_impl_count_sem_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

    if(xSemaphoreTake(impl->xCountSem, portMAX_DELAY) != pdTRUE){
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
   Function: OS_CountSemTimedWait_Impl
 ------------------------------------------------------------------*/
int32 OS_CountSemTimedWait_Impl(const OS_object_token_t *token, uint32 msecs){
    TickType_t ticks_to_wait;
    int32 status;

    OS_impl_count_sem_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

    if(OS_Milli2Ticks(msecs, (int *) &ticks_to_wait) != OS_SUCCESS){
        return OS_ERROR;
    }

    status = xSemaphoreTake(impl->xCountSem, ticks_to_wait);

    if(status == pdTRUE){
        return OS_SUCCESS;
    }else if(status == pdFALSE){
        return OS_SEM_TIMEOUT;
    }

    return OS_ERROR;
}

/*----------------------------------------------------------------
   Function: OS_CountSemDelete_Impl
 ------------------------------------------------------------------*/
int32 OS_CountSemDelete_Impl(const OS_object_token_t *token){
    OS_impl_count_sem_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_count_sem_table, *token);

    // @FIXME add OS_ERROR and unit test for this case:
    // "Do not delete a semaphore that has tasks blocked on it"
    // see: https://www.freertos.org/a00113.html#vSemaphoreDelete
    vSemaphoreDelete(impl->xCountSem);

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
   Function: OS_CountSemGetInfo_Impl
 ------------------------------------------------------------------*/
int32 OS_CountSemGetInfo_Impl(const OS_object_token_t *token, OS_count_sem_prop_t *count_prop){
    return OS_SUCCESS;
}
