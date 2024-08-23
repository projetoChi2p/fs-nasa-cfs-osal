#ifndef INCLUDE_OS_IMPL_MUTEX_H
#define INCLUDE_OS_IMPL_MUTEX_H

#include <osconfig.h>
#include "os-freertos.h"
#include "os-shared-idmap.h"
#include "os-shared-mutex.h"

typedef struct
{
    SemaphoreHandle_t xMutex;
} OS_impl_mutex_internal_record_t;

extern OS_impl_mutex_internal_record_t OS_impl_mutex_table[OS_MAX_MUTEXES];

#endif /* INCLUDE_OS_IMPL_MUTEX_H */
