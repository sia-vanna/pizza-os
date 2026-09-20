ENTRY(_start)

PHDRS
{
  text PT_LOAD FLAGS(5);
  rodata PT_LOAD FLAGS(4);
  data PT_LOAD FLAGS(6);
  note PT_NOTE FLAGS(4);
}

SECTIONS
{
  . = 1M;

  .multiboot2 :
  {
    *(.multiboot2)
  } :text

  .note :
  {
    *(.note)
  } :note

  .text :
  {
    *(.text*)
  } :text

  .rodata :
  {
    *(.rodata*)
  } :rodata

  .data :
  {
    *(.data*)
  } :data

  .bss :
  {
    *(.bss*)
    *(COMMON)
  } :data
}