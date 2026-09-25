#!/bin/bash
set -e
set -x
nasm -f elf32 boot.asm -o boot.o
g++ -m32 -ffreestanding -O2 -c kernel.cpp -o kernel.o
g++ -m32 -ffreestanding -O2 -c mm.cpp -o mm.o
g++ -m32 -ffreestanding -O2 -c paging.cpp -o paging.o
g++ -m32 -ffreestanding -O2 -c fs.cpp -o fs.o
g++ -m32 -ffreestanding -O2 -c user.cpp -o user.o
g++ -m32 -ffreestanding -O2 -c ai_model.cpp -o ai_model.o
g++ -m32 -ffreestanding -O2 -c scheduler.cpp -o scheduler.o
ld -m elf_i386 -T linker.ld -o pizza-os.bin boot.o kernel.o mm.o paging.o fs.o user.o ai_model.o scheduler.o -nostdlib
mkdir -p public
cp pizza-os.bin public/pizza-os.bin
ls -lh pizza-os.bin
grub-file --is-x86-multiboot2 pizza-os.bin && echo "✅ VALID"
