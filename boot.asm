section .multiboot2
align 8
mb2_start:
  dd 0xe85250d6
  dd 0
  dd mb2_end - mb2_start
  dd -(0xe85250d6 + 0 + (mb2_end - mb2_start))
  dw 0,0
  dd 8
mb2_end:

section .note
align 4
  dd 4
  dd 4
  dd 18
  db "Xen",0
  align 4
  dd _start
  align 4

section .text
global _start
extern kernel_main
_start:
  cli
  mov esp, stack_top
  push ebx        ; multiboot info pointer (GRUB puts this in ebx)
  push eax        ; multiboot2 magic (GRUB puts this in eax)
  call kernel_main
.hang: hlt
  jmp .hang
section .bss
align 16
stack_bottom:
  resb 16384
stack_top: