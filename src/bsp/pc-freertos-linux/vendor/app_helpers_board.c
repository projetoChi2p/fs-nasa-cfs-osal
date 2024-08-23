#include <assert.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#include <unistd.h>
#include <time.h>
#include <sys/time.h>


#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "app_helpers.h"

extern SemaphoreHandle_t xStdioMutex;

#define LED_OFF 0
#define LED_ON  1

#ifdef _POSIX_TIMERS
#define HLP_USE_NANOS
#endif

#ifdef HLP_USE_NANOS
static struct timespec g_epoch_timespec;
#else
static struct timeval g_epoch_timeval;
#endif

#define BOARD_LE "CPU is little endian.\r\n"
#define BOARD_BE "CPU is big endian.\r\n"


void HLP_vSystemConfig(void) 
{
    assert(__SIZEOF_POINTER__ == 4);

    HLP_vConsoleInit();

    if (HLP_bIsBigEndian()) 
    {
        HLP_vConsolePrintBytesBspUnlocked((uint8_t*)BOARD_BE, sizeof(BOARD_BE));
    }
    else 
    {
        HLP_vConsolePrintBytesBspUnlocked((uint8_t*)BOARD_LE, sizeof(BOARD_LE));
    }

#ifdef HLP_USE_NANOS
    //HLP_vConsolePrintFormattedBspUnlocked("HLP_vSystemConfig() [%s:%d]\n", __FILE__, __LINE__);
    clock_gettime(CLOCK_MONOTONIC, &g_epoch_timespec);
#else
    //HLP_vConsolePrintFormattedBspUnlocked("HLP_vSystemConfig() [%s:%d]\n", __FILE__, __LINE__);
    gettimeofday(&g_epoch_timeval, NULL);
#endif

    HLP_vRtosBringUp();
}

void HLP_vLEDToggle(int led)
{
//    switch (led) {
//    case 0:
//        g_gpio_led_last_value = !g_gpio_led_last_value;
//        printf("HLP_vLEDToggle(led=%d,value=%d) [%s:%d]\n", led, g_gpio_led_last_value, __FILE__, __LINE__);
//    }
}


uint32_t HLP_u32HiResTickSubtract(uint32_t last, uint32_t now) {
    uint32_t elapsed_ticks;

    if (now < last) {
        elapsed_ticks = last - now;
    }
    else {
        elapsed_ticks = now - last;
    }

    return elapsed_ticks;
}

uint32_t HLP_u32GetHiResTick(void) {
    uint32_t now_time;
    #ifdef HLP_USE_NANOS
    struct timespec now_timespec;
    #else
    struct timeval now_timeval;
    #endif

    #ifdef HLP_USE_NANOS
    clock_gettime(CLOCK_MONOTONIC, &now_timespec);
    now_time = (((now_timespec.tv_sec - g_epoch_timespec.tv_sec) * 1000000000) + now_timespec.tv_nsec - g_epoch_timespec.tv_nsec);
    #else
    gettimeofday(&now_timeval, NULL);
    now_time = (((now_timeval.tv_sec - g_epoch_timeval.tv_sec) * 1000000) + now_timeval.tv_usec - g_epoch_timeval.tv_usec);
    #endif

    return now_time;
}

uint16_t HLP_u16PowerMilliVolts() {
    return 0;
}

uint16_t HLP_u16TemperatureCelsius() {
    return 0;
}

uint32_t HLP_u32GetFlashStatus(void) {
    return 0;
}

uint32_t HLP_u32GetMachineStatus(void) {
    return 0;
}

void HLP_vSmallDelay() {
    volatile int x;
    for (x=0; x<5000; x++) {
        __asm volatile ( "NOP" );
        __asm volatile ( "NOP" );
        __asm volatile ( "NOP" );
        __asm volatile ( "NOP" );
        __asm volatile ( "NOP" );
    }
}


void HLP_vConsolePrintBytesBaremetal( const uint8_t *data, int size )
{
    int i;
    for (i = 0; i < size; i++) 
    {
        printf("%c", (*data++));
    }
}
