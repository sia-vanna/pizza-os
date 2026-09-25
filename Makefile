all:
	rm -f *.o *.bin
	nasm -f elf32 boot.asm -o boot.o
	g++ -m32 -ffreestanding -O2 -c mm.cpp -o mm.o
	g++ -m32 -ffreestanding -O2 -c paging.cpp -o paging.o
	g++ -m32 -ffreestanding -O2 -c fs.cpp -o fs.o
	g++ -m32 -ffreestanding -O2 -c user.cpp -o user.o
	g++ -m32 -ffreestanding -O2 -c ai_model.cpp -o ai_model.o
	g++ -m32 -ffreestanding -O2 -c scheduler.cpp -o scheduler.o
	g++ -m32 -ffreestanding -O2 -c kernel.cpp -o kernel.o
	ld -m elf_i386 -T linker.ld -o pizza-os.bin boot.o kernel.o scheduler.o paging.o mm.o fs.o user.o ai_model.o -nostdlib --no-warn-rwx-segment

run: all
	qemu-system-i386 -kernel pizza-os.bin -nographic

clean:
	rm -f *.o *.bin
