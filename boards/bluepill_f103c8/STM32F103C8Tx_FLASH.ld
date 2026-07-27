ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    RAM   (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
}

_Min_Heap_Size = 0x000;
_Min_Stack_Size = 0x400;
__flash_start__ = ORIGIN(FLASH);
__ram_start__ = ORIGIN(RAM);
_estack = ORIGIN(RAM) + LENGTH(RAM);
_sstack = _estack - _Min_Stack_Size;

SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        KEEP(*(.init))
        KEEP(*(.fini))
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } > FLASH

    _sidata = LOADADDR(.data);
    __flash_image_end__ = LOADADDR(.data) + SIZEOF(.data);

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        __bss_start__ = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
        __bss_end__ = .;
        __static_ram_end__ = .;
    } > RAM

    .noinit (NOLOAD) :
    {
        . = ALIGN(8);
        __noinit_start__ = .;
        *(.noinit)
        *(.noinit*)
        . = ALIGN(8);
        __noinit_end__ = .;
    } > RAM

    .heap (NOLOAD) :
    {
        . = ALIGN(8);
        __end__ = .;
        end = __end__;
        . = . + _Min_Heap_Size;
        __heap_end__ = .;
    } > RAM

    /DISCARD/ :
    {
        *(.comment)
        *(.note*)
    }

    ASSERT(__heap_end__ <= _sstack,
           "RAM overflow: data/heap collides with reserved main stack")
}
