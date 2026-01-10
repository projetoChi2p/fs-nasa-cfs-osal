/*
 * Copyright (c) 2026 Universidade Federal do Rio Grande do Sul
 * Copyright (c) 2020, Cobham Gaisler AB
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



#define MSTATUS_SIE    0x00000002 /* S-mode interrupt enable */
#define MSTATUS_MIE    0x00000008 /* M-mode interrupt enable */
#define MSTATUS_SPIE   0x00000020 /* Previous-state S-mode interrupt enable */
#define MSTATUS_MPIE   0x00000080 /* Previous-state M-mode interrupt enable */
#define MSTATUS_MPP    0x00001800 /* Machine Previous Priviledge mode. */
#define MSTATUS_FS     0x00006000 /* Floating-point status, enabled */

#define MISA_S_MODE    0x00040000 /* Supervisor mode implemented */

.equ CCTRL,       0x7c1

.section ".text.first"

.globl _start
.option norvc;
_start:

  la t0, __default_trap_handler
  csrw mtvec, t0

  # Is new mtvec effective?
  1:
  csrr    t1, mtvec
  bne t0, t1, 1b

  # flush and enable L1-I/D cache
  li t0, 0x1cf
  csrw CCTRL, t0

  # Disable all interrupts globally
  li   t0, (MSTATUS_MIE|MSTATUS_MPIE|MSTATUS_SIE|MSTATUS_SPIE)
  csrc mstatus, t0 # Clear

  # Disable all machine interrupt sources
  csrw   mie, zero
  # Clear pending interrupt bits
  csrw   mip, zero

  # Note: NOEL-V HP and GP have a Return Address Stack (RAS)
  #       with depth 8, which is cleared on cold-reset (rstn pin)
  #       At this moment we are using cold-reset.
  #       We should investigate if clearing RAS is required in
  #       a soft-reset.

  # Clear delegation registers
  # On warm-reset, may delegation be dirty?
  csrr   t0, misa
  li     t1, MISA_S_MODE
  and    t0, t0, t1
  beqz   t0, no_s_mode

  csrw   medeleg, zero
  csrw   mideleg, zero

no_s_mode:

  # Clear machine scratch
  csrw    mscratch, zero

  # Force known privilege return mode
  li      t0, MSTATUS_MPP
  csrs    mstatus, t0 # Set

  # Disable FPU state
  li    t0, MSTATUS_FS
  csrc  mstatus, t0 # Clear
#ifdef __riscv_flen
  csrw   fcsr, zero
#endif

  # Clean-up performance counters
  csrw   mcycle,   zero
  csrw   minstret, zero

  # Clean-up PMP
  # mtvec is being set here to jump back to this
  # boot-up sequence (goto label 2 forward) in 
  # case any trap is raised
  la t0, 2f
  csrw mtvec, t0

  csrw   pmpcfg0, zero
  csrw   pmpaddr0, zero
2:

#ifdef __riscv_flen
  # initialize FPU if we have one

  # mtvec is being set here to jump back to this
  # boot-up sequence (goto label 3 forward) in 
  # case any trap is raised
  la t0, 3f
  csrw mtvec, t0

  # Clear float register file
  fssr    x0
  fmv.s.x f0, x0
  fmv.s.x f1, x0
  fmv.s.x f2, x0
  fmv.s.x f3, x0
  fmv.s.x f4, x0
  fmv.s.x f5, x0
  fmv.s.x f6, x0
  fmv.s.x f7, x0
  fmv.s.x f8, x0
  fmv.s.x f9, x0
  fmv.s.x f10,x0
  fmv.s.x f11,x0
  fmv.s.x f12,x0
  fmv.s.x f13,x0
  fmv.s.x f14,x0
  fmv.s.x f15,x0
  fmv.s.x f16,x0
  fmv.s.x f17,x0
  fmv.s.x f18,x0
  fmv.s.x f19,x0
  fmv.s.x f20,x0
  fmv.s.x f21,x0
  fmv.s.x f22,x0
  fmv.s.x f23,x0
  fmv.s.x f24,x0
  fmv.s.x f25,x0
  fmv.s.x f26,x0
  fmv.s.x f27,x0
  fmv.s.x f28,x0
  fmv.s.x f29,x0
  fmv.s.x f30,x0
  fmv.s.x f31,x0
3:
#endif

  # Restore trap handler, direct mode
  la t0, __default_trap_handler
  csrw mtvec, t0

#ifdef __riscv_flen
  # Enable FPU
  li      t0, MSTATUS_FS
  csrs    mstatus, t0 # Set
  csrw    fcsr, zero
#endif

  # Clear register file
  li  x1, 0
  li  x2, 0
  li  x3, 0
  li  x4, 0
  li  x5, 0
  li  x6, 0
  li  x7, 0
  li  x8, 0
  li  x9, 0
  li  x10,0
  li  x11,0
  li  x12,0
  li  x13,0
  li  x14,0
  li  x15,0
  li  x16,0
  li  x17,0
  li  x18,0
  li  x19,0
  li  x20,0
  li  x21,0
  li  x22,0
  li  x23,0
  li  x24,0
  li  x25,0
  li  x26,0
  li  x27,0
  li  x28,0
  li  x29,0
  li  x30,0
  li  x31,0

  .option push
  .option norelax
  la gp, __global_pointer$
  .option pop

  la sp, __stack_top

  j startup_init

  ecall

  j .
