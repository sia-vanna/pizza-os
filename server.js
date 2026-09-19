const express = require('express');
const path = require('path');
const { exec } = require('child_process');
const fs = require('fs');
const app = express();
app.use(express.static(path.join(__dirname, 'public')));
const BIN = path.join(__dirname, 'pizza-os.bin');
app.get('/pizza-os.bin', (req,res)=>{ if(!fs.existsSync(BIN)) return res.status(404).send('build first'); res.sendFile(BIN); });
function run(cmd,res){ console.log('[API] '+cmd.slice(0,100)); exec(cmd,{cwd:__dirname,timeout:30000},(e,o,er)=>{ res.type('text/plain').send((o||'')+(er||'')+(e?`\n[exit ${e.code}]`:'')); }); }
app.post('/api/build',(req,res)=>{ run(`nasm -f elf32 boot.asm -o boot.o && g++ -m32 -ffreestanding -O2 -c kernel.cpp -o kernel.o && g++ -m32 -ffreestanding -O2 -c ai_model.cpp -o ai_model.o && ld -m elf_i386 -T linker.ld -o ${BIN} boot.o kernel.o ai_model.o -nostdlib && cp ${BIN} public/pizza-os.bin 2>/dev/null; ls -lh ${BIN} && grub-file --is-x86-multiboot2 ${BIN} && echo "✅ VALID"`,res); });
app.post('/api/check',(req,res)=>{ run(`grub-file --is-x86-multiboot2 ${BIN} && echo "✅ VALID" || echo "❌ INVALID"; readelf -n ${BIN} | head -20`,res); });
app.post('/api/qemu',(req,res)=>{
  // try direct kernel first with isapc (disables PVH loader), fallback to iso
  const cmd = `if qemu-system-i386 -kernel ${BIN} -machine isapc -nographic -serial mon:stdio 2>&1 | head -100; then echo "direct boot done"; else mkdir -p iso/boot/grub && cp ${BIN} iso/boot/ && printf 'set timeout=0\nset default=0\nmenuentry "pizza-os" { multiboot2 /boot/pizza-os.bin\\n boot }\n' > iso/boot/grub/grub.cfg && grub-mkrescue -o pizza-os.iso iso && timeout 10 qemu-system-i386 -cdrom pizza-os.iso -boot d -machine pc -nographic 2>&1; fi; echo "\n--- qemu exit ---"`;
  run(`timeout 12 bash -c '${cmd.replace(/'/g,"'\\''")}'`,res);
});
app.post('/api/iso',(req,res)=>{ run(`mkdir -p iso/boot/grub && cp ${BIN} iso/boot/ && printf 'set timeout=0\nmenuentry "pizza-os" { multiboot2 /boot/pizza-os.bin\\n boot }\n' > iso/boot/grub/grub.cfg && grub-mkrescue -o pizza-os.iso iso 2>&1 && ls -lh pizza-os.iso`,res); });
app.listen(3000,()=>console.log(`🍕 http://localhost:3000`));
