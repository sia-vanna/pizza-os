const express = require('express');
const path = require('path');
const { exec } = require('child_process');
const app = express();

app.use(express.static(path.join(__dirname, 'public')));

const BIN = path.join(__dirname, 'pizza-os.bin');
const WWW_BIN = '/var/www/pizza-os/pizza-os.bin';

function run(cmd, res) {
  exec(cmd, { cwd: __dirname, timeout: 30000 }, (e, o, er) => {
    res.send((o || '') + (er || '') + (e ? `\n[exit: ${e.code}]` : ''));
  });
}

app.post('/api/build', (req, res) => run(
  `nasm -f elf32 boot.asm -o boot.o && ` +
  `g++ -m32 -ffreestanding -O2 -c kernel.cpp -o kernel.o && ` +
  `g++ -m32 -ffreestanding -O2 -c ai_model.cpp -o ai_model.o && ` +
  `ld -m elf_i386 -T linker.ld -o ${BIN} boot.o kernel.o ai_model.o -nostdlib && ` +
  `ls -lh ${BIN} && ` +
  `sudo cp ${BIN} ${WWW_BIN} && sudo chmod 644 ${WWW_BIN} && ` +
  `echo "✅ built + copied to ${WWW_BIN}"`,
  res
));

app.post('/api/check', (req, res) => run(
  `ls -lh ${BIN} && ` +
  `grub-file --is-x86-multiboot2 ${BIN} && echo "✅ VALID multiboot2" || echo "❌ INVALID"; ` +
  `echo ""; readelf -l ${BIN} | grep -A1 NOTE`,
  res
));

app.post('/api/qemu', (req, res) => run(
  `if [ ! -f ${BIN} ]; then echo "⚠️ bin missing, building..."; ` +
  `nasm -f elf32 boot.asm -o boot.o && g++ -m32 -ffreestanding -O2 -c kernel.cpp -o kernel.o && g++ -m32 -ffreestanding -O2 -c ai_model.cpp -o ai_model.o && ld -m elf_i386 -T linker.ld -o ${BIN} boot.o kernel.o ai_model.o -nostdlib; fi && ` +
  `echo "Booting ${BIN}..."; timeout 5 qemu-system-i386 -kernel ${BIN} -nographic 2>&1; ` +
  `echo "\\n---\\nBoot succeeded ✅"`,
  res
));

app.post('/api/iso', (req, res) => run(
  `mkdir -p iso/boot/grub && cp ${BIN} iso/boot/ && ` +
  `echo 'set timeout=0\nmenuentry "pizza-os" { multiboot /boot/pizza-os.bin boot }' > iso/boot/grub/grub.cfg && ` +
  `grub-mkrescue -o /var/www/pizza-os/pizza-os.iso iso 2>&1 && ls -lh /var/www/pizza-os/pizza-os.iso`,
  res
));

app.listen(3000, () => console.log(`🍕 http://localhost:3000 BIN=${BIN}`));