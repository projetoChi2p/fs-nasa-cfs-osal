#ifndef INCLUDE_OS_FREERTOS_H
#define INCLUDE_OS_FREERTOS_H

// FreeRTOS headers
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "timers.h"

// standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>

// #include <os-shared-globaldefs.h>
#include "os-shared-common.h"
#include "os-shared-task.h"
#include "os-shared-queue.h"
#include "os-shared-printf.h"
#include "os-shared-timebase.h"
#include "os-shared-mutex.h"
#include "os-shared-countsem.h"
#include "os-shared-binsem.h"

#include "osapi.h"
#include "common_types.h"


/****************************************************************************************
                                     DEFINES
 ***************************************************************************************/

#ifndef MAX_CONSTANT
#define MAX_CONSTANT(a,b)  (a > b ? a : b)
#endif


/****************************************************************************************
                                    TYPEDEFS
 ***************************************************************************************/

typedef struct
{
    TaskHandle_t xCurrentIdlingTask;
    TickType_t   localtime_epoch_freertos;
    OS_time_t    localtime_epoch_osal;
} FreeRTOS_GlobalVars_t;

/****************************************************************************************
                                   GLOBAL DATA
 ***************************************************************************************/

extern FreeRTOS_GlobalVars_t FreeRTOS_GlobalVars;

/****************************************************************************************
                       FreeRTOS IMPLEMENTATION FUNCTION PROTOTYPES
 ***************************************************************************************/

int32 PSP_Console_Init(void);

int32 OS_FreeRTOS_TaskAPI_Impl_Init(void);
int32 OS_FreeRTOS_QueueAPI_Impl_Init(void);
int32 OS_FreeRTOS_BinSemAPI_Impl_Init(void);
int32 OS_FreeRTOS_CountSemAPI_Impl_Init(void);
int32 OS_FreeRTOS_MutexAPI_Impl_Init(void);
int32 OS_FreeRTOS_TimeBaseAPI_Impl_Init(void);
int32 OS_FreeRTOS_ModuleAPI_Impl_Init(void);
int32 OS_FreeRTOS_StreamAPI_Impl_Init(void);
int32 OS_FreeRTOS_DirAPI_Impl_Init(void);
int32 OS_FreeRTOS_FileSysAPI_Impl_Init(void);

int32 OS_FreeRTOS_TableMutex_Init(osal_objtype_t idtype);

UBaseType_t OS_FreeRTOS_MapOsalPriority(osal_priority_t priority);
osal_priority_t OS_MapFreeRTOSPriority(UBaseType_t priority);

/*-------------------------------------------------------------------------------------*/
/**
 * @brief Translates system local path file system path
 *
 * Translates a host local path to a file system token and a device relative path name.
 *
 * @note This is largely based on OSAL OS_TranslatePath().
 *
 * @note The buffer provided in the DevicePath argument is required to be at
 *       least OS_MAX_PATH_LEN characters in length.
 *
 * @param[in]  LocalPath        System virtual path name, @nonnull, previously obtained from OS_TranslatePath()
 * @param[out] FileSystem       DeviceBuffer to store native/translated path name @nonnull
 * @param[out] DevicePath       Buffer to store device relative path name @nonnull
 *
 * @return Execution status, see @ref OSReturnCodes
 * @retval #OS_SUCCESS @copybrief OS_SUCCESS
 * @retval #OS_INVALID_POINTER if either parameter is NULL
 * @retval #OS_FS_ERR_NAME_TOO_LONG if the filename component is too long
 * @retval #OS_FS_ERR_PATH_INVALID if either parameter cannot be interpreted as a path
 * @retval #OS_FS_ERR_PATH_TOO_LONG if either input or output pathnames are too long
 */
int32 OS_FreeRTOS_TranslateLocalPath(const char *LocalPath, OS_object_token_t *FileSystem, char *DevicePath);

#endif /* INCLUDE_OS_FREERTOS_H */
