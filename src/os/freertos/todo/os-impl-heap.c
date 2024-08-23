#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include "common_types.h"
#include "osapi.h"
//#include "osapi-os-core.h"
#include "os-shared-globaldefs.h"
#include "os-freertos.h"

//HeapStats_t heapStats;

int32 OS_HeapGetInfo_Impl(OS_heap_prop_t *heap_prop){
    /* 
     * OSAL for POSIX does not implement this feature.
     * OSAL for VxWorks and RTEMS fill free bytes, blocks, and largest free block.
     * OSAL for VxWorks uses stats from memory partition while RTEMS uses stats from malloc info.
     * FreeRTOS heap stats has information on xAvailableHeapSpaceInBytes, 
     * xNumberOfFreeBlocks, and xSizeOfLargestFreeBlockInBytes from vPortGetHeapStats(). 
     * However, this feature is available only in FreeRTOS 10.3.0 and later.
     */

    /*
    ** Not implemented yet
    */
    return OS_ERR_NOT_IMPLEMENTED;    
}