## Pizza OS 🍕
Bare metal pizza kernel @ 1MB — QEMU + Node dashboard

Impartial OS v0.2 — No telemetry. No account. No feed. Just pizza.

`pizza-os.bin` 12K C++ kernel. Boots via Multiboot2 @ 0x100000. VGA 0xB8000.

### What you get on boot
```

 hacking-pizza.ca // impartial OS // v0.2 // pizza-os.bin @ 1MB


[OK] C++ kernel loaded
[OK] VGA 0xB8000 initialized (80x25)
[OK] No telemetry. No account. No feed.

[INFO] boot: multiboot2 (GRUB) // or PVH / direct (fallback)
[OK] Real memory map found
 base=0x0000000000000000
 base=0x0000000000100000[usable]

[TEST] Bump allocator test:
 alloc1 = 0x0000000000100000
 alloc2 = 0x0000000000101000

 ORDERS IN RAM:

 Pepperoni 14" -> $18.99
 Meat Lovers -> $21.99
...
 REVENUE: $77.96 COST: $25.00 PROFIT: $52.96 (67% margin)

pizza-os> _
```

### dev on local / WSL:
```bash
npm install
npm run dev
```

### Full build commands:

```bash
# 1. Install tools (WSL / Ubuntu)
sudo apt install nasm g++-multilib grub-pc-bin xorriso mtools qemu-system-x86

# 2. Install node deps
npm install

# 3. Build 12K kernel
npm run k:build
# -rwxr-xr-x 12K pizza-os.bin

# 4. Verify multiboot2 header
npm run k:check
# ✅ VALID multiboot2

# 5. Fast dev boot - direct kernel (PVH fallback allocator 16MB-64MB)
npm run k:qemu:bin

# 6. Build bootable GRUB ISO (real e820 memory map)
npm run k:iso
# pizza-os.iso

# 7. Real boot via GRUB
npm run k:qemu:iso

# 8. Start dashboard
npm run dev
# http://localhost:3000
```

### scripts

| command | what it does |
| :--- | :--- |
| `k:build` | `nasm + g++ -m32 + ld -T linker.ld` -> `pizza-os.bin` + `public/pizza-os.bin` |
| `k:check` | `grub-file --is-x86-multiboot2` |
| `k:qemu:bin` | `qemu -kernel pizza-os.bin` (fast, fallback) |
| `k:iso` | copy bin to `iso/boot/` + `grub-mkrescue` |
| `k:qemu:iso` | `qemu -cdrom pizza-os.iso` (real map) |
| `k:clean` | `rm *.o *.bin *.iso iso/` |
| `dev` | `nodemon server.js` |
| `prod` | `k:build + pm2 start` on hacking-pizza.ca |

### structure
```
boot.asm multiboot2 magic 0xe85250d6
kernel.cpp VGA, serial, orders demo
mm.h / mm.cpp e820 parser + bump allocator
ai_model.cpp stub
linker.ld kernel @ 1MB
server.js express + QEMU runner
public/ deployed bin for web
```

### deploy
```bash
npm run prod
pm2 status
pm2 logs pizza-os
```

hacking-pizza.ca — Vanna Bee, Troy James — ISC