ENTRY(_start)

PHDRS
{
  text PT_LOAD FLAGS(5);
  note PT_NOTE;
  data PT_LOAD FLAGS(6);
}

SECTIONS
{
  . = 1M;

  .text :
  {
    *(.multiboot2)
    *(.text*)
    *(.rodata*)
  } :text

  .note :
  {
    *(.note)
  } :note

  .data :
  {
    *(.data*)
  } :data

  .bss :
  {
    *(COMMON)
    *(.bss*)
  } :data

  /DISCARD/ :
  {
    *(.note.GNU-stack)
    *(.eh_frame*)
    *(.comment)
  }
}