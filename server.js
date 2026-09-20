const express = require('express');
const path = require('path');
const { exec } = require('child_process');
const fs = require('fs');
const app = express();
app.use(express.static(path.join(__dirname, 'public')));
const BIN = path.join(__dirname, 'pizza-os.bin');

app.get('/pizza-os.bin',(req,res)=>{
  if(!fs.existsSync(BIN)) return res.status(404).send('run build first');
  res.sendFile(BIN);
});

function run(cmd,res){
  console.log('[API] '+cmd.slice(0,120));
  exec(cmd,{cwd:__dirname,timeout:15000},(err,stdout,stderr)=>{
    res.type('text/plain').send((stdout||'')+(stderr||''));
  });
}

app.post('/api/build',(req,res)=>{
  run(`nasm -f elf32 boot.asm -o boot.o && g++ -m32 -ffreestanding -O2 -c kernel.cpp -o kernel.o && g++ -m32 -ffreestanding -O2 -c mm.cpp -o mm.o && g++ -m32 -ffreestanding -O2 -c ai_model.cpp -o ai_model.o && ld -m elf_i386 -T linker.ld -o ${BIN} boot.o kernel.o mm.o ai_model.o -nostdlib && cp ${BIN} public/pizza-os.bin 2>/dev/null || true && ls -lh ${BIN} && grub-file --is-x86-multiboot2 ${BIN} && echo "✅ VALID"`,res);
});

app.post('/api/check',(req,res)=>{
  run(`readelf -l ${BIN} | grep -A1 NOTE; echo "---"; readelf -n ${BIN} | head -20; echo "---"; grub-file --is-x86-multiboot2 ${BIN} && echo "✅ VALID multiboot2" || echo "❌ INVALID"`,res);
});

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