OUTPUT_ARCH(riscv)
ENTRY(_start)

__STACK_SIZE = 2K;
__HEAP_SIZE = 8K;


MEMORY {
    FLASH (rx)  : ORIGIN = 0x10000000, LENGTH = 4M
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 512K
    RAM2  (rwx) : ORIGIN = 0x20080000, LENGTH = 8K
}

SECTIONS {
    .text : {
        __logical_binary_start = .;
        KEEP(*(.binary_info_header))
        __binary_info_header_end = .;
        KEEP(*(.embedded_block))
        __embedded_block_end = .;
        . = ALIGN(4);
        _stext = .;
        *(.text*)
        *(.rodata*)
        _etext = .;
        . = ALIGN(4);
    } > FLASH

    _sidata = LOADADDR(.data);

    .data : {
        . = ALIGN(4);
        _sdata = .;
        __global_pointer$ = . + 0x800;
        *(.sdata*)
        *(.time_critical*)
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT> FLASH

    .bss : {
        . = ALIGN(4);
        _sbss = .;
        *(.sbss*)
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    /* libc/libbcc heap
     * Note: OS may use its own managed heap on bss
     */
    .heap : {
        . = ALIGN(8);
        PROVIDE (end = .);
        PROVIDE (_end = .);
        . = . + __HEAP_SIZE;
    } > RAM

    .stack : {
        . = ALIGN(16);
        _stack_bottom = .;
        . += __STACK_SIZE;
        _stack_top = .;
    } > RAM

    ASSERT(__binary_info_header_end - __logical_binary_start <= 1024,
           "Binary info must be in first 1024 bytes of the binary")
    ASSERT(__embedded_block_end - __logical_binary_start <= 4096,
           "Embedded block must be in first 4096 bytes of the binary")
    ASSERT(_stack_top <= ORIGIN(RAM) + LENGTH(RAM),
           "Stack overflows RAM!")
}