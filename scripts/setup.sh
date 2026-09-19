#!/bin/bash
set -e
sudo apt update
sudo apt install -y nasm g++-multilib binutils qemu-system-x86 grub-common xorriso mtools
npm install
cat > nodemon.json <<'JSON'
{"watch":["server.js"],"ignore":["*.o","*.bin","*.iso","iso","public"],"ext":"js,json"}
JSON
npm pkg set scripts.dev="nodemon server.js" >/dev/null
echo "✅ fixed - run npm run dev"
