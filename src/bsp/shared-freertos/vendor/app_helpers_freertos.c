#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>
#include <time.h>
#include <ctype.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h" // Used to retrieve timer task handle if OS_CONSOLE_TASK_REPORT_TASKS

#include "semphr.h"

#include "app_helpers.h"

#include "osapi.h"

#include "generic_freertos_bsp_internal.h" // Used to access OS_BSP_GenericFreeRtosGlobal if OS_CONSOLE_TASK_REPORT_TASKS

#define EXCP_M "M"
#define EXCP_O "O"


/* We are using the FreeRTOS tick as a "high resolution" time base
 * supporting CFE_PSP_Get_Timebase() and CFE_PSP_GetTime(), which in
 * turn supports performance and benchmark metrics, while the OSAL
 * timebase, via OS_TimeBaseCreate(), feeds a lower resolution time.
 * If required, even higher accuracy could be achieved adding hardware
 * systick to tick counter.
 * CFE_PSP_Get_Timebase() supports up to 64 bits counter, but we are 
 * reusing legacy code with 32 bits, which is enough for several weeks.
 */
volatile uint32_t g_uwTick;

/* This mutex gives access to serial comunication port and print buffer.
 * It is bypassed by calls to HLP_vConsolePrintBytesBaremetal()
 */
SemaphoreHandle_t xStdioMutex;
StaticSemaphore_t xStdioMutexBuffer;
#define HLP_CONSOLE_PRINTF_BUFFER_SIZE 256
char hlp_console_printf_buffer[HLP_CONSOLE_PRINTF_BUFFER_SIZE];


void vApplicationStackOverflowHook( TaskHandle_t xTask, char * pcTaskName )
{
    /* Run time stack overflow checking is performed if
    configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected. */
	/* The stack space has been execeeded for a task, considering allocating more. */

    
    
    int length;
    int i;
    int isnumber;
    

    length = strlen (pcTaskName);
    isnumber = 1;
    for (i=0;i<length; i++)
    {
        if (!isdigit( (int)(pcTaskName[i]) ))
        {
            isnumber = 0;
            break;
        }
    }
    char *osal_name;
    if (isnumber)
    {
        #ifndef OS_USE_TASK_NAME
            osal_name = "unknown";
        #else /* ! OS_USE_TASK_NAME */
            unsigned long ul;
            osal_id_t task_id;
            OS_task_prop_t task_prop;
            int osal_result;
            
            ul = atol(pcTaskName);
            task_id = OS_ObjectIdFromInteger(ul);
            osal_result = OS_TaskGetInfo(task_id, &task_prop);
            if (osal_result == OS_SUCCESS)
            {
                osal_name = task_prop.name;
            }
            else
            {
                osal_name = "unknown";
            }
        #endif /* ! OS_USE_TASK_NAME */
    }
    else
    {
        osal_name = "not OSAL";
    }

    HLP_vConsolePrintFormattedBaremetal("Stack overflow on %s/%s.\n", pcTaskName, osal_name);
    vAssertCalled( __FILE__, __LINE__ );
}


void vApplicationTickHook(void)
{
    // May be called inside ISR stack
    HLP_vIncTick();
}

void HLP_vIncTick() {
    g_uwTick += 1;
}

uint32_t HLP_u32GetLoResTick(void) 
{
    return g_uwTick;
}

void HLP_vRtosBringUp() 
{
    g_uwTick = 0;
#if (defined(__i386__) && defined(__linux__))
    //HLP_vConsolePrintFormattedBspUnlocked("HLP_vRtosBringUp(), PTHREAD_STACK_MIN=%d [%s:%d]\n", PTHREAD_STACK_MIN, __FILE__, __LINE__);
#endif
}

void HLP_vConsoleInit( void ) 
{
    xStdioMutex = xSemaphoreCreateMutexStatic( &xStdioMutexBuffer );
}

void HLP_vConsolePrintFormattedBaremetal( const char * Format, ... ) {
    va_list va;

    va_start(va, Format);
    vsnprintf(hlp_console_printf_buffer, HLP_CONSOLE_PRINTF_BUFFER_SIZE-1, Format, va);
    va_end(va);
    hlp_console_printf_buffer[HLP_CONSOLE_PRINTF_BUFFER_SIZE-1] = '\0';

    HLP_vConsolePrintBytesBaremetal( (uint8_t*)hlp_console_printf_buffer, strlen(hlp_console_printf_buffer) );
}


void HLP_vConsolePrintFormatted( const char * Format, ... ) {
    va_list va;

    xSemaphoreTake( xStdioMutex, portMAX_DELAY );

    va_start(va, Format);
    vsnprintf(hlp_console_printf_buffer, HLP_CONSOLE_PRINTF_BUFFER_SIZE-1, Format, va);
    va_end(va);
    hlp_console_printf_buffer[HLP_CONSOLE_PRINTF_BUFFER_SIZE-1] = '\0';

    HLP_vConsolePrintBytesBaremetal( (uint8_t*)hlp_console_printf_buffer, strlen(hlp_console_printf_buffer) );

    xSemaphoreGive( xStdioMutex );
}

void HLP_vConsolePrintBytesStdioUnlocked( const uint8_t *data, int size )
{
    xSemaphoreTake( xStdioMutex, portMAX_DELAY );
    HLP_vConsolePrintBytesBaremetal( data, size );
    xSemaphoreGive( xStdioMutex );
}


/*
 * Prototypes for the standard FreeRTOS application hook (callback) functions
 * implemented within this file.  See http://www.freertos.org/a00016.html .
 */
void vApplicationMallocFailedHook( void );
void vApplicationIdleHook( void );

void vApplicationTickHook( void );
void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    uint32_t * pulIdleTaskStackSize );
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     uint32_t * pulTimerTaskStackSize );



/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 * used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    uint32_t * pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
 * function then they must be declared static - otherwise they will be allocated on
 * the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ FREERTOS_IDLE_TASK_STACK_SIZE_WORDS ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
     * state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = FREERTOS_IDLE_TASK_STACK_SIZE_WORDS;
}


/* When configSUPPORT_STATIC_ALLOCATION is set to 1 the application writer can
 * use a callback function to optionally provide the memory required by the idle
 * and timer tasks.  This is the stack that will be used by the timer task.  It is
 * declared here, as a global, so it can be checked by a test that is implemented
 * in a different file. */
StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];


/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 * application must provide an implementation of vApplicationGetTimerTaskMemory()
 * to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     uint32_t * pulTimerTaskStackSize )
{
/* If the buffers to be provided to the Timer task are declared inside this
 * function then they must be declared static - otherwise they will be allocated on
 * the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
     * task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}


#define WALLCLOCK_TICKS_PER_SECOND (configTICK_RATE_HZ)

#ifdef OS_CONSOLE_TASK_REPORT_FILES

#include "os-shared-filesys.h"
#include "os-shared-idmap.h"


TickType_t g_last_files_report_ticks = 0;

#define OS_CONSOLE_TASK_REPORT_FILES_PERIOD_TICKS (WALLCLOCK_TICKS_PER_SECOND * 30)

struct
{
    char mountpoint[OS_MAX_PATH_LEN];
} g_known_volumes [OS_MAX_FILE_SYSTEMS] = {0};

void HLP_ReportFilesEntries(const char* pszPath, uint8 level)
{
    osal_id_t     DirId;
    osal_status_t OS_Status;
    os_dirent_t   DirEntry;
    size_t        DirLen;
    os_fstat_t    FileStat;
    char          FullPath[OS_MAX_PATH_LEN];

    strncpy(FullPath, pszPath, sizeof(FullPath) - 1);
    FullPath[sizeof(FullPath) - 1] = 0;
    DirLen = strlen(FullPath);
    if (DirLen < (sizeof(FullPath) - 2))
    {
        FullPath[DirLen] = '/';
        ++DirLen;
        FullPath[DirLen] = 0;
    }
    else
    {
        printf("\n> Name too long [%s:%d].\n", __func__, __LINE__);
        return;
    }

    OS_Status = OS_DirectoryOpen(&DirId, pszPath);
    if ( OS_Status == OS_SUCCESS )
    {

        /* Read each directory entry and stat the files */
        while (OS_DirectoryRead(DirId, &DirEntry) == OS_SUCCESS)
        {
            strncpy(&FullPath[DirLen], OS_DIRENTRY_NAME(DirEntry), sizeof(FullPath) - DirLen - 1);
            FullPath[sizeof(FullPath) - 1] = 0;

            OS_Status = OS_stat(FullPath, &FileStat);
            if (OS_Status != OS_SUCCESS)
            {
                printf("\n> Failed to stat entry '%s' %ld.\n", FullPath, (long)OS_Status);
            }
            else
            {
                printf("\t");
                for (int x = 0; x<level; x++)
                {
                    printf("|  ");
                }
                printf("+--");
                if ( OS_FILESTAT_ISDIR(FileStat) )
                {
                    printf("d ");
                    printf("%s\n", OS_DIRENTRY_NAME(DirEntry));
                }
                else
                {
                    printf("f ");
                    printf("%s (%d B)\n", OS_DIRENTRY_NAME(DirEntry), FileStat.FileSize);
                }
            }
        }

        OS_DirectoryClose(DirId);
    }
    else
    {
        printf("\n> Failed to open directory %ld.\n", (long)OS_Status);
    }

}

void HLP_ReportFilesIfOnTime(void)
{
    TickType_t now_ticks = xTaskGetTickCount();
    if (g_last_files_report_ticks > now_ticks)
    {
        g_last_files_report_ticks = now_ticks;   
    }

    if ( (now_ticks - g_last_files_report_ticks) > OS_CONSOLE_TASK_REPORT_FILES_PERIOD_TICKS )
    {

        os_fsinfo_t filesys_info;
        int32 osal_rc;

        memset(&filesys_info, 0, sizeof(filesys_info));

        osal_rc = OS_GetFsInfo(&filesys_info);
        if ( osal_rc == OS_SUCCESS)
        {
            printf("\n> FreeRTOS Filesystems:\n");
            printf("\tMax. file descr.:%lu\n", (unsigned long)filesys_info.MaxFds);
            printf("\tFree file descr.:%lu\n", (unsigned long)filesys_info.FreeFds);
            printf("\tMax. volumes:%lu\n", (unsigned long)filesys_info.MaxVolumes);
            printf("\tFree volumes:%lu\n", (unsigned long)filesys_info.FreeVolumes);

            OS_object_iter_t iter;
            OS_filesys_internal_record_t *filesys;
            int known_volume = 0;

            OS_ObjectIdIterateActive(OS_OBJECT_TYPE_OS_FILESYS, &iter);

            while (OS_ObjectIdIteratorGetNext(&iter))
            {
                filesys = OS_OBJECT_TABLE_GET(OS_filesys_table, iter.token);

                printf("\n> FreeRTOS Filesystem:\n");
                printf("\tDevice name: %s\n", filesys->device_name);
                printf("\tVolume name: %s\n", filesys->volume_name);
                printf("\tSystem mount point: %s (OS_FS_GetPhysDriveName)\n", filesys->system_mountpt);
                printf("\tVirtual mount point: %s\n", filesys->virtual_mountpt);
                switch (filesys->fstype)
                {
                    case OS_FILESYS_TYPE_UNKNOWN:
                        printf("\tFilesystem type: UNKNOWN\n");
                        break;
                    case OS_FILESYS_TYPE_FS_BASED:
                        printf("\tFilesystem type: FS_BASED (emulated)\n");
                        break;
                    case OS_FILESYS_TYPE_NORMAL_DISK:
                        printf("\tFilesystem type: NORMAL_DISK\n");
                        break;
                    case OS_FILESYS_TYPE_VOLATILE_DISK:
                        printf("\tFilesystem type: VOLATILE_DISK\n");
                        break;
                    case OS_FILESYS_TYPE_MTD:
                        printf("\tFilesystem type: MTD (Flash, EEPROM)\n");
                        break;
                    default:
                        printf("\tFilesystem type: ***ERROR***\n");
                        break;
                }
                
                // Can't retrieve stat in single pass due to locks
                strncpy( g_known_volumes[known_volume].mountpoint, filesys->virtual_mountpt, OS_MAX_PATH_LEN-1);
                g_known_volumes[known_volume].mountpoint[OS_MAX_PATH_LEN-1] = 0;
                known_volume += 1;

            }
            OS_ObjectIdIteratorDestroy(&iter);

            for ( int vol_index = 0; vol_index < known_volume; vol_index++ )
            {
                OS_statvfs_t StatBuf;

                printf("\n> Virtual mount point: %s\n", g_known_volumes[vol_index].mountpoint);

                memset(&StatBuf, 0, sizeof(StatBuf));
                osal_rc = OS_FileSysStatVolume(g_known_volumes[vol_index].mountpoint, &StatBuf);
                if (osal_rc == OS_SUCCESS)
                {
                    printf("\tBlock size: %lu\n", (unsigned long)StatBuf.block_size);
                    printf("\tTotal blocks: %lu\n", (unsigned long)StatBuf.total_blocks);
                    printf("\tFree blocks: %lu\n", (unsigned long)StatBuf.blocks_free);
                }
                else
                {
                    printf("\n> Failed to retrieve filesystem status.\n");
                }

                printf("\td %s/\n", g_known_volumes[vol_index].mountpoint);
                HLP_ReportFilesEntries(g_known_volumes[vol_index].mountpoint, 0);
            }
        }
        else
        {
            printf("\n> Failed to retrieve filesystems information.\n");
        }
        g_last_files_report_ticks = now_ticks;
    }
}


#endif /* OS_CONSOLE_TASK_REPORT_FILES */


#ifdef OS_CONSOLE_TASK_REPORT_TASKS

TickType_t g_last_tasks_report_ticks = 0;

#define TASK_STATUS_ARRAY_SIZE (OS_MAX_TASKS+10) // Give room for some non-osal pure FreeRTOS tasks, like idle and timer

TaskStatus_t g_task_status_array[TASK_STATUS_ARRAY_SIZE];
#define OS_CONSOLE_TASK_REPORT_TASKS_PERIOD_TICKS (WALLCLOCK_TICKS_PER_SECOND * 30)

// NASA cFS says: It is always a good idea to verify that no more 
// than 1/2 of the stack is used.
#define CFS_STACK_USAGE_WARNING_THRESH_PERCENT 50

/*
 * Macros used by vListTask to indicate which state a task is in.
 */
#define tskRUNNING_CHAR		( 'X' )
#define tskBLOCKED_CHAR		( 'B' )
#define tskREADY_CHAR		( 'R' )
#define tskDELETED_CHAR		( 'D' )
#define tskSUSPENDED_CHAR	( 'S' )
#define tskUNKNOWN_CHAR	    ( '?' )

void HLP_ReportTasksIfOnTime(void)
{
    TickType_t now_ticks = xTaskGetTickCount();

    if (g_last_tasks_report_ticks > now_ticks)
    {
        g_last_tasks_report_ticks = now_ticks;   
    }
    if ( (now_ticks - g_last_tasks_report_ticks) > OS_CONSOLE_TASK_REPORT_TASKS_PERIOD_TICKS )
    {
        uint32_t u32NumberOfTasks;
        uint32_t ulTotalRunTime;
        char cStatus;
        OS_task_prop_t task_prop;
        unsigned long ul;
        osal_id_t task_id;
        int osal_result;
        unsigned long stack_size_bytes;
        unsigned long stack_used_bytes;
        unsigned long stack_free_bytes;
        unsigned long stack_used_percent;
        char* osal_name;
        char* stack_level_warning;

        TaskHandle_t idle_task_handle;
        TaskHandle_t timer_task_handle;

        idle_task_handle = xTaskGetIdleTaskHandle();
        timer_task_handle = xTimerGetTimerDaemonTaskHandle();

        memset(g_task_status_array, 0, sizeof(g_task_status_array));
        u32NumberOfTasks = uxTaskGetSystemState( g_task_status_array, TASK_STATUS_ARRAY_SIZE, &ulTotalRunTime );

        //taskENTER_CRITICAL();
        printf("\n> FreeRTOS Tasks: %lu\n\n", (unsigned long)u32NumberOfTasks);
        printf("FreeRTOS                 OSAL                 S Pri           Stack\r\n");
        printf("No. Name                 Name                         Free   Max.     Used\r\n");
        OS_printf("--- -------------------- -------------------- - --- ------ ------ -----------\r\n");

        /* Create a human readable table from the binary data. */
        for( int x = 0; x < u32NumberOfTasks; x++ )
        {
            ul = 0;
            stack_size_bytes = 0;
            stack_used_bytes = 0;
            stack_used_percent = 0;
            osal_name = "-";
            stack_level_warning = "";
            ul = atol(g_task_status_array[ x ].pcTaskName);
            if (ul!=0) {
                task_id = OS_ObjectIdFromInteger(ul);
                osal_result = OS_TaskGetInfo(task_id, &task_prop);
                if (osal_result == OS_SUCCESS)
                {
                    osal_name = task_prop.name;
                    stack_size_bytes = task_prop.stack_size;
                }
            }
            else
            {
                if ( g_task_status_array[ x ].xHandle == idle_task_handle)
                {
                    stack_size_bytes = FREERTOS_IDLE_TASK_STACK_SIZE_WORDS * sizeof(StackType_t);;
                }
                else if ( g_task_status_array[ x ].xHandle == timer_task_handle)
                {
                    stack_size_bytes = configTIMER_TASK_STACK_DEPTH * sizeof(StackType_t);;
                }
                else if ( g_task_status_array[ x ].xHandle == OS_BSP_GenericFreeRtosGlobal.cfe_psp_task_handle)
                {
                    stack_size_bytes = PSP_CFE_TASK_STACK_SIZE_BYTES;
                }
                else if ( g_task_status_array[ x ].xHandle == OS_BSP_GenericFreeRtosGlobal.console_task_handle)
                {
                    stack_size_bytes = OS_UTILITYTASK_STACK_SIZE;  // assumption timebase uses utility-like stack
                }
                else if ( g_task_status_array[ x ].xHandle == OS_BSP_GenericFreeRtosGlobal.timebase_task_handle)
                {
                    stack_size_bytes = OS_TIMEBASE_TASK_STACK_SIZE;
                }
            }

            // FreeRTOS watermark is always free space
            // The closer to zero may overflow
            stack_free_bytes = (g_task_status_array[ x ].usStackHighWaterMark * sizeof(StackType_t));

            if (stack_size_bytes != 0)
            {
                stack_used_bytes = stack_size_bytes - stack_free_bytes;
                stack_used_percent = (stack_used_bytes*100)/stack_size_bytes;
                if (stack_used_percent > CFS_STACK_USAGE_WARNING_THRESH_PERCENT)
                {
                    stack_level_warning = "*** Warning level";
                }
            }


            switch( g_task_status_array[ x ].eCurrentState )
            {
                case eRunning:		cStatus = tskRUNNING_CHAR;
                                    break;

                case eReady:		cStatus = tskREADY_CHAR;
                                    break;

                case eBlocked:		cStatus = tskBLOCKED_CHAR;
                                    break;

                case eSuspended:	cStatus = tskSUSPENDED_CHAR;
                                    break;

                case eDeleted:		cStatus = tskDELETED_CHAR;
                                    break;

                case eInvalid:		/* Fall through. */
                default:			/* Should not get here, but it is included
                                    to prevent static checking errors. */
                                    cStatus = tskUNKNOWN_CHAR;
                                    break;
            }

            printf("%3lu %-20s %-20s %c %3lu %6lu %6lu %6lu %3lu%% %s\r\n",
                (unsigned long) g_task_status_array[ x ].xTaskNumber,
                g_task_status_array[ x ].pcTaskName,
                osal_name,
                cStatus,
                g_task_status_array[ x ].uxCurrentPriority, 
                stack_free_bytes,
                stack_size_bytes,
                stack_used_bytes,
                stack_used_percent,
                stack_level_warning
            );
        }

        g_last_tasks_report_ticks = now_ticks;
        //taskEXIT_CRITICAL();
    }
}

#endif /* OS_CONSOLE_TASK_REPORT_TASKS */

// INCLUDE_uxTaskGetStackHighWaterMark

// uxTaskGetSystemState()
//  TaskStatus_t 
//  vTaskGetInfo()
//  https://www.freertos.org/Documentation/02-Kernel/04-API-references/03-Task-utilities/01-uxTaskGetSystemState
// https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/08-Run-time-statistics
void vApplicationIdleHook( void )
{
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
     * to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
     * task.  It is essential that code added to this hook function never attempts
     * to block in any way (for example, call xQueueReceive() with a block time
     * specified, or call vTaskDelay()).  If application tasks make use of the
     * vTaskDelete() API function to delete themselves then it is also important
     * that vApplicationIdleHook() is permitted to return to its calling function,
     * because it is the responsibility of the idle task to clean up memory
     * allocated by the kernel to any task that has since deleted itself. */
}

void vApplicationDaemonTaskStartupHook( void )
{
    /* This function will be called once only, when the daemon task starts to
     * execute    (sometimes called the timer task).  This is useful if the
     * application includes initialisation code that would benefit from executing
     * after the scheduler has been started. */
}

void vAssertCalled( const char * const pcFileName,
                    unsigned long ulLine )
{
    static BaseType_t xPrinted = pdFALSE;
    volatile uint32_t ulSetToNonZeroInDebuggerToContinue = 0;

    /* Called if an assertion passed to configASSERT() fails.  See
     * https://www.FreeRTOS.org/a00110.html#configASSERT for more information. */

    /* Parameters are not used. */
    ( void ) ulLine;
    ( void ) pcFileName;


    taskENTER_CRITICAL();
    {
        /* Stop the trace recording. */
        if( xPrinted == pdFALSE )
        {
            xPrinted = pdTRUE;
            HLP_vConsolePrintFormattedBaremetal("vAssertCalled() [%s:%d]\n", pcFileName, ulLine);
        }

        /* You can step out of this function to debug the assertion by using
         * the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
         * value. */
        while( ulSetToNonZeroInDebuggerToContinue == 0 )
        {
            __asm volatile ( "NOP" );
            __asm volatile ( "NOP" );
        }
    }
    taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/


void vApplicationMallocFailedHook( void )
{
    /* vApplicationMallocFailedHook() will only be called if
     * configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
     * function that will get called if a call to pvPortMalloc() fails.
     * pvPortMalloc() is called internally by the kernel whenever a task, queue,
     * timer or semaphore is created.  It is also called by various parts of the
     * demo application.  If heap_1.c, heap_2.c or heap_4.c is being used, then the
     * size of the    heap available to pvPortMalloc() is defined by
     * configTOTAL_HEAP_SIZE in FreeRTOSConfig.h, and the xPortGetFreeHeapSize()
     * API function can be used to query the size of free heap space that remains
     * (although it does not provide information on how the remaining heap might be
     * fragmented).  See http://www.freertos.org/a00111.html for more
     * information. */
    vAssertCalled( __FILE__, __LINE__ );
}
