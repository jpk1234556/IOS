/* link.ld - place kernel at 1MiB (0x100000) */
ENTRY(_start)
SECTIONS
{
  . = 0x00100000;
  .text : { *(.multiboot) *(.text*) }
  .rodata : { *(.rodata*) }
  .data : { *(.data*) }
  .bss : {
    __bss_start = .;
    *(.bss*)
    __bss_end = .;
  }
  _stack_top = 0x00200000; /* 1MiB stack size - adjust if desired */
}