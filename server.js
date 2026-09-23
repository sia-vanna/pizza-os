const express=require('express'),path=require('path'),{exec,spawn}=require('child_process'),fs=require('fs'),WebSocket=require('ws');
const app=express(); const server=require('http').createServer(app);
const wss=new WebSocket.Server({server,path:'/ws'});
app.use(express.static(path.join(__dirname,'public')));
const BIN=path.join(__dirname,'pizza-os.bin');
function run(c,r){ exec(c,{cwd:__dirname,timeout:20000},(e,o,er)=>r.type('text/plain').send((o||'')+(er||''))); }
app.post('/api/build',(req,res)=>run(`nasm -f elf32 boot.asm -o boot.o && g++ -m32 -ffreestanding -O2 -c kernel.cpp -o kernel.o && g++ -m32 -ffreestanding -O2 -c mm.cpp -o mm.o && g++ -m32 -ffreestanding -O2 -c fs.cpp -o fs.o && g++ -m32 -ffreestanding -O2 -c user.cpp -o user.o && g++ -m32 -ffreestanding -O2 -c ai_model.cpp -o ai_model.o && ld -m elf_i386 -T linker.ld -o ${BIN} boot.o kernel.o mm.o fs.o user.o ai_model.o -nostdlib && mkdir -p public && cp ${BIN} public/pizza-os.bin && ls -lh ${BIN} && grub-file --is-x86-multiboot2 ${BIN} && echo "✅ VALID"`,res));
let q=null;
wss.on('connection',ws=>{
 if(!fs.existsSync(BIN)){ ws.send('run build first\n'); return ws.close(); }
 if(q) try{q.kill('SIGKILL')}catch{}
 q=spawn('qemu-system-i386',['-kernel',BIN,'-nographic','-serial','stdio','-monitor','none'],{cwd:__dirname});
 q.stdout.on('data',d=>{ if(ws.readyState===1) ws.send(d.toString()); });
 q.stderr.on('data',d=>{ if(ws.readyState===1) ws.send(d.toString()); });
 q.on('close',c=>{ try{ws.send('\n--- qemu exit '+c+' ---\n')}catch{} ws.close(); q=null; });
 ws.on('message',m=>{
   if(!q||!q.stdin.writable) return;
   let s=m.toString();
   // browser -> qemu : Enter is \n, Backspace is \x7f
   q.stdin.write(s);
 });
 ws.on('close',()=>{ if(q){ q.kill('SIGKILL'); q=null; } });
});
server.listen(3000,()=>console.log('🍕 http://localhost:3000'));