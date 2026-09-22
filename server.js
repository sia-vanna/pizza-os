const express = require('express');
const path = require('path');
const { exec } = require('child_process');
const fs = require('fs');
const app = express();
app.use(express.static(path.join(__dirname, 'public')));

const BIN = path.join(__dirname, 'pizza-os.bin');
const PUB_BIN = path.join(__dirname, 'public', 'pizza-os.bin');

function ensureBin(res) {
  if (!fs.existsSync(BIN) && fs.existsSync(PUB_BIN)) fs.copyFileSync(PUB_BIN, BIN);
  if (!fs.existsSync(BIN)) {
    res.status(404).type('text/plain').send('BIN not found — POST /api/build first');
    return false;
  }
  return true;
}
function run(cmd, res) {
  exec(cmd, { cwd: __dirname, timeout: 20000 }, (err, stdout, stderr) => {
    // timeout 8 qemu = exit 124 is expected, not an error
    const out = (stdout||'') + (stderr||'');
    if (err && err.code !== 124 && !out.includes('qemu')) {
      res.type('text/plain').send(out + '\nERR:'+err.message);
    } else {
      res.type('text/plain').send(out + '\n--- qemu exit (ok, timeout after 8s) ---');
    }
  });
}

app.get('/pizza-os.bin', (req,res)=>{ if(!fs.existsSync(BIN)) return res.status(404).send('run build first'); res.sendFile(BIN); });
app.post('/api/build', (req,res)=>{ run(`nasm -f elf32 boot.asm -o boot.o && g++ -m32 -ffreestanding -O2 -c kernel.cpp -o kernel.o && g++ -m32 -ffreestanding -O2 -c mm.cpp -o mm.o && g++ -m32 -ffreestanding -O2 -c ai_model.cpp -o ai_model.o && ld -m elf_i386 -T linker.ld -o ${BIN} boot.o kernel.o mm.o ai_model.o -nostdlib && mkdir -p public && cp ${BIN} ${PUB_BIN} && ls -lh ${BIN} && grub-file --is-x86-multiboot2 ${BIN} && echo "✅ VALID"`, res); });
app.post('/api/check', (req,res)=>{ if(!ensureBin(res)) return; run(`grub-file --is-x86-multiboot2 ${BIN} && echo "✅ VALID" || echo "❌ INVALID"`, res); });
app.post('/api/qemu', (req,res)=>{ if(!ensureBin(res)) return; run(`timeout 6 qemu-system-i386 -kernel ${BIN} -nographic -serial mon:stdio 2>&1`, res); });
app.post('/api/qemu-iso', (req,res)=>{ if(!ensureBin(res)) return; run(`mkdir -p iso/boot/grub && cp ${BIN} iso/boot/ && printf 'set timeout=0\\nmenuentry "pizza-os" { multiboot2 /boot/pizza-os.bin\\n boot\\n}\\n' > iso/boot/grub/grub.cfg && grub-mkrescue -o pizza-os.iso iso 2>&1 | tail -2 && timeout 8 qemu-system-i386 -cdrom pizza-os.iso -boot d -nographic 2>&1`, res); });
app.post('/api/iso', (req,res)=>{ if(!ensureBin(res)) return; run(`mkdir -p iso/boot/grub && cp ${BIN} iso/boot/ && printf 'set timeout=0\\nmenuentry "pizza-os" { multiboot2 /boot/pizza-os.bin\\n boot\\n}\\n' > iso/boot/grub/grub.cfg && grub-mkrescue -o pizza-os.iso iso 2>&1 | tail -2 && cp pizza-os.iso public/ && ls -lh pizza-os.iso`, res); });

// --- HOT RELOAD SSE ---
let clients = [];
app.get('/events', (req,res)=>{
  res.writeHead(200, {'Content-Type':'text/event-stream','Cache-Control':'no-cache','Connection':'keep-alive'});
  clients.push(res);
  req.on('close',()=>{ clients=clients.filter(c=>c!==res); });
});
fs.watch(path.join(__dirname,'public'), {recursive:true}, ()=>{
  clients.forEach(c=>c.write('data: reload\n\n'));
});

app.listen(3000, ()=>console.log('🍕 http://localhost:3000'));