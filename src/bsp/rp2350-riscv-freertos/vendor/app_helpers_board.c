#include <stdio.h>
#include <stddef.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "app_helpers.h"
#include "uart.h"
#include "gpio.h"
#include "ticks.h"

#define read_csr(csr)                                    \
    ({                                                         \
        uint32_t __val;                                        \
        __asm volatile("csrr %0, " #csr : "=r"(__val));       \
        __val;                                                 \
    })

#define write_csr(csr, val)                              \
    ({                                                         \
        uint32_t __val = (uint32_t)(val);                      \
        __asm volatile("csrw " #csr ", %0" :: "r"(__val));    \
    })

#define set_csr(csr, val)                                \
    __asm volatile("csrs " #csr ", %0" :: "r"((uint32_t)(val)))

#define clear_csr(csr, val)                              \
    __asm volatile("csrc " #csr ", %0" :: "r"((uint32_t)(val)))

// http://patorjk.com/software/taag/#p=display&f=Colossal&t=
//
// 888    888 8888888888        d8888 8888888b.
// 888    888 888              d88888 888   Y88b
// 888    888 888             d88P888 888    888
// 8888888888 8888888        d88P 888 888   d88P
// 888    888 888           d88P  888 8888888P"
// 888    888 888          d88P   888 888
// 888    888 888         d8888888888 888
// 888    888 8888888888 d88P     888 888



/* FBV 2024-11-27 This is the FreeRTOS heap for head_4.c policy
 * I may be allocate explicitly to enforce alignment or to
 * place it in an arbitraty memory region.
 */
#if (configAPPLICATION_ALLOCATED_HEAP == 1)
//__attribute__ ((section(".l2_scratchpad")))
//__attribute__ ((aligned (8)))
//__attribute__ ((section(".noinit.freertos_heap")))
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif


/***************************************************************************
 */
void HLP_vSystemRestart(uint32_t reset_type)
{
    while (1)
    {
        __asm volatile("nop");
    }
}

/***************************************************************************
 */
uint32_t HLP_uGetResetType(void)
{
    uint32_t reset_type;

    reset_type = HLP_RESET_TYPE_POWERON;

    return reset_type;
}

/***************************************************************************
 */
uint32_t HLP_vWatchdogEnable( uint32_t millis )
{
    uint32_t ticks;

    return millis;
}

/***************************************************************************
 */
void HLP_vWatchdogDisable( void )
{

}

/***************************************************************************
 */
void HLP_vWatchdogFeed( void )
{

}


// http://patorjk.com/software/taag/#p=display&f=Colossal&t=
//
// 888    d8b                              .d8888b.                                      .d888
// 888    Y8P                             d88P  "88b                                    d88P"
// 888                                    Y88b. d88P                                    888
// 888888 888 88888b.d88b.   .d88b.        "Y8888P"           88888b.   .d88b.  888d888 888888
// 888    888 888 "888 "88b d8P  Y8b      .d88P88K.d88P       888 "88b d8P  Y8b 888P"   888
// 888    888 888  888  888 88888888      888"  Y888P"        888  888 88888888 888     888
// Y88b.  888 888  888  888 Y8b.          Y88b .d8888b        888 d88P Y8b.     888     888
//  "Y888 888 888  888  888  "Y8888        "Y8888P" Y88b      88888P"   "Y8888  888     888
//                                                            888
//                                                            888
//                                                            888

#define DELAY_CYCLES_500_NS            ((uint32_t)(0.0000005 * CPU_FREQUENCY))
#define DELAY_CYCLES_1_MICRO           ((uint32_t)(DELAY_CYCLES_500_NS * 2U))
#define DELAY_CYCLES_5_MICRO           ((uint32_t)(DELAY_CYCLES_500_NS * 10U))
#define DELAY_CYCLES_50_MICRO          ((uint32_t)(DELAY_CYCLES_500_NS * 100U))
#define DELAY_CYCLES_150_MICRO         ((uint32_t)(DELAY_CYCLES_500_NS * 300U))
#define DELAY_CYCLES_250_MICRO         ((uint32_t)(DELAY_CYCLES_500_NS * 500U))
#define DELAY_CYCLES_500_MICRO         ((uint32_t)(DELAY_CYCLES_500_NS * 1000U))
#define DELAY_CYCLES_2MS               ((uint32_t)(DELAY_CYCLES_500_NS * 4000U))
#define DELAY_CYCLES_100MS             ((uint32_t)(DELAY_CYCLES_2MS * 50U))


static inline uint64_t read_cycle64(void)
{
    uint32_t hi1, hi2, lo;

    do {
        hi1 = read_csr(cycleh);
        lo  = read_csr(cycle);
        hi2 = read_csr(cycleh);
    } while (hi1 != hi2);

    return ((uint64_t)hi1 << 32) | lo;
}

#define read_hi_res_tick() read_cycle64()


/***************************************************************************
 */
static inline void minidelay(uint32_t n)
{
    volatile uint64_t cycles_end = read_hi_res_tick() + n ;
    while ( read_hi_res_tick() < cycles_end )
    {
        __asm volatile("nop");
    }
}


#if configGENERATE_RUN_TIME_STATS == 1

static uint64_t g_cycles_start;

/***************************************************************************
 */
void HLP_vSystemConfigPerfCounter(void)
{
    g_cycles_start = read_hi_res_tick();
}

/***************************************************************************
 */
uint32_t HLP_ulSystemGetPerfCounter(void)
{
    return (uint32_t)(read_hi_res_tick() - g_cycles_start);
}

#endif /* configGENERATE_RUN_TIME_STATS == 1 */

/*********************************************************************
 */
uint8_t HLP_u8GetCacheSettings(void) {
    uint8_t cache_settings = 0;

    return cache_settings;
}


// http://patorjk.com/software/taag/#p=display&f=Colossal&t=
//
// d8b        d88P
// Y8P       d88P
//          d88P
// 888     d88P  .d88b.
// 888    d88P  d88""88b
// 888   d88P   888  888
// 888  d88P    Y88..88P
// 888 d88P      "Y88P"


/***************************************************************************
 */
void HLP_vConsolePrintStringBaremetal( const char *sz )
{
    uart_puts(sz);
}

/***************************************************************************
 */
void HLP_vConsolePrintBytesBaremetal( const uint8_t *data, int size )
{
	for (int i = 0; i < size; i++)
	{
        uart_putc(*data++);
	}
}



// http://patorjk.com/software/taag/#p=display&f=Colossal&t=
//
// 888
// 888
// 888
// 888888 888d888 8888b.   .d8888b .d88b.
// 888    888P"      "88b d88P"   d8P  Y8b
// 888    888    .d888888 888     88888888
// Y88b.  888    888  888 Y88b.   Y8b.
//  "Y888 888    "Y888888  "Y8888P "Y8888



#ifdef FREERTOS_TRACE_ENABLED

/* FBV 2025-12-31 The routine below is meant to support
 * the FreeRTOS task trace, for instance, by adding the
 * following lines to *_defs/FreeRTOSConfig.h.in
 *
 * #if defined(FREERTOS_TRACE_ENABLED)
 *   void extern HLP_vPrintChar(const char, const char);
 *   #define configUSE_APPLICATION_TASK_TAG 1
 *   #define traceTASK_SWITCHED_IN()   HLP_vPrintChar((char) xTaskGetApplicationTaskTagFromISR(xTaskGetCurrentTaskHandle()),0)
 *   #define traceTASK_SWITCHED_OUT()  HLP_vPrintChar((char) (xTaskGetApplicationTaskTagFromISR(xTaskGetCurrentTaskHandle())),32)
 * #endif
 */

/***************************************************************************
 */
 void HLP_vPrintChar(const char c, const char out)
{
    if (c != 0)
    {
        uart_putc('~');
        uart_putc(c + out);
        uart_putc('\n');
    }
}

#endif /* FREERTOS_TRACE_ENABLED */


// http://patorjk.com/software/taag/#p=display&f=Colossal&t=
//
// 888                        888                     .d8888b.           d8b
// 888                        888                    d88P  "88b          Y8P
// 888                        888                    Y88b. d88P
// 88888b.   .d88b.   .d88b.  888  888 .d8888b        "Y8888P"           888 .d8888b  888d888
// 888 "88b d88""88b d88""88b 888 .88P 88K           .d88P88K.d88P       888 88K      888P"
// 888  888 888  888 888  888 888888K  "Y8888b.      888"  Y888P"        888 "Y8888b. 888
// 888  888 Y88..88P Y88..88P 888 "88b      X88      Y88b .d8888b        888      X88 888
// 888  888  "Y88P"   "Y88P"  888  888  88888P'       "Y8888P" Y88b      888  88888P' 888




/***************************************************************************
 */
__attribute__((weak))
void handle_plic_interrupt(void)
{
    // TODO we may need to weave PLIC here
    // FBV 2026-01-02 When using external IRQs, attached to PLIC,
    //                we need to handle PLIC claim/completion here.
    //                For now we are only using polled I/O and
    //                CLINT timer IRQ, hence we are neglecting PLIC
    //                management.
}

/***************************************************************************
 * Redefinition of the "weak" function defined in:
 * portable/GCC/RISC-V/portASM.S".
 * The only diference it is that now may handle external
 * (PLIC) interrupt requests.
 */
void freertos_risc_v_application_interrupt_handler(void) {
    uint64_t mepc;
    uint64_t mstatus;
    uint32_t mcause;

    uint8_t debug_buffer[32];

    // Fetch the cause value for the interrupt
    mepc = read_csr(mepc);
    mstatus = read_csr(mstatus);
    mcause = read_csr(mcause);

    uart_puts("Intr. mcause:");
    HLP_vPrintHexU64(debug_buffer, mcause);
    uart_puts((char*)debug_buffer);
    uart_puts(" mepc:");
    HLP_vPrintHexU64(debug_buffer, mepc);
    uart_puts((char*)debug_buffer);
    uart_puts(" mstatus:");
    HLP_vPrintHexU64(debug_buffer, mstatus);
    uart_puts((char*)debug_buffer);
    uart_puts("\n");

    while (1)
    {
        __asm volatile("nop");
    }
}


/***************************************************************************
 */
void freertos_risc_v_application_exception_handler(void )
{
    uint32_t mcause;
    uint32_t mepc;
    uint32_t mstatus;
    uint32_t misa;
    uint32_t mtval;

    uint8_t debug_buffer[32];

    // Fetch the cause value for the interrupt
    mcause  = read_csr(mcause);
    mepc    = read_csr(mepc);
    mstatus = read_csr(mstatus);
    misa    = read_csr(misa);
    mtval   = read_csr(mtval);

    uart_puts("Exc. mcause:");
    HLP_vPrintHexU64(debug_buffer, mcause);
    uart_puts((char*)debug_buffer);
    uart_puts(" mepc:");
    HLP_vPrintHexU64(debug_buffer, mepc);
    uart_puts((char*)debug_buffer);
    uart_puts(" mstatus:");
    HLP_vPrintHexU64(debug_buffer, mstatus);
    uart_puts((char*)debug_buffer);
    uart_puts(" misa:");
    HLP_vPrintHexU64(debug_buffer, misa);
    uart_puts((char*)debug_buffer);
    uart_puts(" mtval:");
    HLP_vPrintHexU64(debug_buffer, mtval);
    uart_puts((char*)debug_buffer);
    uart_puts("\n");


    uint32_t sp_val;
    // Grab the current Stack Pointer
    __asm volatile("mv %0, sp" : "=r"(sp_val));

    uart_puts("\n=== BACKTRACE (STACK DUMP) ===\n");
    uint32_t *stack = (uint32_t *)sp_val;

    // Scan the next 64 words (256 bytes) of the stack
    for(int i = 0; i < 64; i++)
    {
        uint32_t val = stack[i];

        // Filter: Only print if the value is in the Flash memory range (0x10000000 - 0x10FFFFFF)
        // AND it is an even number (RISC-V instructions are 16-bit or 32-bit aligned)
        if(((val & 0xFF000000) == 0x10000000) && ((val & 0x1) == 0))
        {
            uart_puts("Found at SP+"); uart_print8hex(i * 4); uart_puts(": ");
            uart_print8hex(val);
            uart_printnl();
        }
    }
    uart_puts("==============================\n");


    // Tell the compiler about the FreeRTOS global task pointer
    extern uint32_t * volatile pxCurrentTCB;

    uart_puts("\n=== THE SNIPER TRICK ===\n");

    if (pxCurrentTCB != 0) {
        // The first element of the TCB is ALWAYS the task's stack pointer!
        uint32_t *task_sp = (uint32_t *)(*pxCurrentTCB);

        // FreeRTOS RISC-V saves the 'ra' register exactly at index 1 (Offset 4)
        uint32_t true_ra = task_sp[1];

        uart_puts("True Caller (Return Address): ");
        uart_print8hex(true_ra);
        uart_printnl();
    } else {
        uart_puts("pxCurrentTCB is NULL!\n");
    }
    uart_puts("========================\n");

    while (1)
    {
        __asm volatile("nop");
    }
}


// http://patorjk.com/software/taag/#p=display&f=Colossal&t=
//
//                            .d888 d8b                      d88P                        888
//                           d88P"  Y8P                     d88P                         888
//                           888                           d88P                          888
//  .d8888b .d88b.  88888b.  888888 888  .d88b.           d88P         .d8888b   .d88b.  888888 888  888 88888b.
// d88P"   d88""88b 888 "88b 888    888 d88P"88b         d88P          88K      d8P  Y8b 888    888  888 888 "88b
// 888     888  888 888  888 888    888 888  888        d88P           "Y8888b. 88888888 888    888  888 888  888
// Y88b.   Y88..88P 888  888 888    888 Y88b 888       d88P                 X88 Y8b.     Y88b.  Y88b 888 888 d88P
//  "Y8888P "Y88P"  888  888 888    888  "Y88888      d88P              88888P'  "Y8888   "Y888  "Y88888 88888P"
//                                           888                                                         888
//                                      Y8b d88P                                                         888
//                                       "Y88P"                                                          888


extern void freertos_risc_v_trap_handler( void );
extern void freertos_vector_table( void );


/***************************************************************************
 */
void HLP_vSystemConfig(void)
{
    uint8_t debug_buffer[32];

    uart_init();

    pin_output(LED_PIN);


    // FBV 2026-01-02 When using external IRQs, attached to PLIC,
    //                we need to initialize PLIC here.
    //                For now we are only using polled I/O and
    //                CLINT timer IRQ, so we are negrecting PLIC
    //                setup.

    // Just some blinking for visually observing a reboot
    for (int i = 0; i < 3; i++)
    {
        pin_low(LED_PIN);
        for (int s = 0; s < 1; s++){
            minidelay(DELAY_CYCLES_100MS);
        }

        pin_high(LED_PIN);
        for (int s = 0; s < 1; s++){
            minidelay(DELAY_CYCLES_100MS);
        }
    }

    /**************************************************************
     * Other relativelly platform independent stuff
     **************************************************************/

    HLP_vConsoleInit();

    HLP_vConsolePrintStringBaremetal("*************************************\n");
    HLP_vConsolePrintStringBaremetal("*************************************\n");
    HLP_vConsolePrintStringBaremetal("*************************************\n");
    HLP_vConsolePrintFormattedBaremetal("%s [%d]:RP2350 RISC-V FreeRTOS\n", __func__, __LINE__);
    HLP_vConsolePrintFormattedBaremetal("%s [%d]: FreeRTOS Kernel %s \n", __func__, __LINE__, tskKERNEL_VERSION_NUMBER);
    #ifdef __GNUC__
    HLP_vConsolePrintFormattedBaremetal("%s [%d]: Compiler GCC %s\n", __func__, __LINE__, __VERSION__);
    #endif /* __GNUC__ */

    switch( HLP_uGetResetType() )
    {
        case HLP_RESET_TYPE_POWERON:
            HLP_vConsolePrintFormattedBaremetal("%s [%d]: Boot type POWERON\n", __func__, __LINE__);
            break;
        case HLP_RESET_TYPE_SOFTWARE:
            HLP_vConsolePrintFormattedBaremetal("%s [%d]: Boot type SOFTWARE\n", __func__, __LINE__);
            break;
        case HLP_RESET_TYPE_WATCHDOG:
            HLP_vConsolePrintFormattedBaremetal("%s [%d]: Boot type WATCHDOG\n", __func__, __LINE__);
            break;
        default:
            HLP_vConsolePrintFormattedBaremetal("%s [%d]: Boot type UNKNOWN\n", __func__, __LINE__);
            break;
    }

    uint32_t hartid = read_csr(mhartid);
    HLP_vConsolePrintFormattedBaremetal("%s [%d]: Executing at hart (ID): %d\r\n", __func__, __LINE__, (int)hartid);

    HLP_vConsolePrintFormattedBaremetal("%s [%d]: Size of char:%d short:%d int:%d long:%d long long:%d float:%d double:%d char*:%d void*:%d\r\n",
        __func__, __LINE__,
        sizeof(char),
        sizeof(short),
        sizeof(int),
        sizeof(long),
        sizeof(long long),
        sizeof(float),
        sizeof(double),
        sizeof(char*),
        sizeof(void*)
    );

    if (HLP_bIsBigEndian())
    {

        HLP_vConsolePrintFormattedBaremetal("%s [%d]: CPU is big endian.\r\n", __func__, __LINE__);
    }
    else
    {
        HLP_vConsolePrintFormattedBaremetal("%s [%d]: CPU is little endian.\r\n", __func__, __LINE__);
    }

    #if __riscv_flen == 0
        HLP_vConsolePrintFormattedBaremetal("%s [%d]: Not using FPU\n", __func__, __LINE__);
    #elif __riscv_flen == 32
        HLP_vConsolePrintFormattedBaremetal("%s [%d]: FPU is 32-bits\n", __func__, __LINE__);
    #elif __riscv_flen == 64
        HLP_vConsolePrintFormattedBaremetal("%s [%d]: FPU is 64-bits\n", __func__, __LINE__);
    #else
        #error Undefined FPU.
    #endif

    uint32_t mstatus;
    uint32_t misa;
    // Fetch the cause value for the interrupt
    mstatus = read_csr(mstatus);
    misa = read_csr(misa);

    HLP_vPrintHexU32(debug_buffer, misa);
    HLP_vConsolePrintFormattedBaremetal("%s [%d]: misa %s.\r\n", __func__, __LINE__, debug_buffer);
    HLP_vPrintHexU32(debug_buffer, mstatus);
    HLP_vConsolePrintFormattedBaremetal("%s [%d]: mstatus %s.\r\n", __func__, __LINE__, debug_buffer);

    HLP_vRtosBringUp();

    write_csr(mie, 0);
    write_csr(mip, 0);
    clear_csr(mstatus, 0x8);    /* bit 3 = MIE (Machine Interrupt Enable) */
    write_csr(mtvec, (uintptr_t)freertos_risc_v_trap_handler);

    // FreeRTOS will enable interrupts on mie     in xPortStartScheduler()
    //                                               __asm volatile( "csrs mie, %0" :: "r"(0x880) );
    // FreeRTOS will enable interrupts on mstatus in xPortStartFirstTask()
    //                                               addi    x5, x5, 0x08
    //                                               csrrw   x0, mstatus, x5
    //
}




// http://patorjk.com/software/taag/#p=display&f=Colossal&t=
//
// 888 d8b 888
// 888 Y8P 888
// 888     888
// 888 888 88888b.   .d8888b
// 888 888 888 "88b d88P"
// 888 888 888  888 888
// 888 888 888 d88P Y88b.
// 888 888 88888P"   "Y8888P


/***************************************************************************
 */
int __io_getchar(void) {
    char ch;

    ch = uart_getc();

    return ch;
}

/***************************************************************************
 */
int __io_putchar(int n) {
    char ch = n;

    uart_putc(ch);

    return ch;
}


/***************************************************************************
 */
int _fstat(int file, struct stat *st)
{
    UNUSED_ARGUMENT(file);

	st->st_mode = S_IFCHR;
	return 0;
}

/***************************************************************************
 */
int _isatty(int file)
{
    UNUSED_ARGUMENT(file);

	return 1;
}

/***************************************************************************
 */
int _lseek(int file, int ptr, int dir)
{
    UNUSED_ARGUMENT(file);
    UNUSED_ARGUMENT(ptr);
    UNUSED_ARGUMENT(dir);

	return 0;
}

/***************************************************************************
 */
int _open(char *path, int flags, ...)
{
    UNUSED_ARGUMENT(path);
    UNUSED_ARGUMENT(flags);

	return -1;
}

/***************************************************************************
 */
int _close(int file)
{
    UNUSED_ARGUMENT(file);

	return -1;
}

/***************************************************************************
 */
int _read(int file, char *ptr, int len)
{
    UNUSED_ARGUMENT(file);

	for (int i  = 0; i < len; i++)
	{
		*ptr++ = __io_getchar();
	}

    return len;
}

/***************************************************************************
 */
int _write(int file, char *ptr, int len)
{
    UNUSED_ARGUMENT(file);

	for (int i = 0; i < len; i++)
	{
		__io_putchar(*ptr++);
	}
	return len;
}

/***************************************************************************
 */
int _getpid(void)
{
	return 1;
}

/***************************************************************************
 */
int _kill(int pid, int sig)
{
    UNUSED_ARGUMENT(pid);
    UNUSED_ARGUMENT(sig);

	errno = EINVAL;
	return -1;
}
