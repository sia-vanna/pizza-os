ENTRY(_start)
PHDRS
{
  text PT_LOAD FLAGS(5); /* R+X */
  note PT_NOTE;
  data PT_LOAD FLAGS(6); /* R+W */
}

SECTIONS
{
  . = 1M;
  .text : {
    *(.multiboot2)
    *(.text*)
  } :text

  .note : {
    *(.note)
  } :note

  .rodata : { *(.rodata*) } :text
  .data : { *(.data*) } :data
  .bss : { *(COMMON) *(.bss*) } :data
}