ENTRY(_start)
PHDRS{ text PT_LOAD FLAGS(5); data PT_LOAD FLAGS(6); note PT_NOTE FLAGS(4); }
SECTIONS{
. = 1M;
.multiboot : { *(.multiboot) } :text
.note.Xen : { *(.note.Xen) } :note
.text : { *(.text*) } :text
.rodata : { *(.rodata*) } :text
.data : { *(.data*) } :data
.bss : { *(.bss*) *(COMMON) } :data
 _kernel_end =.;
 /DISCARD/ : { *(.note.GNU-stack) *(.eh_frame*) *(.comment) }
}