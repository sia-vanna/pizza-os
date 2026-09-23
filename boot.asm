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
 ; --- SERIAL 0x3F8 EARLY INIT ---
 mov dx, 0x3F8+1
 mov al, 0x00
 out dx, al        ; disable interrupts
 mov dx, 0x3F8+3
 mov al, 0x80
 out dx, al        ; enable DLAB
 mov dx, 0x3F8+0
 mov al, 0x03
 out dx, al        ; divisor low 3 = 38400 baud
 mov dx, 0x3F8+1
 mov al, 0x00
 out dx, al        ; divisor high
 mov dx, 0x3F8+3
 mov al, 0x03
 out dx, al        ; 8n1
 mov dx, 0x3F8+2
 mov al, 0xC7
 out dx, al        ; enable FIFO
 mov dx, 0x3F8+4
 mov al, 0x0B
 out dx, al        ; IRQs, RTS/DSR set
 ; -------------------------------

 mov esp, stack_top
 push ebx          ; multiboot info
 push eax          ; magic
 call kernel_main

.hang: hlt
 jmp .hang

section .bss
align 16
stack_bottom:
 resb 16384
stack_top:

section .note.GNU-stack noalloc noexec nowrite progbits