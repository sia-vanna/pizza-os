section .multiboot
align 8
dd 0xE85250D6
dd 0
dd 24
dd -(0xE85250D6 + 0 + 24)
dw 0
dw 0
dd 8

section .note.Xen
align 4
dd 4
dd 4
dd 18
db "Xen",0
dd _start

section .text
global _start
extern kernel_main
_start:
 cli
 mov dx, 0x3F8+1
 xor al, al
 out dx, al
 mov dx, 0x3F8+3
 mov al, 0x80
 out dx, al
 mov dx, 0x3F8+0
 mov al, 0x03
 out dx, al
 mov dx, 0x3F8+1
 xor al, al
 out dx, al
 mov dx, 0x3F8+3
 mov al, 0x03
 out dx, al
 mov dx, 0x3F8+2
 mov al, 0xC7
 out dx, al
 mov dx, 0x3F8+4
 mov al, 0x0B
 out dx, al
 mov edi, 0xB8000
 mov ecx, 80*25
 mov ax, 0x0F20
 rep stosw
 mov esp, stack_top
 push ebx
 push eax
 call kernel_main
.hang: hlt
 jmp .hang

section .bss
align 16
stack_bottom:
 resb 16384
stack_top:

section .note.GNU-stack noalloc noexec nowrite progbits