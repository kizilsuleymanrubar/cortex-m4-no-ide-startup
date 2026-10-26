/*Specifying the firmware's entry point*/
ENTRY(Reset_Handler)


/*Detailing the available memory*/
MEMORY
{
    FLASH(rx) : ORIGIN = 0x08000000, LENGTH = 512K
    SRAM(rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}


/*Specifying the necessary heap and stack sizes*/
/*__max_heap_size = 0x200;*/
/*__max_stack_size = 0x400;*/


/*Specifying the stack pointer*/
_estack = ORIGIN(SRAM) + LENGTH(SRAM);


/*Defining output sections*/
SECTIONS
{

    .text :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector_tbl))
        *(.text)
        *(.text.*)
        *(.init)
        *(.fini)
        *(.rodata)
        *(.rodata.*)
        . = ALIGN(4);
    }> FLASH

    _etext = LOADADDR(.data); 

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data.*)
        *(.init_array*)
        *(.fini_array*)
        . = ALIGN(4);
        _edata = .;
    }> SRAM AT> FLASH   /*> VMA AT> LMA*/

    .bss : 
    {
        . = ALIGN(4);
        _sbss = .;
        __bss_start__ = _sbss;
        *(.bss)
        *(.bss.*)
        . = ALIGN(4);
        _ebss = .;
        __bss_end__ = _ebss;
        . = ALIGN(4);
        end = .;
        __end__ = .;
    }> SRAM AT> SRAM

}

