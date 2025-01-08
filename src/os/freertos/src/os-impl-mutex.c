#include <os-shared-globaldefs.h>
#include "os-impl-mutex.h"

// globals
OS_impl_mutex_internal_record_t OS_impl_mutex_table[OS_MAX_MUTEXES];

/*----------------------------------------------------------------
 * Function: OS_FreeRTOS_MutexAPI_Impl_Init
 *-----------------------------------------------------------------*/
int32 OS_FreeRTOS_MutexAPI_Impl_Init(void)
{
    memset(OS_impl_mutex_table, 0, sizeof(OS_impl_mutex_table));
    return OS_SUCCESS;
} /* end OS_FreeRTOS_MutexAPI_Impl_Init */

/*----------------------------------------------------------------
   Function: OS_MutSemCreate_Impl
 ------------------------------------------------------------------*/
int32 OS_MutSemCreate_Impl(const OS_object_token_t *token, uint32 options){
    OS_impl_mutex_internal_record_t *impl;
    
    UNUSED_ARGUMENT(options);

    impl = OS_OBJECT_TABLE_GET(OS_impl_mutex_table, *token);

    // returns Binary Semaphore which includes Priority
    //  Inheritance mechanism
    //  see: https://www.freertos.org/Real-time-embedded-RTOS-mutexes.html
    impl->xMutex = xSemaphoreCreateMutex();

    if(impl->xMutex == NULL){
        OS_printf("xSemaphoreCreateMutex failed\n");
        return OS_SEM_FAILURE;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
   Function: OS_MutSemGive_Impl
 ------------------------------------------------------------------*/
int32 OS_MutSemGive_Impl(const OS_object_token_t *token){
    OS_impl_mutex_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_mutex_table, *token);

    if(xSemaphoreGiveRecursive(impl->xMutex) != pdTRUE){
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
   Function: OS_MutSemTake_Impl
 ------------------------------------------------------------------*/
int32 OS_MutSemTake_Impl(const OS_object_token_t *token){
    OS_impl_mutex_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_mutex_table, *token);

    if(xSemaphoreTakeRecursive(impl->xMutex, portMAX_DELAY) != pdTRUE){
        return OS_ERROR;
    }

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
   Function: OS_MutSemDelete_Impl
 ------------------------------------------------------------------*/
int32 OS_MutSemDelete_Impl(const OS_object_token_t *token){
    OS_impl_mutex_internal_record_t *impl;

    impl = OS_OBJECT_TABLE_GET(OS_impl_mutex_table, *token);

    if(impl->xMutex == NULL){
        OS_printf("OS_MutSemDelete() non-existing mutex.\n");
        return OS_ERROR;
    }

    // @FIXME add OS_ERROR and unit test for this case:
    // "Do not delete a semaphore that has tasks blocked on it"
    // see: https://www.freertos.org/a00113.html#vSemaphoreDelete
    vSemaphoreDelete(impl->xMutex);

    /* Reset the table entry */
    memset(impl, 0, sizeof(*impl));

    return OS_SUCCESS;
}

/*----------------------------------------------------------------
   Function: OS_MutSemGetInfo_Impl
 ------------------------------------------------------------------*/
int32 OS_MutSemGetInfo_Impl(const OS_object_token_t *token, OS_mut_sem_prop_t *mut_prop){
    
    UNUSED_ARGUMENT(token);
    UNUSED_ARGUMENT(mut_prop);

    return OS_SUCCESS;
}
