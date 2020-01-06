OUTPUT_FORMAT("binary")
ENTRY(KernelStart)
SECTIONS
{
  . = 0x00040000;
  __kernel_start = .;
  .text   : { __code_start = .;   *(.text); __code_end = .;               }
  . = ALIGN(0x1000);
  .data   : { __data_start = .;   *(.data); __data_end = .;               }
  .rodata : { __rodata_start = .; *(.rodata) *(.rdata); __rodata_end = .; }
  .bss    : { __bss_start = .;    *(.bss) *(COMMON); __bss_end = .;       }
  __kernel_end = .;
  __end = .;
}
