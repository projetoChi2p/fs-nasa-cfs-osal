#ifndef INCLUDE_OS_IMPL_TASK_H_
#define INCLUDE_OS_IMPL_TASK_H_

#include <osconfig.h>
#include "os-freertos.h"
#include "os-shared-idmap.h"
#include "os-shared-task.h"

typedef struct
{
    StaticTask_t* xTaskBuffer;
    TaskHandle_t xTask;
    char task_called_exit; // signals OS_TaskExit()/vTaskDelete(NULL)
    unsigned long obj_id; // see OS_ObjectIdToInteger()
    char obj_id_str[10]; // OSAL ID formatted as string, also used as FreeRTOS task name
} OS_impl_task_internal_record_t;

extern OS_impl_task_internal_record_t OS_impl_task_table[OS_MAX_TASKS];

#endif /* INCLUDE_OS_IMPL_TASK_H_ */
