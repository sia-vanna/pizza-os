# Pizza OS
## dev on your local / WSL machine:
```bash
npm run dev
```
### auto-restarts when you edit server.js, runs on http://localhost:3000
## prod on droplet:
```bash
npm run prod
```
### or for CI/CD it does:
```bash
npm run restart
```
## STOP
```bash
npm run stop
```
# test build
```bash
nasm -f elf32 boot.asm -o boot.o && g++ -m32 -ffreestanding -O2 -c kernel.cpp -o kernel.o && g++ -m32 -ffreestanding -O2 -c ai_model.cpp -o ai_model.o && ld -m elf_i386 -T linker.ld -o pizza-os.bin boot.o kernel.o ai_model.o -nostdlib && ls -lh pizza-os.bin && echo "✅ clean build works"
```