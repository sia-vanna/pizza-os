const term=document.getElementById('term');
let ws=null;
let buf='';

function print(s){term.textContent+=s;term.scrollTop=term.scrollHeight;}

function clean(s){
  return s.replace(/\x1B\[[0-9;?]*[a-zA-Z]/g,'')
         .replace(/\x1B\].*?\x07/g,'')
         .replace(/\x1B[=>]/g,'')
         .replace(/\[7m/g,'');
}

function connect(){
 if(ws) ws.close();
 term.textContent='';
 buf='';
 ws=new WebSocket((location.protocol==='https:'?'wss:':'ws:')+'//'+location.host+'/ws');
 ws.binaryType='arraybuffer';
 ws.onopen=()=>term.focus();
 ws.onmessage=e=>{
   let d=typeof e.data==='string'?e.data:new TextDecoder().decode(e.data);
   d=clean(d);
   // drop any BIOS chunk completely
   if(d.includes('SeaBIOS') || d.includes('Booting from ROM') || d.includes('iPXE')) return;
   // also drop the weird [disconnected] + BIOS combo
   if(d.includes('disconnected')){ term.textContent=''; return; }
   print(d);
 };
 ws.onclose=()=>{};
}
document.getElementById('c').onclick=connect;
document.getElementById('b').onclick=async()=>{
 print('\n[building...]\n');
 const r=await fetch('/api/build',{method:'POST'});
 print(await r.text()+'\n');
 setTimeout(connect,500);
};
document.addEventListener('keydown',e=>{
 if(!ws||ws.readyState!==1) return;
 if(e.ctrlKey||e.metaKey||e.altKey) return;
 if(e.key.length===1){ws.send(e.key);e.preventDefault();}
 else if(e.key==='Enter'){ws.send('\r');e.preventDefault();}
 else if(e.key==='Backspace'){ws.send('\x7f');e.preventDefault();}
});
term.addEventListener('click',()=>term.focus());
connect();