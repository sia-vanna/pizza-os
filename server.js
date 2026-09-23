const express = require('express');
const path = require('path');
const { exec, spawn } = require('child_process');
const fs = require('fs');
const WebSocket = require('ws');

const app = express();
const server = require('http').createServer(app);
const wss = new WebSocket.Server({ server, path: '/ws' });

app.use(express.static(path.join(__dirname, 'public')));
app.use(express.text({ type: '*/*' }));
app.use(express.json());

const BIN = path.join(__dirname, 'pizza-os.bin');

function run(c, r) {
  exec(c, { cwd: __dirname, timeout: 20000 }, (e, o, er) => r.type('text/plain').send((o || '') + (er || '')));
}

app.post('/api/build', (req, res) => run(
  `nasm -f elf32 boot.asm -o boot.o && g++ -m32 -ffreestanding -O2 -c kernel.cpp -o kernel.o && g++ -m32 -ffreestanding -O2 -c mm.cpp -o mm.o && g++ -m32 -ffreestanding -O2 -c fs.cpp -o fs.o && g++ -m32 -ffreestanding -O2 -c user.cpp -o user.o && g++ -m32 -ffreestanding -O2 -c ai_model.cpp -o ai_model.o && ld -m elf_i386 -T linker.ld -o ${BIN} boot.o kernel.o mm.o fs.o user.o ai_model.o -nostdlib && mkdir -p public && cp ${BIN} public/pizza-os.bin && ls -lh ${BIN} && grub-file --is-x86-multiboot2 ${BIN} && echo "✅ VALID"`, res));

wss.on('connection', ws => {
  if (!fs.existsSync(BIN)) {
    ws.send('BIN missing, POST /api/build first\n');
    return ws.close();
  }
  const q = spawn('qemu-system-i386', ['-accel', 'tcg', '-machine', 'pc', '-kernel', BIN, '-nographic', '-serial', 'stdio', '-monitor', 'none'], { cwd: __dirname, stdio: ['pipe', 'pipe', 'pipe'] });
  console.log('[WS] new client, qemu pid', q.pid);
  q.stdout.on('data', d => { if (ws.readyState === 1) ws.send(d.toString()); });
  q.stderr.on('data', d => { if (ws.readyState === 1) ws.send(d.toString()); });
  q.on('close', c => { console.log('[QEMU] exit', c); try { ws.send('\n--- qemu exit ' + c + ' ---\n') } catch {} ws.close(); });
  ws.on('message', m => {
    if (!q.stdin.writable) return;
    let s = m.toString().replace(/\x7f/g, '\x08');
    q.stdin.write(s);
  });
  ws.on('close', () => { q.kill('SIGKILL'); console.log('[WS] closed'); });
});

server.listen(3000, '0.0.0.0', () => console.log('🍕 http://localhost:3000 BIN=' + BIN));