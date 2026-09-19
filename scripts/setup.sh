#!/bin/bash
set -e
echo "🍕 pizza-os setup (no sudo loop)"

# cache sudo
sudo -v

sudo apt-get update -qq
sudo apt-get install -y -qq nasm g++-multilib binutils qemu-system-x86 grub-common grub-pc-bin xorriso mtools > /dev/null

npm install --package-lock=false --silent

mkdir -p public

cat > nodemon.json <<'JSON'
{
  "watch": ["server.js"],
  "ignore": ["*.o","*.bin","*.iso","iso","public/*"],
  "ext": "js,json",
  "delay": 1000
}
JSON

echo "building..."
nasm -f elf32 boot.asm -o boot.o
g++ -m32 -ffreestanding -O2 -c kernel.cpp -o kernel.o
g++ -m32 -ffreestanding -O2 -c ai_model.cpp -o ai_model.o
ld -m elf_i386 -T linker.ld -o pizza-os.bin boot.o kernel.o ai_model.o -nostdlib
cp pizza-os.bin public/pizza-os.bin 2>/dev/null || true

readelf -l pizza-os.bin | grep -E "NOTE" && echo "✅ PT_NOTE ok"
grub-file --is-x86-multiboot2 pizza-os.bin && echo "✅ multiboot2 VALID"

echo "✅ setup done - now run npm run dev in a NEW terminal"
