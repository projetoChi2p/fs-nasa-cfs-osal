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

#include "mpfs_hal/mss_hal.h"
#include "drivers/mss/mss_mmuart/mss_uart.h"
#include "drivers/mss/mss_gpio/mss_gpio.h"
#include "drivers/mss/mss_rtc/mss_rtc.h"


/* FBV 2024-11-27 This is the FreeRTOS heap for head_4.c policy we 
 * are allocating explicitly to enforce alignment or to put it inside
 * arbitraty memory region, e.g. MPFS MSS scratchpad.
 */
#if (configAPPLICATION_ALLOCATED_HEAP == 1)
//__attribute__ ((section(".l2_scratchpad")))
//__attribute__ ((aligned (8)))
//__attribute__ ((section(".noinit.freertos_heap")))
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif



extern int main(void);
extern void freertos_risc_v_trap_handler( void );
extern void freertos_vector_table( void );

#if ( (MPFS_HAL_FIRST_HART<=0) && (0>=MPFS_HAL_LAST_HART) )
void e51(void) {
    (void)main();
}
#endif

#if ( (MPFS_HAL_FIRST_HART<=1) && (1>=MPFS_HAL_LAST_HART) )
void u54_1(void) {
    (void)main();
}
#endif




void HLP_vConsolePrintBytesBaremetal( const uint8_t *data, int size ) 
{
    MSS_UART_polled_tx(&g_mss_uart4_lo, data, size);
}


#define DELAY_CYCLES_500_NS            ((uint32_t)(0.0000005 * LIBERO_SETTING_MSS_COREPLEX_CPU_CLK))
#define DELAY_CYCLES_1_MICRO           ((uint32_t)(DELAY_CYCLES_500_NS * 2U))
#define DELAY_CYCLES_5_MICRO           ((uint32_t)(DELAY_CYCLES_500_NS * 10U))
#define DELAY_CYCLES_50_MICRO          ((uint32_t)(DELAY_CYCLES_500_NS * 100U))
#define DELAY_CYCLES_150_MICRO         ((uint32_t)(DELAY_CYCLES_500_NS * 300U))
#define DELAY_CYCLES_250_MICRO         ((uint32_t)(DELAY_CYCLES_500_NS * 500U))
#define DELAY_CYCLES_500_MICRO         ((uint32_t)(DELAY_CYCLES_500_NS * 1000U))
#define DELAY_CYCLES_2MS               ((uint32_t)(DELAY_CYCLES_500_NS * 4000U))
#define DELAY_CYCLES_100MS             ((uint32_t)(DELAY_CYCLES_2MS * 50U))


#define rdcycle() read_csr(cycle)

#if configGENERATE_RUN_TIME_STATS == 1

static uint64_t g_cycles_start;

void HLP_vSystemConfigPerfCounter(void)
{
    g_cycles_start = rdcycle();
}

uint32_t HLP_ulSystemGetPerfCounter(void)
{
    return rdcycle()-g_cycles_start;
}
#endif /* configGENERATE_RUN_TIME_STATS == 1 */


static inline void minidelay(uint32_t n)
{
    volatile uint64_t cycles_end = rdcycle() + n ;
    while (rdcycle() < cycles_end)
    {
        __asm volatile ( "NOP" );
    }
}

/*
    * Redefinition of the "weak" function defined in: 
    * fs-nasa-cfs-mission-v0/third-party/freertos-v10.5.1-gcc-riscv/portable/GCC/RISC-V/portASM.S".
    * The only diference it is that now handles external interruptions.
*/
void freertos_risc_v_application_interrupt_handler(void) {
    volatile uintptr_t mcause = read_csr(mcause);
    if (((mcause & MCAUSE_INT) == MCAUSE_INT) && ((mcause & MCAUSE_CAUSE) == IRQ_M_EXT)) {
        handle_m_ext_interrupt();
    }
    else {
        __asm volatile("csrr t0, mcause");  /* For viewing in the debugger only */
        __asm volatile("csrr t1, mepc");    /* For viewing in the debugger only */
        __asm volatile("csrr t2, mstatus"); /* For viewing in the debugger only */
        __asm volatile("j .");
    }
}

uint8_t rtc_wakeup_plic_IRQHandler(void) {
    MSS_RTC_clear_irq();

    return EXT_IRQ_DISABLE;
}

void enable_RTC_isr(uint32_t alarm_value_us) {
    MSS_RTC_reset_counter();

    MSS_RTC_set_binary_count_alarm(alarm_value_us, MSS_RTC_SINGLE_SHOT_ALARM);

    MSS_RTC_enable_irq();

    MSS_RTC_start();
}

void setup_RTC_isr() {
    PLIC_SetPriority(RTC_WAKEUP_PLIC, 2);
    (void)mss_config_clk_rst(MSS_PERIPH_RTC, MPFS_HAL_LAST_HART, PERIPHERAL_ON);

    // RTCCLK = 1 us
    SYSREG->RTC_CLOCK_CR &= ~0x00010000U;
    SYSREG->RTC_CLOCK_CR = LIBERO_SETTING_MSS_EXT_SGMII_REF_CLK / LIBERO_SETTING_MSS_RTC_TOGGLE_CLK;
    SYSREG->RTC_CLOCK_CR |= 0x00010000U;

    MSS_RTC_init(MSS_RTC_LO_BASE, MSS_RTC_BINARY_MODE, 0);

    MSS_RTC_reset_counter();
}

/*
    * The Interruption Sub-Routine that performs the FI.
*/
void FI_ISR(mss_uart_instance_t *this_uart)  {
    uint8_t     rx_buff [14];
    uint64_t    FI_addr;
    uint8_t     FI_btf;

    /*
    [0 .. 7] = Memory Address (ADDR)
    [8]      = Bit To Flip (BTF), value between 0 and 31.
    [9]      = \0
    [10 .. 13] = Not Used
    */

    MSS_UART_get_rx(&g_mss_uart1_lo, rx_buff, sizeof(rx_buff));

    memcpy(&FI_addr, rx_buff, sizeof(FI_addr));
    memcpy(&FI_btf, &rx_buff[8], sizeof(FI_btf));

    // Cast fi_addr to a pointer and flip the specified bit
    uint32_t *injection_address = (uint32_t *)FI_addr;
    *injection_address ^= (1U << FI_btf);
}


/*
    * Enables Fault Injection Mode, where it is set a 
    * Interrutption Routine in the UART1 port.
    * It is expected a 14 byte array containing information
    * to perform the injection, described as:
    *   [0 .. 7] = Memory Addres to do the injection
    *   [8]      = Bit to Flip (BTF), is expected a value between 0 and 31
    *   [9]         = '\0', string terminator
    *   [10 .. 13]  = Not Used
*/
void setup_FI_ISR() {
    PLIC_init();

    (void)mss_config_clk_rst(MSS_PERIPH_MMUART1, (uint8_t)MPFS_HAL_LAST_HART,  PERIPHERAL_ON);

    MSS_UART_init( &( g_mss_uart1_lo ), MSS_UART_921600_BAUD
                , MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT );

    MSS_UART_set_rx_handler(&g_mss_uart1_lo, FI_ISR, MSS_UART_FIFO_FOURTEEN_BYTES);

    /* It is required to set a priority for a PLIC interrupt, even if no other
     * interrupt is used */
    PLIC_SetPriority(MMUART1_PLIC, 2u);
    PLIC_SetPriority_Threshold(0u);

    MSS_UART_enable_irq(&g_mss_uart1_lo, MSS_UART_RBF_IRQ);
}


void HLP_vPrintChar(char c, int8_t out) {
    if (c == 0) return;
    static volatile char trace_task_tag[3];
    trace_task_tag[0] = '~';
    trace_task_tag[1] = c + out;
    trace_task_tag[2] = '\n';
    MSS_UART_polled_tx(&g_mss_uart1_lo, (uint8_t*)trace_task_tag, 3);
}


// called from ../osal/src/bsp/generic-freetos/src/bsp_start.c
void HLP_vSystemConfig(void) 
{

    /**************************************************************
     * PolarFire RISC-V CPU stuff
     **************************************************************/

    clear_soft_interrupt();
    set_csr(mie, MIP_MSIP);
    uint64_t hartid = read_csr(mhartid);

    /**************************************************************
     * PolarFire SoC peripherals stuff
     **************************************************************/
    (void)mss_config_clk_rst(MSS_PERIPH_GPIO0,   (uint8_t) MPFS_HAL_FIRST_HART, PERIPHERAL_ON);
    (void)mss_config_clk_rst(MSS_PERIPH_GPIO1,   (uint8_t) MPFS_HAL_FIRST_HART, PERIPHERAL_ON);
    (void)mss_config_clk_rst(MSS_PERIPH_GPIO2,   (uint8_t) MPFS_HAL_FIRST_HART, PERIPHERAL_ON);
    (void)mss_config_clk_rst(MSS_PERIPH_MMUART4, (uint8_t) MPFS_HAL_LAST_HART,  PERIPHERAL_ON);
    (void)mss_config_clk_rst(MSS_PERIPH_CFM,     (uint8_t) MPFS_HAL_FIRST_HART, PERIPHERAL_ON);

    MSS_UART_init( &( g_mss_uart4_lo ),
                   MSS_UART_115200_BAUD /* MSS_UART_921600_BAUD MSS_UART_115200_BAUD */,
                   MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT );

    MSS_GPIO_init(GPIO1_LO);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_9, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_9, 0);

    // Just some blinking for visually observing a reboot
    for (int i=0; i<3; i++)
    {
        MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_9, 0);
        for (int s=0; s<1; s++){
            minidelay(DELAY_CYCLES_100MS);
        }

        MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_9, 1);
        for (int s=0; s<1; s++){
            minidelay(DELAY_CYCLES_100MS);
        }
    }

    #ifdef ENABLE_FI
        setup_FI_ISR();
    #endif


    /**************************************************************
     * Other relativelly platform independent stuff
     **************************************************************/

    HLP_vConsoleInit();

    HLP_vConsolePrintFormattedBaremetal("*************************************\r\n");
    HLP_vConsolePrintFormattedBaremetal("*************************************\r\n");
    HLP_vConsolePrintFormattedBaremetal("*************************************\r\n");
    HLP_vConsolePrintFormattedBaremetal("%s [%d]: FreeRTOS Kernel is %s \r\n", __func__, __LINE__, tskKERNEL_VERSION_NUMBER);
    HLP_vConsolePrintFormattedBaremetal("%s [%d]: Compiler %s\r\n", __func__, __LINE__, __VERSION__);

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

    /**************************************************************
     * More PolarFire SoC specific stuff
     **************************************************************/

   /*
    * Change the RTC clock divisor, so RTC clock is 1MHz
    */
    //set_RTC_divisor();
    uint64_t cr = SYSREG->RTC_CLOCK_CR;
    uint64_t div = cr & ~(0x01U<<16);
    uint64_t rtcclk = LIBERO_SETTING_MSS_EXT_SGMII_REF_CLK / div;

    HLP_vConsolePrintFormattedBaremetal("%s [%d]: PolarFire SoC REFCLK:%lu RTCCLK:%lu CR:%lx div:%lu rtc:%lu\r\n",
        __func__, __LINE__, 
        LIBERO_SETTING_MSS_EXT_SGMII_REF_CLK,
        LIBERO_SETTING_MSS_RTC_TOGGLE_CLK,
        cr,
        div,
        rtcclk
    );

    if ((LIBERO_SETTING_DDRPHY_MODE & DDRPHY_MODE_MASK) != DDR_OFF_MODE) {
        HLP_vConsolePrintFormattedBaremetal("%s [%d]: Libero/PFSoC Configurator DDR address: 0x%08lx - 0x%08lx size: 0x%08lx\r\n",
            __func__, __LINE__, 
            LIBERO_SETTING_DDR_32_CACHE,
            LIBERO_SETTING_DDR_32_CACHE + LIBERO_SETTING_DDR_32_CACHE_SIZE - 1,
            LIBERO_SETTING_DDR_32_CACHE_SIZE
        );
    }
    else
    {
        HLP_vConsolePrintFormattedBaremetal("%s [%d]: DDR is disabled in Libero/PFSoC Configurator.\r\n",
            __func__, __LINE__
        );
    }

    /**************************************************************
     * FreeRTOS specific stuff
     **************************************************************/

    HLP_vRtosBringUp();
    // PolarFire RISC-V CPU supports both vectored or non-vectored IRQ dispatching
    // In vectored mode, timer function is dispatched directly
    // In non-vectored mode, handler conditionally calls timer function
    // Overall behaviour is similar/equivalent:
    //    - IRQ 0 (exception) with mcause 11 (ecall, a.k.a. FreeRTOS yield): task switch
    //    - IRQ 0 (exception) any mcause: hang
    //    - IRQ 7 (timer): tick, then task switch
    //    - any other IRQ: hang
    // Main difference seems to be that non-vectored handler switch to ISR stack memory
    __asm__ volatile ( "csrw mtvec, %0" : : "r" ( freertos_risc_v_trap_handler ) );
    //__asm__ volatile ( "csrw mtvec, %0" : : "r" ( ( uintptr_t ) freertos_vector_table | 0x1 ) );

}


/********************************************************************************
 The system calls placeholder functions bellow are based on auto-generated
 code from STMicroelectronics, under BSD licence.
 Copyright (c) 2020 STMicroelectronics.
 All rights reserved.

 This software component is licensed by ST under BSD 3-Clause license,
 the "License"; You may not use this file except in compliance with the
 License. You may obtain a copy of the License at:
                        opensource.org/licenses/BSD-3-Clause
 ********************************************************************************/

extern int __io_getchar(void) __attribute__((weak));

int __io_putchar(int ch) {
    uint8_t u8 = ch;

    MSS_UART_polled_tx(&g_mss_uart4_lo, &u8, 1);
    
    return ch;
}


int _fstat(int file, struct stat *st)
{
    UNUSED_ARGUMENT(file);

	st->st_mode = S_IFCHR;
	return 0;
}

int _isatty(int file)
{
    UNUSED_ARGUMENT(file);

	return 1;
}

int _lseek(int file, int ptr, int dir)
{
    UNUSED_ARGUMENT(file);
    UNUSED_ARGUMENT(ptr);
    UNUSED_ARGUMENT(dir);

	return 0;
}

int _open(char *path, int flags, ...)
{
    UNUSED_ARGUMENT(path);
    UNUSED_ARGUMENT(flags);

	return -1;
}

int _close(int file)
{
    UNUSED_ARGUMENT(file);

	return -1;
}

__attribute__((weak)) int _read(int file, char *ptr, int len)
{
    UNUSED_ARGUMENT(file);

	for (int i  = 0; i < len; i++)
	{
		*ptr++ = __io_getchar();
	}

    return len;
}

__attribute__((weak)) int _write(int file, char *ptr, int len)
{
    UNUSED_ARGUMENT(file);

	for (int i = 0; i < len; i++)
	{
		__io_putchar(*ptr++);
	}
	return len;
}



int _getpid(void)
{
	return 1;
}

int _kill(int pid, int sig)
{
    UNUSED_ARGUMENT(pid);
    UNUSED_ARGUMENT(sig);

	errno = EINVAL;
	return -1;
}
