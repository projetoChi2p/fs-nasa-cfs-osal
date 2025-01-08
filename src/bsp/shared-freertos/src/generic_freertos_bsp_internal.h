#ifndef GENERIC_FREERTOS_BSP_INTERNAL_H
#define GENERIC_FREERTOS_BSP_INTERNAL_H

#include "osapi.h"
#include "bsp-impl.h"

#include "FreeRTOS.h"
#include "semphr.h"

#ifndef MAX_CONSTANT
#define MAX_CONSTANT(a,b)  (a > b ? a : b)
#endif


/*
** BSP types
*/
typedef struct
{
    SemaphoreHandle_t AccessMutex;
    TaskHandle_t bsp_main_task_handle;
    TaskHandle_t console_task_handle;
    TaskHandle_t timebase_task_handle;
} OS_BSP_GenericFreeRtosGlobalData_t;

/*
 * Global Data object
 */
extern OS_BSP_GenericFreeRtosGlobalData_t OS_BSP_GenericFreeRtosGlobal;


#endif /* GENERIC_FREERTOS_BSP_INTERNAL_H */
