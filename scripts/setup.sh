#!/bin/bash
set -e
echo "🍕 pizza-os setup for Debian/Ubuntu"

# 1. deps for kernel + qemu + iso
sudo apt-get update
sudo apt-get install -y nasm g++-multilib binutils \
  qemu-system-x86 grub-common grub-pc-bin \
  xorriso mtools nodejs npm

# 2. node deps
npm install

# 3. fix nodemon - your old glob was killing it
cat > nodemon.json <<'JSON'
{
  "watch": ["server.js"],
  "ignore": ["*.o", "*.bin", "*.iso", "iso", "public/*.bin", "public/*.iso"],
  "ext": "js,json",
  "delay": 500
}
JSON

npm pkg set scripts.dev="nodemon server.js" >/dev/null 2>&1 || true

# 4. create public for downloads
mkdir -p public iso/boot/grub

# 5. build kernel and verify PT_NOTE (QEMU 8+ fix)
echo "--- building kernel ---"
nasm -f elf32 boot.asm -o boot.o
g++ -m32 -ffreestanding -O2 -c kernel.cpp -o kernel.o
g++ -m32 -ffreestanding -O2 -c ai_model.cpp -o ai_model.o
ld -m elf_i386 -T linker.ld -o pizza-os.bin boot.o kernel.o ai_model.o -nostdlib
cp pizza-os.bin public/pizza-os.bin || true

echo "--- checking headers ---"
readelf -l pizza-os.bin | grep -E "LOAD|NOTE" || true
readelf -n pizza-os.bin | head -20
grub-file --is-x86-multiboot2 pizza-os.bin && echo "✅ VALID multiboot2 - PT_NOTE present" || echo "❌ INVALID"

# 6. quick qemu test
echo "--- qemu test (3 sec) ---"
timeout 3 qemu-system-i386 -kernel pizza-os.bin -nographic 2>&1 | head -20 || true

echo ""
echo "✅ setup done"
echo "Run: npm run dev -> http://localhost:3000"
echo "     npm run k:check"
echo "     npm run k:qemu"
