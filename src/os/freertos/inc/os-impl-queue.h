#ifndef INCLUDE_OS_IMPL_QUEUE_H_
#define INCLUDE_OS_IMPL_QUEUE_H_

#include <osconfig.h>
#include "os-freertos.h"
#include "os-shared-idmap.h"
#include "os-shared-queue.h"

typedef struct
{
    QueueHandle_t xQueue;
} OS_impl_queue_internal_record_t;

extern OS_impl_queue_internal_record_t OS_impl_queue_table[OS_MAX_QUEUES];

#endif /* INCLUDE_OS_IMPL_QUEUE_H_ */
