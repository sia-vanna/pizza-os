const logEl = document.getElementById('log');
const statusEl = document.getElementById('status');
const statsEl = document.getElementById('stats');

function clearLog(){ logEl.textContent=''; }
function dl(f){ window.open('/'+f); }

async function go(cmd){
  statusEl.textContent = cmd+'...';
  logEl.textContent = `> ${cmd} running @ ${new Date().toLocaleTimeString()}\n\n`;
  try{
    const r = await fetch('/api/'+cmd,{method:'POST'});
    const t = await r.text();
    logEl.textContent = t;
    if(statsEl){
      const isReal = t.includes('Real memory map');
      const alloc = t.match(/alloc1 = (0x[0-9a-fx]+)/i);
      statsEl.innerHTML = `boot: ${isReal ? 'GRUB multiboot2 ✅' : t.includes('fallback') ? 'PVH fallback' : cmd}<br>map: ${isReal ? 'real e820' : 'fallback 16-64M'}<br>alloc: ${alloc ? alloc[1] : '-'}`;
    }
    statusEl.textContent = 'done ✅';
  }catch(e){
    logEl.textContent = `❌ ${e.message}`;
    statusEl.textContent = 'error';
  }
  logEl.scrollTop = logEl.scrollHeight;
}

// HOT RELOAD - waits for server, no "page not working"
if (location.hostname === 'localhost') {
  const es = new EventSource('/events');
  let reloading = false;
  es.onmessage = async () => {
    if (reloading) return;
    reloading = true;
    statusEl.textContent = 'reloading...';
    localStorage.setItem('log', logEl.textContent);
    for (let i = 0; i < 30; i++) {
      try { await fetch('/', { cache: 'no-store' }); break; } catch {}
      await new Promise(r => setTimeout(r, 200));
    }
    location.reload();
  };
  es.onerror = () => console.log('server restarting...');
  const saved = localStorage.getItem('log');
  if (saved) { logEl.textContent = saved; localStorage.removeItem('log'); }
}
