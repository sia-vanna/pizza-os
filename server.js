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

<<<<<<< Updated upstream
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
=======
app.post('/api/build',(req,res)=>{
  run(`nasm -f elf32 boot.asm -o boot.o && g++ -m32 -ffreestanding -O2 -c kernel.cpp -o kernel.o && g++ -m32 -ffreestanding -O2 -c mm.cpp -o mm.o && g++ -m32 -ffreestanding -O2 -c ai_model.cpp -o ai_model.o && ld -m elf_i386 -T linker.ld -o ${BIN} boot.o kernel.o mm.o ai_model.o -nostdlib && cp ${BIN} public/pizza-os.bin 2>/dev/null || true && ls -lh ${BIN} && grub-file --is-x86-multiboot2 ${BIN} && echo "✅ VALID"`,res);
});
>>>>>>> Stashed changes

app.post('/api/check', (req, res) => run(
  `ls -lh ${BIN} && ` +
  `grub-file --is-x86-multiboot2 ${BIN} && echo "✅ VALID multiboot2" || echo "❌ INVALID"; ` +
  `echo ""; readelf -l ${BIN} | grep -A1 NOTE`,
  res
));

<<<<<<< Updated upstream
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

app.listen(3000, () => console.log(`🍕 http://localhost:3000 BIN=${BIN}`));// troy was here
=======
app.post('/api/qemu',(req,res)=>{
  run(`timeout 6 qemu-system-i386 -kernel ${BIN} -nographic 2>&1; echo "\n--- qemu exit (PVH fast path) ---"`,res);
});

app.post('/api/qemu-iso',(req,res)=>{
  run(`mkdir -p iso/boot/grub && cp ${BIN} iso/boot/ && printf 'set timeout=0\\nserial --unit=0 --speed=9600\\nterminal_input serial\\nterminal_output serial\\nmenuentry "pizza-os" { multiboot2 /boot/pizza-os.bin\\n boot }\\n' > iso/boot/grub/grub.cfg && grub-mkrescue -o pizza-os.iso iso 2>&1 | tail -3 && timeout 8 qemu-system-i386 -cdrom pizza-os.iso -nographic 2>&1; echo "\n--- qemu exit (real multiboot2 path) ---"`,res);
});

app.post('/api/iso',(req,res)=>{
  run(`mkdir -p iso/boot/grub && cp ${BIN} iso/boot/ && printf 'set timeout=0\\nserial --unit=0 --speed=9600\\nterminal_input serial\\nterminal_output serial\\nmenuentry "pizza-os" { multiboot2 /boot/pizza-os.bin\\n boot }\\n' > iso/boot/grub/grub.cfg && grub-mkrescue -o pizza-os.iso iso 2>&1 | tail -5 && ls -lh pizza-os.iso`,res);
});

app.listen(3000,()=>console.log(`🍕 http://localhost:3000 BIN=${BIN}`));
>>>>>>> Stashed changes
