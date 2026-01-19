/*
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


#include <stdlib.h>
#include <stdint.h>

#include "riscv_noelv_drivers.h"

/********************************* Globals ***********************************/

/* BSS limits are defined in .ld linker script */
extern uint32_t __bss_target_start;
extern uint32_t __bss_target_end;

/* .data Load Memory Address (In ROM if ROM resident application) */
extern const uint32_t __data_source_start_lma;

/* Start/end of .data Run-time Memory Address */
extern uint32_t __data_target_start;
extern uint32_t __data_target_end;



/************************** Function prototypes ******************************/

/***********************************************
 * Toolchain libc initialization
 */
void __libc_init_array(void);

/***********************************************
 * User main routine
 */
int main(int argc, char *argv[]);



/**************************** Local functions *******************************/


/***********************************************
 * Copy .data from Load Memory Address (LMA) to 
 * Virtual Memory Address (VMA) if needed.
 */
static inline void __copy_data()
{
    const uint32_t *src =  &__data_source_start_lma;
    uint32_t *dst = &__data_target_start;
    uint32_t *stop = &__data_target_end;

    if (src == dst) 
    {
        return;
    }

    while (dst < stop) 
    {
        *dst++ = *src++;
    }
}


/***********************************************
 */
static inline void __zero_bss()
{
    uint32_t *start = &__bss_target_start;
    const uint32_t *stop = &__bss_target_end;

    while( start < stop )
    {
        *start++ = 0;
    }
}



/*************************** Exported functions *****************************/

__attribute__((weak, aligned(8), interrupt("machine")))
void __default_trap_handler(void)
{
    __asm__ __volatile__ ("csrr t0, mcause;\n"
                          "csrr t1, mepc;\n"
                          "csrr t2, mstatus;\n"
    );

    /* This handler is used during startup.
     * It should weave any spurious trap
     * and return.
     * It should not hang, altough an
     * exception is an abnormal situation and
     * may hang for debugging.
     */
    if (! (read_csr_by_name(mcause) & CSR_MCAUSE_INTERRUPT) )
    {
        /* Trap is exception */
        while(1) {
            __nop();
        }
    }
}



/***********************************************
 */
void startup_init()
{
    __copy_data();
    __zero_bss();

    __libc_init_array();

    main(0, NULL);
}
