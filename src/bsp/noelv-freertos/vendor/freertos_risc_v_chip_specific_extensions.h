/*
 * FreeRTOS Kernel V10.5.1
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/*
 * The FreeRTOS kernel's RISC-V port is split between the the code that is
 * common across all currently supported RISC-V chips (implementations of the
 * RISC-V ISA), and code that tailors the port to a specific RISC-V chip:
 *
 * + FreeRTOS\Source\portable\GCC\RISC-V\portASM.S contains the code that
 *   is common to all currently supported RISC-V chips.  There is only one
 *   portASM.S file because the same file is built for all RISC-V target chips.
 *
 * + Header files called freertos_risc_v_chip_specific_extensions.h contain the
 *   code that tailors the FreeRTOS kernel's RISC-V port to a specific RISC-V
 *   chip.  There are multiple freertos_risc_v_chip_specific_extensions.h files
 *   as there are multiple RISC-V chip implementations.
 *
 * !!!NOTE!!!
 * TAKE CARE TO INCLUDE THE CORRECT freertos_risc_v_chip_specific_extensions.h
 * HEADER FILE FOR THE CHIP IN USE.  This is done using the assembler's (not the
 * compiler's!) include path.  For example, if the chip in use includes a core
 * local interrupter (CLINT) and does not include any chip specific register
 * extensions then add the path below to the assembler's include path:
 * FreeRTOS\Source\portable\GCC\RISC-V\chip_specific_extensions\RISCV_MTIME_CLINT_no_extensions
 *
 */


#ifndef __FREERTOS_RISC_V_EXTENSIONS_H__
#define __FREERTOS_RISC_V_EXTENSIONS_H__

#define portasmHAS_SIFIVE_CLINT         1
#define portasmHAS_MTIME                1

#if __riscv_flen == 0

#define portasmADDITIONAL_CONTEXT_SIZE  0

.macro portasmSAVE_ADDITIONAL_REGISTERS
    /* No additional registers to save, so this macro does nothing. */
    .endm

.macro portasmRESTORE_ADDITIONAL_REGISTERS
    /* No additional registers to restore, so this macro does nothing. */
    .endm

#endif


#if __riscv_flen == 64
    #define portFPWORD_SIZE 8
    #define fpstore_x fsd
    #define fpload_x fld
#elif __riscv_flen == 32
    #define portFPWORD_SIZE 4
    #define fpstore_x fsw
    #define fpload_x flw

    #error This code remains untested on single-precision hardware float (bin32)
#endif

#if __riscv_flen > 0
    // We will save frcsr + 32 FPU regs
    #define portasmADDITIONAL_CONTEXT_SIZE ( 1 + ((32*portFPWORD_SIZE)/portWORD_SIZE) )

.macro portasmSAVE_ADDITIONAL_REGISTERS

    /* Make room for the additional registers. */
    addi sp, sp, -(portasmADDITIONAL_CONTEXT_SIZE * portWORD_SIZE)

    frcsr t0

    fpstore_x f0,  ( 1 * portWORD_SIZE + 0  * portFPWORD_SIZE )( sp )
    fpstore_x f1,  ( 1 * portWORD_SIZE + 1  * portFPWORD_SIZE )( sp )
    fpstore_x f2,  ( 1 * portWORD_SIZE + 2  * portFPWORD_SIZE )( sp )
    fpstore_x f3,  ( 1 * portWORD_SIZE + 3  * portFPWORD_SIZE )( sp )
    fpstore_x f4,  ( 1 * portWORD_SIZE + 4  * portFPWORD_SIZE )( sp )
    fpstore_x f5,  ( 1 * portWORD_SIZE + 5  * portFPWORD_SIZE )( sp )
    fpstore_x f6,  ( 1 * portWORD_SIZE + 6  * portFPWORD_SIZE )( sp )
    fpstore_x f7,  ( 1 * portWORD_SIZE + 7  * portFPWORD_SIZE )( sp )
    fpstore_x f8,  ( 1 * portWORD_SIZE + 8  * portFPWORD_SIZE )( sp )
    fpstore_x f9,  ( 1 * portWORD_SIZE + 9  * portFPWORD_SIZE )( sp )
    fpstore_x f10, ( 1 * portWORD_SIZE + 10 * portFPWORD_SIZE )( sp )
    fpstore_x f11, ( 1 * portWORD_SIZE + 11 * portFPWORD_SIZE )( sp )
    fpstore_x f12, ( 1 * portWORD_SIZE + 12 * portFPWORD_SIZE )( sp )
    fpstore_x f13, ( 1 * portWORD_SIZE + 13 * portFPWORD_SIZE )( sp )
    fpstore_x f14, ( 1 * portWORD_SIZE + 14 * portFPWORD_SIZE )( sp )
    fpstore_x f15, ( 1 * portWORD_SIZE + 15 * portFPWORD_SIZE )( sp )
    fpstore_x f16, ( 1 * portWORD_SIZE + 16 * portFPWORD_SIZE )( sp )
    fpstore_x f17, ( 1 * portWORD_SIZE + 17 * portFPWORD_SIZE )( sp )
    fpstore_x f18, ( 1 * portWORD_SIZE + 18 * portFPWORD_SIZE )( sp )
    fpstore_x f19, ( 1 * portWORD_SIZE + 19 * portFPWORD_SIZE )( sp )
    fpstore_x f20, ( 1 * portWORD_SIZE + 20 * portFPWORD_SIZE )( sp )
    fpstore_x f21, ( 1 * portWORD_SIZE + 21 * portFPWORD_SIZE )( sp )
    fpstore_x f22, ( 1 * portWORD_SIZE + 22 * portFPWORD_SIZE )( sp )
    fpstore_x f23, ( 1 * portWORD_SIZE + 23 * portFPWORD_SIZE )( sp )
    fpstore_x f24, ( 1 * portWORD_SIZE + 24 * portFPWORD_SIZE )( sp )
    fpstore_x f25, ( 1 * portWORD_SIZE + 25 * portFPWORD_SIZE )( sp )
    fpstore_x f26, ( 1 * portWORD_SIZE + 26 * portFPWORD_SIZE )( sp )
    fpstore_x f27, ( 1 * portWORD_SIZE + 27 * portFPWORD_SIZE )( sp )
    fpstore_x f28, ( 1 * portWORD_SIZE + 28 * portFPWORD_SIZE )( sp )
    fpstore_x f29, ( 1 * portWORD_SIZE + 29 * portFPWORD_SIZE )( sp )
    fpstore_x f30, ( 1 * portWORD_SIZE + 30 * portFPWORD_SIZE )( sp )
    fpstore_x f31, ( 1 * portWORD_SIZE + 31 * portFPWORD_SIZE )( sp )
    frcsr t0
    sw t0,         ( 1 * portWORD_SIZE + 32 * portFPWORD_SIZE )( sp )
.endm

.macro portasmRESTORE_ADDITIONAL_REGISTERS

    fpload_x f0,  ( 1 * portWORD_SIZE +  0 * portFPWORD_SIZE )( sp )
    fpload_x f1,  ( 1 * portWORD_SIZE +  1 * portFPWORD_SIZE )( sp )
    fpload_x f2,  ( 1 * portWORD_SIZE +  2 * portFPWORD_SIZE )( sp )
    fpload_x f3,  ( 1 * portWORD_SIZE +  3 * portFPWORD_SIZE )( sp )
    fpload_x f4,  ( 1 * portWORD_SIZE +  4 * portFPWORD_SIZE )( sp )
    fpload_x f5,  ( 1 * portWORD_SIZE +  5 * portFPWORD_SIZE )( sp )
    fpload_x f6,  ( 1 * portWORD_SIZE +  6 * portFPWORD_SIZE )( sp )
    fpload_x f7,  ( 1 * portWORD_SIZE +  7 * portFPWORD_SIZE )( sp )
    fpload_x f8,  ( 1 * portWORD_SIZE +  8 * portFPWORD_SIZE )( sp )
    fpload_x f9,  ( 1 * portWORD_SIZE +  9 * portFPWORD_SIZE )( sp )
    fpload_x f10, ( 1 * portWORD_SIZE + 10 * portFPWORD_SIZE )( sp )
    fpload_x f11, ( 1 * portWORD_SIZE + 11 * portFPWORD_SIZE )( sp )
    fpload_x f12, ( 1 * portWORD_SIZE + 12 * portFPWORD_SIZE )( sp )
    fpload_x f13, ( 1 * portWORD_SIZE + 13 * portFPWORD_SIZE )( sp )
    fpload_x f14, ( 1 * portWORD_SIZE + 14 * portFPWORD_SIZE )( sp )
    fpload_x f15, ( 1 * portWORD_SIZE + 15 * portFPWORD_SIZE )( sp )
    fpload_x f16, ( 1 * portWORD_SIZE + 16 * portFPWORD_SIZE )( sp )
    fpload_x f17, ( 1 * portWORD_SIZE + 17 * portFPWORD_SIZE )( sp )
    fpload_x f18, ( 1 * portWORD_SIZE + 18 * portFPWORD_SIZE )( sp )
    fpload_x f19, ( 1 * portWORD_SIZE + 19 * portFPWORD_SIZE )( sp )
    fpload_x f20, ( 1 * portWORD_SIZE + 20 * portFPWORD_SIZE )( sp )
    fpload_x f21, ( 1 * portWORD_SIZE + 21 * portFPWORD_SIZE )( sp )
    fpload_x f22, ( 1 * portWORD_SIZE + 22 * portFPWORD_SIZE )( sp )
    fpload_x f23, ( 1 * portWORD_SIZE + 23 * portFPWORD_SIZE )( sp )
    fpload_x f24, ( 1 * portWORD_SIZE + 24 * portFPWORD_SIZE )( sp )
    fpload_x f25, ( 1 * portWORD_SIZE + 25 * portFPWORD_SIZE )( sp )
    fpload_x f26, ( 1 * portWORD_SIZE + 26 * portFPWORD_SIZE )( sp )
    fpload_x f27, ( 1 * portWORD_SIZE + 27 * portFPWORD_SIZE )( sp )
    fpload_x f28, ( 1 * portWORD_SIZE + 28 * portFPWORD_SIZE )( sp )
    fpload_x f29, ( 1 * portWORD_SIZE + 29 * portFPWORD_SIZE )( sp )
    fpload_x f30, ( 1 * portWORD_SIZE + 30 * portFPWORD_SIZE )( sp )
    fpload_x f31, ( 1 * portWORD_SIZE + 31 * portFPWORD_SIZE )( sp )

    lw t0,        ( 1 * portWORD_SIZE + 32 * portFPWORD_SIZE )( sp )
    fscsr t0

    /* Remove space added for additional registers. */
    addi sp, sp, (portasmADDITIONAL_CONTEXT_SIZE * portWORD_SIZE )

.endm

#endif


#endif /* __FREERTOS_RISC_V_EXTENSIONS_H__ */
