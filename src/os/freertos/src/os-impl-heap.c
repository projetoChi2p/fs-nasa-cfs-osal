#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "common_types.h"
#include "osapi.h"
#include "os-shared-globaldefs.h"
#include "os-freertos.h"


/*----------------------------------------------------------------
 *
 *  Purpose: Implemented per internal OSAL API
 *           See prototype for argument/return detail
 *
 *-----------------------------------------------------------------*/
int32 OS_HeapGetInfo_Impl(OS_heap_prop_t *heap_prop){

    OS_CHECK_POINTER(heap_prop);

    /*
     * FreeRTOS heap stats has information on xAvailableHeapSpaceInBytes,
     * xNumberOfFreeBlocks, and xSizeOfLargestFreeBlockInBytes from vPortGetHeapStats().
     * However, this feature is available only in FreeRTOS 10.3.0 and later.
    */
    #if (tskKERNEL_VERSION_MAJOR > 10) || ((tskKERNEL_VERSION_MAJOR >= 10) && (tskKERNEL_VERSION_MINOR >= 3))
        HeapStats_t stats;

        /* FreeRTOS 10.3.0 or later */
        vPortGetHeapStats(&stats);

        heap_prop->free_blocks        = stats.xNumberOfFreeBlocks;
        heap_prop->free_bytes         = stats.xAvailableHeapSpaceInBytes;
        heap_prop->largest_free_block = stats.xSizeOfLargestFreeBlockInBytes;

    #else
        /* Older FreeRTOS: xPortGetFreeHeapSize() */
        heap_prop->free_bytes = xPortGetFreeHeapSize();
        heap_prop->free_blocks = 0;
        heap_prop->largest_free_block = 0;
    #endif

    return OS_SUCCESS;
}