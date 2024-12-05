#include <stdio.h>
#include <stddef.h>
#include <sys/stat.h>
#include <stdlib.h>
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


/* FBV 2024-11-27 This is the FreeRTOS heap for head_4.c policy we 
 * are allocating explicitly to enforce alignment or to put it inside
 * arbitraty memory region, e.g. MPFS MSS scratchpad.
 */
#if (configAPPLICATION_ALLOCATED_HEAP == 1)
//__attribute__ ((section(".l2_scratchpad")))
//__attribute__ ((aligned (8)))
//__attribute__ ((section(".data.freertos_head")))
uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
#endif

#define BOARD_LE "CPU is little endian.\r\n"
#define BOARD_BE "CPU is big endian.\r\n"

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




void HLP_vConsolePrintBytesBaremetal( const uint8_t *data, int size ) {
    MSS_UART_polled_tx_string(&g_mss_uart4_lo, data); 
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


// // #define rdtime() read_csr(time)
#define rdcycle() read_csr(cycle)
// // #define rdinstret() read_csr(instret)


// // typedef unsigned long cycles_t;

// // static inline cycles_t get_cycles_inline(void)
// // {
// // 	cycles_t n;

// // 	__asm__ __volatile__ (
// // 		"rdtime %0"
// // 		: "=r" (n));
// // 	return n;
// // }
// // #define get_cycles get_cycles_inline

static inline void minidelay(uint32_t n)
{
    volatile uint64_t cycles_end = rdcycle() + n ;
    while (rdcycle() < cycles_end)
    {
        __asm volatile ( "NOP" );
    }
}


// void __delay(unsigned long cycles)
// {
// 	u64 t0 = get_cycles();

// 	while ((unsigned long)(get_cycles() - t0) < cycles)
// 		cpu_relax();
// }


// called from ../osal/src/bsp/generic-freetos/src/bsp_start.c
void HLP_vSystemConfig(void) {
    clear_soft_interrupt();
    set_csr(mie, MIP_MSIP);

    (void)mss_config_clk_rst(MSS_PERIPH_GPIO0, (uint8_t) MPFS_HAL_FIRST_HART, PERIPHERAL_ON);
    (void)mss_config_clk_rst(MSS_PERIPH_GPIO1, (uint8_t) MPFS_HAL_FIRST_HART, PERIPHERAL_ON);
    (void)mss_config_clk_rst(MSS_PERIPH_GPIO2, (uint8_t) MPFS_HAL_FIRST_HART, PERIPHERAL_ON);
    (void)mss_config_clk_rst(MSS_PERIPH_MMUART4, (uint8_t) MPFS_HAL_LAST_HART, PERIPHERAL_ON);
    (void)mss_config_clk_rst(MSS_PERIPH_CFM, (uint8_t) MPFS_HAL_FIRST_HART, PERIPHERAL_ON);

    MSS_UART_init( &( g_mss_uart4_lo ),
                   MSS_UART_115200_BAUD,
                   MSS_UART_DATA_8_BITS | MSS_UART_NO_PARITY | MSS_UART_ONE_STOP_BIT );

    char buffer [80];

    MSS_GPIO_init(GPIO1_LO);
    MSS_GPIO_config(GPIO1_LO, MSS_GPIO_9, MSS_GPIO_OUTPUT_MODE);
    MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_9, 0);

    // for (int i=0; i<5; i++)
    // {
    //     MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_9, 0);
    //     for (int s=0; s<5; s++){
    //         minidelay(DELAY_CYCLES_100MS);
    //     }

    //     MSS_GPIO_set_output(GPIO1_LO, MSS_GPIO_9, 1);
    //     for (int s=0; s<5; s++){
    //         minidelay(DELAY_CYCLES_100MS);
    //     }
    // }


    uint64_t hartid = read_csr(mhartid);

    printf("*** This is a printf() call from %s [%d] \r\n", __func__, __LINE__);

    snprintf(buffer, sizeof(buffer), "executing at hard (ID): %d", (int)hartid);
    MSS_UART_polled_tx_string(&g_mss_uart4_lo, buffer);
    MSS_UART_polled_tx_string(&g_mss_uart4_lo, "\r\n");

    snprintf(buffer, sizeof(buffer), "sizeof char:%d short:%d int:%d long:%d long long:%d float:%d double:%d\r\n",
        sizeof(char),
        sizeof(short),
        sizeof(int),
        sizeof(long),
        sizeof(long long),
        sizeof(float),
        sizeof(double)
    );
    MSS_UART_polled_tx_string(&g_mss_uart4_lo, buffer);


   /*
    * Change the RTC clock divisor, so RTC clock is 1MHz
    */
    //set_RTC_divisor();
    uint64_t cr = SYSREG->RTC_CLOCK_CR;
    uint64_t div = cr & ~(0x01U<<16);
    uint64_t rtcclk = LIBERO_SETTING_MSS_EXT_SGMII_REF_CLK / div;

    snprintf(buffer, sizeof(buffer), "REFCLK:%lu RTCCLK:%lu CR:%lx div:%lu rtc:%lu\r\n",
    LIBERO_SETTING_MSS_EXT_SGMII_REF_CLK,
    LIBERO_SETTING_MSS_RTC_TOGGLE_CLK,
    cr,
    div,
    rtcclk
    );
    MSS_UART_polled_tx_string(&g_mss_uart4_lo, buffer);

    HLP_vConsoleInit();

    HLP_vRtosBringUp();

    //__asm__ volatile ( "csrw mtvec, %0" : : "r" ( freertos_risc_v_trap_handler ) );
    __asm__ volatile ( "csrw mtvec, %0" : : "r" ( ( uintptr_t ) freertos_vector_table | 0x1 ) );

    if (HLP_bIsBigEndian()) 
    {
        HLP_vConsolePrintBytesBaremetal((uint8_t*)BOARD_BE, sizeof(BOARD_BE));
    }
    else 
    {
        HLP_vConsolePrintBytesBaremetal((uint8_t*)BOARD_LE, sizeof(BOARD_LE));
    }
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
}


int _fstat(int file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int _isatty(int file)
{
	return 1;
}

int _lseek(int file, int ptr, int dir)
{
	return 0;
}

int _open(char *path, int flags, ...)
{
	return -1;
}

int _close(int file)
{
	return -1;
}

__attribute__((weak)) int _read(int file, char *ptr, int len)
{
	for (int i  = 0; i < len; i++)
	{
		*ptr++ = __io_getchar();
	}

    return len;
}

__attribute__((weak)) int _write(int file, char *ptr, int len)
{

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
	errno = EINVAL;
	return -1;
}
