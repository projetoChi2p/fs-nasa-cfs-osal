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
#include "riscv_noelv_drivers.h"


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



// http://patorjk.com/software/taag/#p=display&f=Colossal&t=
//
//      888                                               
//      888                                               
//      888                                               
//  .d88888 888  888 88888b.d88b.  88888b.d88b.  888  888 
// d88" 888 888  888 888 "888 "88b 888 "888 "88b 888  888 
// 888  888 888  888 888  888  888 888  888  888 888  888 
// Y88b 888 Y88b 888 888  888  888 888  888  888 Y88b 888 
//  "Y88888  "Y88888 888  888  888 888  888  888  "Y88888 
//                                                    888 
//                                               Y8b d88P 
//                                                "Y88P"  


#define BOOT_INFO_MAGIC     0x0123B001U
#define BOOT_INFO_WARM_BOOT 0xD000B001U
#define BOOT_INFO_COLD_BOOT 0xD000B000U

__attribute__ ((section(".noinit.boot_info")))
struct
{
    volatile uint32_t magic;  
    volatile uint32_t watchdog_control_at_boot;
    volatile uint32_t user_software_reset;
    volatile uint32_t boot_count;
} boot_info;


/***************************************************************************
 */
void HLP_vSystemRestart(uint32_t reset_type)
{
    if (reset_type == HLP_RESET_TYPE_POWERON)
    {
        boot_info.user_software_reset = BOOT_INFO_COLD_BOOT;
    }
    else
    {
        boot_info.user_software_reset = BOOT_INFO_WARM_BOOT;
    }
    system_warm_boot();
    while (1)
    {
        __nop();
    }
}

/***************************************************************************
 */
uint32_t HLP_uGetResetType(void)
{
    uint32_t reset_type;

    reset_type = HLP_RESET_TYPE_POWERON;

    if (boot_info.magic == BOOT_INFO_MAGIC)
    {
        if (boot_info.user_software_reset == BOOT_INFO_COLD_BOOT)
        {
            reset_type = HLP_RESET_TYPE_POWERON;
        }
        else if (boot_info.user_software_reset == BOOT_INFO_WARM_BOOT)
        {
            reset_type = HLP_RESET_TYPE_SOFTWARE;
        }
        else if (boot_info.watchdog_control_at_boot & WATCHDOG_TRIGGERED_BITS)
        {
            reset_type = HLP_RESET_TYPE_WATCHDOG;
        }
    }

    return reset_type;
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


#define read_hi_res_tick() read_csr_by_name(cycle)

/***************************************************************************
 */
static inline void minidelay(uint32_t n)
{
    volatile uint64_t cycles_end = read_hi_res_tick() + n ;
    while ( read_hi_res_tick() < cycles_end )
    {
        __nop();
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
    return read_hi_res_tick()-g_cycles_start;
}

#endif /* configGENERATE_RUN_TIME_STATS == 1 */


#define CPU_ICACHE_ENABLED              (1<<4)
#define CPU_DCACHE_ENABLED              (1<<5)

/*********************************************************************
 */
uint8_t HLP_u8GetCacheSettings(void) {
    uint8_t cache_settings = 0;
    
    unsigned long cctrl;

    cctrl = read_csr_by_number(CSR_NOELV_CCTRL);

    if ( (cctrl & NOELV_CCTRL_DCS) == NOELV_CCTRL_DCS)
    {
        cache_settings |= CPU_DCACHE_ENABLED;
    }
    if ( (cctrl & NOELV_CCTRL_ICS) == NOELV_CCTRL_ICS)
    {
        cache_settings |= CPU_ICACHE_ENABLED;
    }
	
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
    uart_send_string(UART0, sz);
}

/***************************************************************************
 */
void HLP_vConsolePrintBytesBaremetal( const uint8_t *data, int size )
{
	for (int i = 0; i < size; i++)
	{
        uart_send_char(UART0, *data++);
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
        uart_send_char(UARTx0, '~');
        uart_send_char(UARTx0, c + out);
        uart_send_char(UARTx0, '\n');
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
__attribute__((weak)) void handle_m_ext_interrupt(void)
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
    volatile uint64_t mcause = read_csr_by_name(mcause);
    if (((mcause & CSR_MCAUSE_INT) == CSR_MCAUSE_INT) && ((mcause & CSR_MCAUSE_CAUSE) == IRQ_M_EXT)) {
        handle_m_ext_interrupt();
    }
    else {
        uint64_t mepc;
        uint64_t mstatus;

        uint8_t debug_buffer[32];

        // Fetch the cause value for the interrupt
        mepc = read_csr_by_number(CSR_MEPC);
        mstatus = read_csr_by_number(CSR_MSTATUS);
        
        uart_send_string(UART0, "Intr. mcause:");
        HLP_vPrintHexU64(debug_buffer, mcause);
        uart_send_string(UART0, (char*)debug_buffer);
        uart_send_string(UART0, " mepc:");
        HLP_vPrintHexU64(debug_buffer, mepc);
        uart_send_string(UART0, (char*)debug_buffer);
        uart_send_string(UART0, " mstatus:");
        HLP_vPrintHexU64(debug_buffer, mstatus);
        uart_send_string(UART0, (char*)debug_buffer);
        uart_send_string(UART0, "\n");

        while (1)
        {
            __nop();
        }
    }
}


/***************************************************************************
 */
void freertos_risc_v_application_exception_handler(void )
{
    uint64_t mcause;
    uint64_t mepc;
    uint64_t mstatus;
    uint64_t misa;
    uint64_t mtval;

    uint8_t debug_buffer[32];

    // Fetch the cause value for the interrupt
    mcause = read_csr_by_number(CSR_MCAUSE);
    mepc = read_csr_by_number(CSR_MEPC);
    mstatus = read_csr_by_number(CSR_MSTATUS);
    misa = read_csr_by_number(CSR_MISA);
    mtval = read_csr_by_number(CSR_MTVAL);

    uart_send_string(UART0, "Exc. mcause:");
    HLP_vPrintHexU64(debug_buffer, mcause);
    uart_send_string(UART0, (char*)debug_buffer);
    uart_send_string(UART0, " mepc:");
    HLP_vPrintHexU64(debug_buffer, mepc);
    uart_send_string(UART0, (char*)debug_buffer);
    uart_send_string(UART0, " mstatus:");
    HLP_vPrintHexU64(debug_buffer, mstatus);
    uart_send_string(UART0, (char*)debug_buffer);
    uart_send_string(UART0, " misa:");
    HLP_vPrintHexU64(debug_buffer, misa);
    uart_send_string(UART0, (char*)debug_buffer);
    uart_send_string(UART0, " mtval:");
    HLP_vPrintHexU64(debug_buffer, mtval);
    uart_send_string(UART0, (char*)debug_buffer);
    uart_send_string(UART0, "\n");


    riscv_clear_csr(mie,     CSR_MIE_MTIE_BITS);
    riscv_clear_csr(mstatus, CSR_MIE_MSIE_BITS);

    while (1)
    {
        __nop();
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
    // Initialize persistent data
    if (boot_info.magic != BOOT_INFO_MAGIC)
    {
        memset(&boot_info, 0, sizeof(boot_info));
        boot_info.magic = BOOT_INFO_MAGIC;
    }
    boot_info.watchdog_control_at_boot = get_watchdog_state();


    clear_watchdog_all();
    clear_clint_software_interrupts_enable();
    set_csr_by_name(mie, CSR_MIP_MSIP_BITS);
    uint64_t hartid = read_csr_by_name(mhartid);

    gpio_enable_output(GPIO0, GPIO_PIN16_MASK);

    // FBV 2026-01-02 When using external IRQs, attached to PLIC,
    //                we need to initialize PLIC here.
    //                For now we are only using polled I/O and
    //                CLINT timer IRQ, so we are negrecting PLIC
    //                setup.

    /*
     * Enable interrupts.
     */
    set_csr_by_name(mstatus, MSTATUS_MIE);

    // Just some blinking for visually observing a reboot
    for (int i = 0; i < 3; i++)
    {
        gpio_clear(GPIO0, GPIO_PIN16_MASK);
        for (int s = 0; s < 1; s++){
            minidelay(DELAY_CYCLES_100MS);
        }

        gpio_set(GPIO0, GPIO_PIN16_MASK);
        for (int s = 0; s < 1; s++){
            minidelay(DELAY_CYCLES_100MS);
        }
    }

    /**************************************************************
     * Other relativelly platform independent stuff
     **************************************************************/

    HLP_vConsoleInit();

    uart_set_scaler(UART0, CPU_FREQUENCY/8/460800);

    HLP_vConsolePrintStringBaremetal("*************************************\n");
    HLP_vConsolePrintStringBaremetal("*************************************\n");
    HLP_vConsolePrintStringBaremetal("*************************************\n");
    HLP_vConsolePrintFormattedBaremetal("%s [%d]: RISC-V NOEL-V FreeRTOS\n", __func__, __LINE__);
    HLP_vConsolePrintFormattedBaremetal("%s [%d]: FreeRTOS Kernel is %s \n", __func__, __LINE__, tskKERNEL_VERSION_NUMBER);
    HLP_vConsolePrintFormattedBaremetal("%s [%d]: Compiler GCC %s\n", __func__, __LINE__, __VERSION__);

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
        HLP_vConsolePrintFormattedBaremetal("%s [%d]: No FPU\n", __func__, __LINE__);
    #elif __riscv_flen == 32
        HLP_vConsolePrintFormattedBaremetal("%s [%d]: FPU is 32-bits\n", __func__, __LINE__);
    #elif __riscv_flen == 64
        HLP_vConsolePrintFormattedBaremetal("%s [%d]: FPU is 64-bits\n", __func__, __LINE__);
    #else
        #error Undefined FPU.
    #endif

    uint8_t debug_buffer[32];
    uint64_t mstatus;
    uint64_t misa;
    // Fetch the cause value for the interrupt
    mstatus = read_csr_by_number(CSR_MSTATUS);
    misa = read_csr_by_number(CSR_MISA);
    
    HLP_vPrintHexU64(debug_buffer, misa);
    HLP_vConsolePrintFormattedBaremetal("%s [%d]: misa %s.\r\n", __func__, __LINE__, debug_buffer);
    HLP_vPrintHexU64(debug_buffer, mstatus);
    HLP_vConsolePrintFormattedBaremetal("%s [%d]: mstatus %s.\r\n", __func__, __LINE__, debug_buffer);
 
    HLP_vRtosBringUp();
    __asm__ volatile ( "csrw mtvec, %0" : : "r" ( freertos_risc_v_trap_handler ) );

    riscv_set_csrs(mie, CSR_MIE_MTIE_BITS);
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

    ch = uart_read(UART0);
    
    return ch;
}

/***************************************************************************
 */
int __io_putchar(int n) {
    char ch = n;

    uart_send_char(UART0, ch);

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
