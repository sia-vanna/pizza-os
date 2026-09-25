const express = require('express');
const path = require('path');
const { exec, spawn } = require('child_process');
const fs = require('fs');
const WebSocket = require('ws');
const app = express();
const server = require('http').createServer(app);
const wss = new WebSocket.Server({ server, path: '/ws' });

app.use(express.static(path.join(__dirname, 'public')));

const BIN = path.join(__dirname, 'pizza-os.bin');

app.post('/api/build', (req, res) => {
  exec(`./scripts/build.sh`, { cwd: __dirname, timeout: 20000 }, (e,o,er)=> res.type('text/plain').send((o||'')+(er||'')));
});

function isJunk(s){
  return s.includes('SeaBIOS') || s.includes('Booting from ROM') || s.includes('iPXE') || s.includes('Press F12');
}

wss.on('connection', ws => {
  if (!fs.existsSync(BIN)) {
    ws.send('BIN missing, POST /api/build first\n');
    return ws.close();
  }
  // FIX: -nographic instead of -machine isapc -display none
  const q = spawn('qemu-system-i386', [
    '-kernel', BIN,
    '-nographic',
    '-no-reboot'
  ], { stdio: ['pipe','pipe','pipe'] });

  q.stdout.on('data', d => {
    let s = d.toString();
    if(isJunk(s)) return;
    if(ws.readyState===1) ws.send(s);
  });
  q.stderr.on('data', d => {
    let s = d.toString();
    if(isJunk(s)) return;
    console.log('[QEMU]', s.trim());
  });
  ws.on('message', m => {
    // web sends \n, qemu wants \r for nographic serial
    let str = m.toString().replace(/\x7f/g,'\x08').replace(/\n/g,'\r');
    if(q.stdin.writable) q.stdin.write(str);
  });
  ws.on('close', () => q.kill('SIGKILL'));
  q.on('close', c => { try{ ws.close(); }catch{} });
});

server.listen(3000,'0.0.0.0',()=>console.log('🍕 http://localhost:3000'));