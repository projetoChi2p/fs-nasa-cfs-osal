/*******************************************************************************
 * Copyright (c) 2026 Universidade Federal do Rio Grande do Sul
 * Copyright (c) 2017-2020, Cobham Gaisler AB
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __RISCV_NOELV_DRIVERS__
#define __RISCV_NOELV_DRIVERS__

#include <stdint.h>


// grmon4> info sys
//   cpu0       Frontgrade Gaisler  NOEL-V RISC-V Processor    
//              AHB Master 0
//   ahbuart0   Frontgrade Gaisler  AHB Debug UART    
//              AHB Master 9
//              APB: ff986000 - ff986100
//              Baudrate 115200, AHB frequency 50.00 MHz
//   apbmst0    Frontgrade Gaisler  AHB/APB Bridge    
//              AHB: ff900000 - ffa00000
//   ahbram0    Frontgrade Gaisler  Single-port AHB SRAM module    
//              AHB: 00000000 - 10000000
//              32-bit SRAM: 128 kB @ 0x00000000
//   ahbrom0    Frontgrade Gaisler  Generic AHB ROM    
//              AHB: c0000000 - e0000000
//              32-bit ROM: 512 MB @ 0xc0000000
//   apbmst1    Frontgrade Gaisler  AHB/APB Bridge    
//              AHB: ff400000 - ff500000
//   apbmst2    Frontgrade Gaisler  AHB/APB Bridge    
//              AHB: ff500000 - ff600000
//   adev7      Frontgrade Gaisler  AMBA AHB/AXI Bridge    
//              AHB: 40000000 - 41000000
//   spim0      Frontgrade Gaisler  SPI Memory Controller    
//              AHB: fff42000 - fff43000
//              AHB: 41000000 - 42000000
//              IRQ: 10
//              SPI memory device read command: 0x0b
//   clint0     Frontgrade Gaisler  RISC-V ACLINT    
//              AHB: e0000000 - e0100000
//   plic0      Frontgrade Gaisler  RISC-V PLIC    
//              AHB: f8000000 - fc000000
//              4 contexts, 31 interrupt sources, 7 max priority
//   dm0        Frontgrade Gaisler  RISC-V Debug Module    
//              AHB: fe000000 - ff000000
//              hart0: ISA rv64imac, Modes M U
//                     i, m, a, c, zaamo, zalrsc, zca, zicntr, zifencei, zihpm
//                     zimop
//                     Stack pointer 0x0001fff0
//                     icache 4 * 4 kB, 32 B/line, rnd,
//                     dcache 4 * 4 kB, 32 B/line, rnd
//                     2 triggers,
//                     itrace 64 lines
//   uart0      Frontgrade Gaisler  Generic UART    
//              APB: ff900000 - ff900100
//              IRQ: 1
//              Baudrate 38343, FIFO debug mode available
//   gptimer0   Frontgrade Gaisler  Modular Timer Unit    
//              APB: ff908000 - ff908100
//              IRQ: 2
//              16-bit scaler, 2 * 32-bit timers, divisor 50
//   version0   Frontgrade Gaisler  Version and Revision Register    
//              APB: ff981000 - ff981100
//              Version 643, Revision 140
//   ahbstat0   Frontgrade Gaisler  AHB Status Register    
//              APB: ff982000 - ff982100
//              IRQ: 4
//   gpio0      Frontgrade Gaisler  General Purpose I/O port    
//              APB: ff983000 - ff983100



// http://patorjk.com/software/taag/#p=display&f=Colossal&t=
//
//  8888b.  .d8888b  88888b.d88b.  
//     "88b 88K      888 "888 "88b 
// .d888888 "Y8888b. 888  888  888 
// 888  888      X88 888  888  888 
// "Y888888  88888P' 888  888  888 

#define __nop() __asm__ __volatile__("nop")


#if __riscv_xlen == 64


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#define read_csr_by_number(csrnumber)          \
({                                             \
    unsigned long __tmp;                       \
    __asm__ __volatile__ (                     \
        "csrr %0, %1"                          \
        : "=r" (__tmp)                         \
        : "i" (csrnumber)                      \
        : "memory"                             \
    );                                         \
    __tmp;                                     \
})
#pragma GCC diagnostic pop


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#define read_csr_by_name(csrname) __extension__ \
({                                              \
    unsigned long __tmp;                        \
    __asm__ __volatile__ (                      \
        "csrr %0, " #csrname                    \
        : "=r" (__tmp)                          \
    );                                          \
    __tmp;                                      \
})
#pragma GCC diagnostic pop


#define riscv_set_csrs(csrname, val)           \
do {                                           \
    unsigned long __v = (unsigned long)(val);  \
    __asm__ __volatile__ (                     \
        "csrs " #csrname ", %0"                \
        :                                      \
        : "r" (__v)                            \
        : "memory"                             \
    );                                         \
} while (0)


#define riscv_set_csr(csrname, data) ({ \
    if (__builtin_constant_p(data) && !((data) & -32u)) { \
        asm volatile ("csrsi " #csrname ", %0" : : "i" (data)); \
    } else { \
        asm volatile ("csrs " #csrname ", %0" : : "r" (data)); \
    } \
})

#define riscv_clear_csr(csrname, data) ({ \
    if (__builtin_constant_p(data) && !((data) & -32u)) { \
        asm volatile ("csrci " #csrname ", %0" : : "i" (data)); \
    } else { \
        asm volatile ("csrc " #csrname ", %0" : : "r" (data)); \
    } \
})


#define set_csr_by_name(reg, bit) __extension__({ unsigned long __tmp; \
  asm volatile ("csrrs %0, " #reg ", %1" : "=r"(__tmp) : "rK"(bit)); \
  __tmp; })


#endif /* __riscv_xlen == 64 */






// http://patorjk.com/software/taag/#p=display&f=Colossal&t=
//
//  .d8888b.  8888888b.  888     888 
// d88P  Y88b 888   Y88b 888     888 
// 888    888 888    888 888     888 
// 888        888   d88P 888     888 
// 888        8888888P"  888     888 
// 888    888 888        888     888 
// Y88b  d88P 888        Y88b. .d88P 
//  "Y8888P"  888         "Y88888P"  

//   cpu0       Frontgrade Gaisler  NOEL-V RISC-V Processor    
//              AHB Master 0



#define CSR_MCAUSE      0x342
#define CSR_MEPC        0x341
#define CSR_MSTATUS     0x300
#define CSR_MISA        0x301
#define CSR_MTVAL       0x343
#define CSR_CYCLE       0xc00

#define CSR_NOELV_CCTRL 0x7c1 // Machine-Level CSRs, Custom read/write (0x7C0-0x7FF)
#define NOELV_CCTRL_DCS (0x3 <<  2) // data cache state, bit 0: active
#define NOELV_CCTRL_ICS (0x3 <<  0) // intruction cache state, bit 0: active

#define MSTATUS_MIE     0x8

#if __riscv_xlen == 64

#define CSR_MCAUSE_CAUSE      0x7FFFFFFFFFFFFFFF
#define CSR_MCAUSE_INTERRUPT  0x8000000000000000



/* Machine interrupt bit offset 
 * Common to both mip and mie registers.
 */
#define IRQ_M_SOFT      3U  /* Machine software interrupt */
#define IRQ_M_TIMER     7U  /* Machine timer interrupt */
#define IRQ_M_EXT       11U /* Machine external interrupt */


/* Machine interrupt mask
 * Common to both mip and mie registers
 */

 /* Timer interrupt enable.
 * Timer interrupts when mie.mtie, mip.mtip and mstatus.mie
 * are all 1, unless a software or an external interrupt request
 * is also pending and enabled.
 */
#define CSR_MIE_MTIE_BITS   0x80U

/* Software interrupt enable.
 * Software interrupts when mie.msie, mip.msip and mstatus.mie
 * are all 1, unless an external interrupt request is also 
 * pending and enabled.
 */
#define CSR_MIE_MSIE_BITS   0x8U
#define CSR_MIP_MSIP_BITS   (1U << IRQ_M_SOFT)


#endif /* __riscv_xlen == 64 */



// http://patorjk.com/software/taag/#p=display&f=Colossal&t=
//
//        d8888  .d8888b.  888      8888888 888b    888 88888888888 
//       d88888 d88P  Y88b 888        888   8888b   888     888     
//      d88P888 888    888 888        888   88888b  888     888     
//     d88P 888 888        888        888   888Y88b 888     888     
//    d88P  888 888        888        888   888 Y88b888     888     
//   d88P   888 888    888 888        888   888  Y88888     888     
//  d8888888888 Y88b  d88P 888        888   888   Y8888     888     
// d88P     888  "Y8888P"  88888888 8888888 888    Y888     888   

//   clint0     Frontgrade Gaisler  RISC-V ACLINT    
//              AHB: e0000000 - e0100000

union clint_time_regs {
    uint64_t val_64;
    uint32_t val_32[2];
};

typedef struct {
    /* one per hart */
    /* 0x0000 */
    uint32_t msip[4096];
    /* Low/High half of RISC-V machine-mode
     * timer comparator. This register is 
     * core-local. The comparison result is
     * routed to core's own interrupt line.
     */
    /* 0x4000 */
    union clint_time_regs mtimecmp[4096/2];
    /* 0x8000 */
    uint32_t reserved_8000[4094];
    /* Read/write access to the low/high half of 
    * RISC-V machine-mode timer. The register 
    * is shared between both cores.
    */
    /* 0xbff8 */
    union clint_time_regs mtime;
    /* 0xc000 */
    uint32_t reserved_c000[4096];
} MTimerDevice;

#define MTIMER0_BASE (0xe0000000U)
#define MTIMER0 ((MTimerDevice *)MTIMER0_BASE)


/***************************************************************************
 */
static inline void clear_clint_software_interrupts_enable(void)
{
    uint64_t hartid = read_csr_by_name(mhartid);
    MTIMER0->msip[hartid] = 0;
}




// http://patorjk.com/software/taag/#p=display&f=Colossal&t=
//
// 888       888          888            888           888                   
// 888   o   888          888            888           888                   
// 888  d8b  888          888            888           888                   
// 888 d888b 888  8888b.  888888 .d8888b 88888b.   .d88888  .d88b.   .d88b.  
// 888d88888b888     "88b 888   d88P"    888 "88b d88" 888 d88""88b d88P"88b 
// 88888P Y88888 .d888888 888   888      888  888 888  888 888  888 888  888 
// 8888P   Y8888 888  888 Y88b. Y88b.    888  888 Y88b 888 Y88..88P Y88b 888 
// 888P     Y888 "Y888888  "Y888 "Y8888P 888  888  "Y88888  "Y88P"   "Y88888 
//                                                                       888 
//                                                                  Y8b d88P 
//                                                                   "Y88P"  

/* Note: watchdog is, in fact, part of ACLINT 
 */

/*
 * NOEL-V from GRLIB GPL offer two watchdog options:
 *  - A watchdog integrated into the ACLINT interrupt controller aside the timers
 *  - A watchdog integrated into the Modular Timer Unit (grtimer) on APB bus
 * Both watchdogs raises external interrupts, via PLIC.
 * GRLIB GPL does not offer a reset controller fed by watchdog.
 * On default NOEL-V system from GRLIB GPL watchdor at grtimer is not present.
 * The ACLINT watchdog is present by default, but disabled at boot.
 * ACLINT watchdog can be enabled and disabled at any moment by software.
 * ACLINT watchdog has two stages:
 * - First stage raises an interrupt (WATCHDOG_HIRQ1) when timeout occurs.
 * - Second stage raises a second interrupt (WATCHDOG_HIRQ2) when timeout occurs 
 *   again after first stage.
 * Both interrupts can be handled at the BSP level, having no relevant meaning
 * to PSP.
 * Both flags of timer expired can be cleared by software.
 * Loading a counter value into the watchdog register reloads the watchdog counter.
 * Although, if the expired flags are not cleared, interrupts will be raised
 * instantly after reloading.
 * 
 * Scaling watchdog counter (milliseconds) to NOEL-V ACLINT  watchdog ticks depends
 * on RTC clock frequency (system clock/2) and clock divider (wdtickbit).
 * ACLINT watchdog counter is 10 bits.
 * e.g.: system clock               50 MHz
 *       RTC clock (/2)             25 MHz
 *                            tick bit      tick bit    tick bit    tick bit   tick bit
 *                           4 (default)       6           13          14           20
 *       divider              /32           /128        /16384      /32768     /2097152
 *       watchdog clock     781.3 kHz     195.3 kHz     1.5 kHz     762.9 Hz     11.9 Hz
 *       resolution          1.28 us       5.12 us      655.4 us     1.3 ms      83.9 ms
 *       10 bits counter      1.3 ms       5.3 ms       686.5 ms     1.4 s       85.9 s
 *       2 watchdog stages    2.7 ms      10.7 ms        1.4 s       2.7 s      171.8 s
 */


typedef struct {
    uint32_t control;
} WatchdogDevice;


#define WATCHDOG0_BASE (MTIMER0_BASE+0x10000U)
#define WATCHDOG0 ((WatchdogDevice *)WATCHDOG0_BASE)

#define WATCHDOG_ENABLED          0U
#define WATCHDOG_TRIGGERED_STAGE1 2U
#define WATCHDOG_TRIGGERED_STAGE2 3U
#define WATCHDOG_COUNTER          4U
#define WATCHDOG_COUNTER_SIZE    (13-4+1)

#define WATCHDOG_ENABLED_BITS    (1U<<WATCHDOG_ENABLED)
#define WATCHDOG_COUNTER_MAX     ((1U << WATCHDOG_COUNTER_SIZE)-1)
#define WATCHDOG_COUNTER_MASK    ((WATCHDOG_COUNTER_MAX<<WATCHDOG_COUNTER))

#define WATCHDOG_TRIGGERED_BITS ((1U<<WATCHDOG_TRIGGERED_STAGE1)|(1U<<WATCHDOG_TRIGGERED_STAGE2))

#define WATCHDOG_TICK_BIT       (20)
#define WATCHDOG_TICK_BIT_DIV   (1UL << (WATCHDOG_TICK_BIT+1))
#define WATCHDOG_RTC_DIV        (2UL)
#define WATCHDOG_STAGES         (2U)


#define WATCHDOG_MILLISECONDS_TO_TICKS(millis) (((CPU_FREQUENCY/WATCHDOG_RTC_DIV/1000U)*millis)/WATCHDOG_STAGES/WATCHDOG_TICK_BIT_DIV)
#define WATCHDOG_TICKS_TO_MILLISECONDS(ticks) ((WATCHDOG_STAGES*WATCHDOG_TICK_BIT_DIV*ticks)/(CPU_FREQUENCY/WATCHDOG_RTC_DIV/1000U))
#define WATCHDOG_MAX_MILLISECONDS WATCHDOG_TICKS_TO_MILLISECONDS(WATCHDOG_COUNTER_MAX+1)

/***************************************************************************
 */
static inline void set_watchdog_count_and_clear_events(uint16_t count)
{
    uint32_t control;
    if (count > WATCHDOG_COUNTER_MAX)
    {
        count = WATCHDOG_COUNTER_MAX;
    }

    control = WATCHDOG0->control;
    control &= (~WATCHDOG_TRIGGERED_BITS);
    control &= (~WATCHDOG_COUNTER_MASK);
    control |= (count<<WATCHDOG_COUNTER);
    
    WATCHDOG0->control = control;
}

/***************************************************************************
 */
static inline void set_watchdog_enable()
{
    uint32_t control;
    control = WATCHDOG0->control;
    control |= WATCHDOG_ENABLED_BITS;
    WATCHDOG0->control = control;
}

/***************************************************************************
 */
static inline void clear_watchdog_enable()
{
    uint32_t control;
    control = WATCHDOG0->control;
    control &= (~WATCHDOG_ENABLED_BITS);
    WATCHDOG0->control = control;
}

/***************************************************************************
 */
static inline void clear_watchdog_all()
{
    WATCHDOG0->control = 0;
}

static inline uint16_t get_watchdog_counter()
{
    uint32_t control;
    control = WATCHDOG0->control;
    control &= WATCHDOG_COUNTER_MASK;
    return (control>>WATCHDOG_COUNTER);
}


static inline uint32_t get_watchdog_state()
{
    return WATCHDOG0->control;
}




// http://patorjk.com/software/taag/#p=display&f=Colossal&t=
//
//        d8888 8888888b.  888888b.   888     888       d8888 8888888b. 88888888888 
//       d88888 888   Y88b 888  "88b  888     888      d88888 888   Y88b    888     
//      d88P888 888    888 888  .88P  888     888     d88P888 888    888    888     
//     d88P 888 888   d88P 8888888K.  888     888    d88P 888 888   d88P    888     
//    d88P  888 8888888P"  888  "Y88b 888     888   d88P  888 8888888P"     888     
//   d88P   888 888        888    888 888     888  d88P   888 888 T88b      888     
//  d8888888888 888        888   d88P Y88b. .d88P d8888888888 888  T88b     888     
// d88P     888 888        8888888P"   "Y88888P" d88P     888 888   T88b    888     

//   uart0      Frontgrade Gaisler  Generic UART    
//              APB: ff900000 - ff900100
//              IRQ: 1
//              Baudrate 38343, FIFO debug mode available


typedef struct
{
    volatile uint32_t data;
    volatile uint32_t status;
    volatile uint32_t ctrl;
    volatile uint32_t scaler;
} UartDevice;

#define UART0_BASE (0xff900000U)
#define UART0 ((UartDevice *)UART0_BASE)


/* Control register */
#define APBUART_CTRL_FA         (1u << 31)
#define APBUART_CTRL_BRK_SZ     ((10u-1u)<<16)
#define APBUART_CTRL_TE         (1u << 1)
#define APBUART_CTRL_RE         (1u << 0)
#define APBUART_CTRL_RESET      (APBUART_CTRL_FA|APBUART_CTRL_BRK_SZ|APBUART_CTRL_TE|APBUART_CTRL_RE)

/* Status register */
#define APBUART_STATUS_DR                  (1 << 0)
#define APBUART_STATUS_TF                  (1 << 9)
/* For APBUART implemented without FIFO */
#define APBUART_STATUS_HOLD_REGISTER_EMPTY (1 << 2)

enum {
    FIFO_UNKNOWN,
    FIFO_YES,
    FIFO_NO,
};

static int fifoinfo = FIFO_UNKNOWN;


/***************************************************************************
 */
static inline void uart_init_and_set_scaler(UartDevice *UARTx, const uint32_t scaler)
{
    UARTx->scaler = scaler;
    UARTx->ctrl = APBUART_CTRL_RESET;
    fifoinfo = FIFO_UNKNOWN;
}

/***************************************************************************
 */
static inline uint32_t uart_get_scaler(UartDevice *UARTx)
{
    return UARTx->scaler;
}


/***************************************************************************
 */
static inline void uart_send_char(UartDevice *UARTx, const char c)
{

    int fi;

    /* Use transmitter FIFO if available */
again:
    fi = fifoinfo;
    if (FIFO_YES == fi) {
        /* Transmitter FIFO full flag is available */
        while (UARTx->status & APBUART_STATUS_TF) {
            __nop();
        }
    } else if (FIFO_NO == fi) {
        /*
         * Transmitter "hold register empty" AKA "FIFO empty" flag is
         * available
         */
        while (!(UARTx->status & APBUART_STATUS_HOLD_REGISTER_EMPTY)) {
            __nop();
        }

    } else {
        /* First time: probe */
        if (UARTx->ctrl & APBUART_CTRL_FA) {
            fifoinfo = FIFO_YES;
        } else {
            fifoinfo = FIFO_NO;
        }
        goto again;
    }

    UARTx->data = c & 0xff;

}


/***************************************************************************
 */
static inline void uart_send_string(UartDevice *UARTx, const char *str)
{
    while (*(str) != '\0')
    {
        uart_send_char(UARTx, *(str));
        str++;
    }
}


/***************************************************************************
 */
static inline char uart_read(UartDevice *UARTx)
{
    while ( (UARTx->status & APBUART_STATUS_DR) == 0 );

    return UARTx->data & 0xff;
}






// http://patorjk.com/software/taag/#p=display&f=Colossal&t=
//
//  .d8888b.  8888888b. 8888888 .d88888b.  
// d88P  Y88b 888   Y88b  888  d88P" "Y88b 
// 888    888 888    888  888  888     888 
// 888        888   d88P  888  888     888 
// 888  88888 8888888P"   888  888     888 
// 888    888 888         888  888     888 
// Y88b  d88P 888         888  Y88b. .d88P 
//  "Y8888P88 888       8888888 "Y88888P"  

//   gpio0      Frontgrade Gaisler  General Purpose I/O port    
//              APB: ff983000 - ff983100

typedef struct
{
  volatile uint32_t DIN;
  volatile uint32_t DOUT;
  volatile uint32_t DIR;
} GpioDevice;

#define GPIO0_BASE (0xff983000U)
#define GPIO0 ((GpioDevice *)GPIO0_BASE)

#define GPIO_PIN16_OFFSET 16U
#define GPIO_PIN16_MASK (0x1U << GPIO_PIN16_OFFSET)

/***************************************************************************
 */
static inline void gpio_enable_output(GpioDevice *GPIOx, const uint32_t pin_mask)
{
  GPIOx->DIR |= pin_mask;
}

/***************************************************************************
 */
static inline void gpio_set(GpioDevice *GPIOx, const uint32_t pin_mask)
{
    GPIOx->DOUT |= pin_mask;
}

/***************************************************************************
 */
static inline void gpio_clear(GpioDevice *GPIOx, const uint32_t pin_mask)
{
    GPIOx->DOUT &= ~pin_mask;
}

// http://patorjk.com/software/taag/#p=display&f=Colossal&t=
//
// 8888888b.                            888    
// 888   Y88b                           888    
// 888    888                           888    
// 888   d88P .d88b.  .d8888b   .d88b.  888888 
// 8888888P" d8P  Y8b 88K      d8P  Y8b 888    
// 888 T88b  88888888 "Y8888b. 88888888 888    
// 888  T88b Y8b.          X88 Y8b.     Y88b.  
// 888   T88b "Y8888   88888P'  "Y8888   "Y888 

/* For this implementation, warm-reset is triggered by external 
 * glue-logic on GPIO
 */
static inline void system_warm_boot(void)
{
    gpio_enable_output(GPIO0, 0xF0000);
    gpio_set(GPIO0, 0xD0000);
}


#endif // __RISCV_NOELV_DRIVERS__
