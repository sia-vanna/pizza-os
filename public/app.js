const term=document.getElementById('term');
let ws=null;
function print(s){term.textContent+=s;term.scrollTop=term.scrollHeight;}
function connect(){
  if(ws) ws.close(); term.textContent='';
  ws=new WebSocket((location.protocol==='https:'?'wss:':'ws:')+'//'+location.host+'/ws');
  ws.binaryType='arraybuffer';
  ws.onopen=()=>term.focus();
  ws.onmessage=e=>{
    let d=typeof e.data==='string'?e.data:new TextDecoder().decode(e.data);
    d=d.replace(/\x1Bc/g,'').replace(/\x1B\[[0-9;?]*[A-Za-z]/g,'');
    // drop BIOS lines that still slip through
    if(d.includes('SeaBIOS')||d.includes('iPXE')||d.includes('Booting from ROM')) return;
    print(d);
  };
  ws.onclose=()=>print('\n[disconnected]\n');
}
document.getElementById('c').onclick=connect;
document.getElementById('b').onclick=async()=>{
  print('\n[building...]\n');
  const r=await fetch('/api/build',{method:'POST'});
  print(await r.text()+'\n');
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