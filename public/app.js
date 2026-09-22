const logEl = document.getElementById('log');
const statusEl = document.getElementById('status');
function clearLog(){ logEl.textContent=''; }
function dl(f){ window.open('/'+f); }

// HOT RELOAD - waits for server, no "page not working"
if (location.hostname === 'localhost') {
  const es = new EventSource('/events');
  let reloading = false;

  es.onmessage = async () => {
    if (reloading) return;
    reloading = true;
    statusEl.textContent = 'reloading...';
    logEl.textContent += '\n> [hot reload] waiting for server...\n';

    // save log
    const saved = logEl.textContent;
    localStorage.setItem('log', saved);

    // wait for server to come back up (poll)
    for (let i = 0; i < 30; i++) {
      try {
        await fetch('/', { cache: 'no-store' });
        break; // server back!
      } catch {}
      await new Promise(r => setTimeout(r, 200));
    }
    location.reload();
  };

  es.onerror = () => {
    // server down during nodemon restart, don't show error
    console.log('server restarting...');
  };

  const saved = localStorage.getItem('log');
  if (saved) {
    logEl.textContent = saved;
    localStorage.removeItem('log');
  }
}
