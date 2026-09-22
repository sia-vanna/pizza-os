const express = require('express');
const path = require('path');
const { exec } = require('child_process');
const fs = require('fs');
const app = express();

app.use(express.static(path.join(__dirname, 'public')));

const BIN = path.join(__dirname, 'pizza-os.bin');
const PUB_BIN = path.join(__dirname, 'public', 'pizza-os.bin');

function ensureBin(res) {
  if (!fs.existsSync(BIN) && fs.existsSync(PUB_BIN)) {
    fs.copyFileSync(PUB_BIN, BIN);
  }
  if (!fs.existsSync(BIN)) {
    res.status(404).type('text/plain').send('BIN not found — POST /api/build first');
    return false;
  }
  return true;
}

function run(cmd, res) {
  console.log('[API] ' + cmd.slice(0, 180));
  exec(cmd, { cwd: __dirname, timeout: 20000 }, (err, stdout, stderr) => {
    res.type('text/plain').send((stdout || '') + (stderr || '') + (err ? '\nERR:' + err.message : ''));
  });
}

// serve bin
app.get('/pizza-os.bin', (req, res) => {
  if (!fs.existsSync(BIN)) return res.status(404).send('run build first');
  res.sendFile(BIN);
});

app.post('/api/build', (req, res) => {
  run(`nasm -f elf32 boot.asm -o boot.o && g++ -m32 -ffreestanding -O2 -c kernel.cpp -o kernel.o && g++ -m32 -ffreestanding -O2 -c mm.cpp -o mm.o && g++ -m32 -ffreestanding -O2 -c ai_model.cpp -o ai_model.o && ld -m elf_i386 -T linker.ld -o ${BIN} boot.o kernel.o mm.o ai_model.o -nostdlib && mkdir -p public && cp ${BIN} ${PUB_BIN} && ls -lh ${BIN} && grub-file --is-x86-multiboot2 ${BIN} && echo "✅ VALID multiboot2"`, res);
});

app.post('/api/check', (req, res) => {
  if (!ensureBin(res)) return;
  run(`ls -lh ${BIN}; echo "---"; grub-file --is-x86-multiboot2 ${BIN} && echo "✅ VALID multiboot2" || echo "❌ INVALID"; echo "---"; readelf -l ${BIN} | grep -A2 NOTE || true`, res);
});

app.post('/api/qemu', (req, res) => {
  if (!ensureBin(res)) return;
  // PVH fast path — shows fallback allocator
  run(`timeout 6 qemu-system-i386 -kernel ${BIN} -nographic -serial mon:stdio 2>&1; echo "\\n--- qemu exit (PVH fast path) ---"`, res);
});

// FIXED: no serial terminal override so VGA works
app.post('/api/qemu-iso', (req, res) => {
  if (!ensureBin(res)) return;
  run(`mkdir -p iso/boot/grub && cp ${BIN} iso/boot/ && echo 'set timeout=0\nset default=0\nmenuentry "pizza-os" {\n  multiboot2 /boot/pizza-os.bin\n  boot\n}' > iso/boot/grub/grub.cfg && grub-mkrescue -o pizza-os.iso iso 2>&1 | tail -3 && timeout 8 qemu-system-i386 -cdrom pizza-os.iso -boot d -nographic 2>&1; echo "\\n--- qemu exit (real multiboot2 path) ---"`, res);
});

app.post('/api/iso', (req, res) => {
  if (!ensureBin(res)) return;
  run(`mkdir -p iso/boot/grub && cp ${BIN} iso/boot/ && echo 'set timeout=0\nset default=0\nmenuentry "pizza-os" {\n  multiboot2 /boot/pizza-os.bin\n  boot\n}' > iso/boot/grub/grub.cfg && grub-mkrescue -o pizza-os.iso iso 2>&1 | tail -5 && ls -lh pizza-os.iso && cp pizza-os.iso public/ 2>/dev/null || true`, res);
});

app.listen(3000, () => console.log(`🍕 http://localhost:3000 BIN=${BIN}`));