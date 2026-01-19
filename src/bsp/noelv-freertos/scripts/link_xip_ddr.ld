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

/*******************************************************************************
 * Copyright (C) 2014-2020 Free Software Foundation, Inc.
 * Copying and distribution of this script, with or without modification,
 * are permitted in any medium without royalty provided the copyright
 * notice and this notice are preserved.  
 */

OUTPUT_FORMAT("elf64-littleriscv", "elf64-littleriscv", "elf64-littleriscv")
OUTPUT_ARCH(riscv)

ENTRY(_start)

/* Required amount of stack */
__STACK_SIZE = 4k;

/* Needs to be checked for each application
 * libc uses malloc()
 * libbcc uses malloc() for interrupt service setup
 */
__HEAP_SIZE = 1k;

/* Our system is all-ram, but we are enforcing 
 * RO placement to validate correct initialization
 * targetting potential future use of spimctrl.
 */
MEMORY
{
  RAM  (!rwx) : ORIGIN = 0x00000000, LENGTH = 128K
  PROM (rx)   : ORIGIN = 0xC0000000, LENGTH = 512K
  DDR  (rw)   : ORIGIN = 0x40000000, LENGTH = 640K
  CDS  (rw)   : ORIGIN = 0x400A0000, LENGTH = 384K
  ROM  (rx)   : ORIGIN = 0x41400000, LENGTH = 320K
}

/* We want NASA cFS/cFE/PSP CDS, as well as other
 * control data, to be placed in the same address
 * so that booting with different software
 * versions will find the data preserved between
 * boots. Data elements sizing and structure 
 * alignment/packing must also be canonicalized
 * at the software.
 */

REGION_ALIAS("REGION_TEXT",        ROM);
REGION_ALIAS("REGION_RODATA",      ROM);
REGION_ALIAS("REGION_DATA_VMA",    DDR);
REGION_ALIAS("REGION_DATA_LMA",    ROM);
REGION_ALIAS("REGION_BSS",         DDR);
REGION_ALIAS("REGION_CDS",         CDS);
/* 
REGION_ALIAS("REGION_EXTDATA_VMA", DDR);
REGION_ALIAS("REGION_EXTDATA_LMA", ROM);
*/

SECTIONS {
	.text : {
    *(.text.first)

    . = ALIGN(16);
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);

    . = ALIGN(16);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP(*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);

    . = ALIGN(16);
    *(.text*)

  } > REGION_TEXT


/*
	.rodata : {


  } > REGION_RODATA
*/

  .data : ALIGN(16)
  {
    __data_start_lma = LOADADDR(.data);
    __data_start = .;
    *(.data*)
    *(.gnu.linkonce.d.*)

    /* It is RO, but did not work on Flash XIP */
    . = ALIGN(16);
    *(.rodata*)

    . = ALIGN(16);
    *(.srodata*)

    . = ALIGN(16);
    PROVIDE( __global_pointer$ = . + 0x800 );
    *(.sdata*)
    *(.gnu.linkonce.s.*)

    . = ALIGN(16);
    __data_end = .;
  } > REGION_DATA_VMA AT > REGION_DATA_LMA


  .bss : 
  {
    . = ALIGN(16);
    __bss_start = .;
    *(.sbss*)
    *(.gnu.linkonce.sb.*)
    *(.bss*)
    *(.gnu.linkonce.b.*)
    *(COMMON)

  } > REGION_BSS
  
  . = ALIGN(16);
  __bss_end = .;


  /* bss-like section, but non initialized on boot and placed in a fixed address */
  .cds (NOLOAD) : ALIGN(16)
  {
      __noinit_cds_start = .; 
      *(.noinit.boot_info)
      *(.noinit.psp_reserved)
      . = ALIGN(16);
      __noinit_cds_end = .;
  } > REGION_CDS


  /* bss-like section, but non initialized on boot */
  .noinit (NOLOAD) : ALIGN(16)
  {
      __noinit_start = .; 
      *(.noinit.*)
      . = ALIGN(16);
      __noinit_end = .;
  } > REGION_BSS


  /* libc/libbcc heap 
   * Note: OS may use its own managed heap on bss
   */
  __heap_min = .;
  .heap :
  {
    . = ALIGN(16);
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + __HEAP_SIZE;
  } > REGION_BSS

  . = ALIGN(16);
  __heap_max = .;

  /* Main stack section, may be reused as FreeRTOS stack */
  .stack : ALIGN(16)
  {
    PROVIDE ( __stack_limit = . );
    . += __STACK_SIZE;
    . = ALIGN(16);
    PROVIDE ( __stack_top = . );
  } > REGION_BSS

}


PROVIDE( __data_source_start_lma    = LOADADDR(.data));
PROVIDE( __data_target_start        = ADDR(.data));
PROVIDE( __data_target_end          = ADDR(.data) + SIZEOF(.data));


PROVIDE( __bss_target_start         = ADDR(.bss));
PROVIDE( __bss_target_end           = ADDR(.bss) + SIZEOF(.bss));
