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

<<<<<<< Updated upstream
  .rodata : { *(.rodata*) } :text
  .data : { *(.data*) } :data
  .bss : { *(COMMON) *(.bss*) } :data
=======
  .data :
  {
    *(.data*)
  } :data

  .bss :
  {
    *(.bss*)
    *(COMMON)
  } :data
>>>>>>> Stashed changes
}