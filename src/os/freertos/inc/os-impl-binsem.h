#ifndef INCLUDE_OS_IMPL_BIN_SEM_H
#define INCLUDE_OS_IMPL_BIN_SEM_H

#include <osconfig.h>
#include "os-freertos.h"
#include "os-shared-idmap.h"
#include "os-shared-binsem.h"

typedef struct
{
    SemaphoreHandle_t xBinSem;
} OS_impl_bin_sem_internal_record_t;

extern OS_impl_bin_sem_internal_record_t OS_impl_bin_sem_table[OS_MAX_BIN_SEMAPHORES];

#endif /* INCLUDE_OS_IMPL_BIN_SEM_H */
