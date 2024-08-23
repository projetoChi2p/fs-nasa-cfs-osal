#ifndef INCLUDE_OS_IMPL_COUNT_SEM_H
#define INCLUDE_OS_IMPL_COUNT_SEM_H

#include <osconfig.h>
#include "os-freertos.h"
#include "os-shared-idmap.h"
#include "os-shared-countsem.h"

// @FIXME
// UBaseType_t is defined in portmacro.h
// We use a limit of signed long here but
// larger or smaller could be necessary
#define MAX_SEM_VALUE 0x7FFFFFFF

typedef struct
{
    SemaphoreHandle_t xCountSem;
} OS_impl_count_sem_internal_record_t;

extern OS_impl_count_sem_internal_record_t OS_impl_count_sem_table[OS_MAX_COUNT_SEMAPHORES];

#endif /* INCLUDE_OS_IMPL_COUNT_SEM_H */
