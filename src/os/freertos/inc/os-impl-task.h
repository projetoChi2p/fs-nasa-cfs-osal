#ifndef INCLUDE_OS_IMPL_TASK_H_
#define INCLUDE_OS_IMPL_TASK_H_

#include <osconfig.h>
#include "os-freertos.h"
#include "os-shared-idmap.h"
#include "os-shared-task.h"

typedef struct
{
    TaskHandle_t xTask;
    osal_id_t obj_id;
    char obj_id_str[10];
} OS_impl_task_internal_record_t;

extern OS_impl_task_internal_record_t OS_impl_task_table[OS_MAX_TASKS];

#endif /* INCLUDE_OS_IMPL_TASK_H_ */
