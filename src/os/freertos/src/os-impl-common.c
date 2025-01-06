/****************************************************************************************
                                    INCLUDE FILES
 ***************************************************************************************/

#include "os-freertos.h"
#include "os-shared-common.h"

FreeRTOS_GlobalVars_t FreeRTOS_GlobalVars = {0};


/****************************************************************************************
                                INITIALIZATION FUNCTION
 ***************************************************************************************/

/*---------------------------------------------------------------------------------------
   Name: OS_API_Init

   Purpose: Initialize the tables that the OS API uses to keep track of information
            about objects

   returns: OS_SUCCESS or OS_ERROR
---------------------------------------------------------------------------------------*/
int32 OS_API_Impl_Init(osal_objtype_t idtype)
{
    int32 return_code;

    return_code = OS_FreeRTOS_TableMutex_Init(idtype);
    if(return_code != OS_SUCCESS){
        return return_code;
    }

    switch (idtype){
        case OS_OBJECT_TYPE_OS_TASK:
            return_code = OS_FreeRTOS_TaskAPI_Impl_Init();
            break;
        case OS_OBJECT_TYPE_OS_QUEUE:
            return_code = OS_FreeRTOS_QueueAPI_Impl_Init();
            break;
        case OS_OBJECT_TYPE_OS_BINSEM:
            return_code = OS_FreeRTOS_BinSemAPI_Impl_Init();
            break;
        case OS_OBJECT_TYPE_OS_COUNTSEM:
            return_code = OS_FreeRTOS_CountSemAPI_Impl_Init();
            break;
        case OS_OBJECT_TYPE_OS_MUTEX:
            return_code = OS_FreeRTOS_MutexAPI_Impl_Init();
            break;
        case OS_OBJECT_TYPE_OS_MODULE:
            return_code = OS_FreeRTOS_ModuleAPI_Impl_Init();
            break;
        case OS_OBJECT_TYPE_OS_TIMEBASE:
            return_code = OS_FreeRTOS_TimeBaseAPI_Impl_Init();
            break;
        case OS_OBJECT_TYPE_OS_STREAM:
            return_code = OS_FreeRTOS_StreamAPI_Impl_Init();
            break;
        case OS_OBJECT_TYPE_OS_DIR:
            return_code = OS_FreeRTOS_DirAPI_Impl_Init();
            break;
        case OS_OBJECT_TYPE_OS_FILESYS:
            return_code = OS_FreeRTOS_FileSysAPI_Impl_Init();
            break;
        default:
            break;
    }

    return return_code;
} 

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
void OS_IdleLoop_Impl()
{
    if ( FreeRTOS_GlobalVars.xCurrentIdlingTask != NULL )
    {
        OS_printf("OS_IdleLoop_Impl() found a previous task idling engaged, which will be lost.\n");
    }

    FreeRTOS_GlobalVars.xCurrentIdlingTask = xTaskGetCurrentTaskHandle();
    vTaskSuspend(FreeRTOS_GlobalVars.xCurrentIdlingTask);
    FreeRTOS_GlobalVars.xCurrentIdlingTask = NULL;
}

/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
void OS_ApplicationShutdown_Impl()
{

    // Updating GlobalState will break OS_IdleLoop()
    // Resuming xCurrentIdlingTask will return to OS_IdleLoop()

    OS_SharedGlobalVars.GlobalState = OS_SHUTDOWN_MAGIC_NUMBER;

    if ( FreeRTOS_GlobalVars.xCurrentIdlingTask != NULL )
    {
        /*
        * Resume suspended task in OS_IdleLoop().
        * It would suspended again, immediately, if GlobalState 
        * not OS_SHUTDOWN_MAGIC_NUMBER.
        *
        * If shutdown is called too short from engaging idle loop,
        * then resume may be missed if occurs before suspend.
        */
        vTaskResume( FreeRTOS_GlobalVars.xCurrentIdlingTask );
        FreeRTOS_GlobalVars.xCurrentIdlingTask = NULL;
    }

    return;
} 
