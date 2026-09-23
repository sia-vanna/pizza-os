#include "user.h"
#include "fs.h"
#include <stdint.h>

extern void print(const char* s, unsigned char c=0x0F);
extern void putchar(char c, unsigned char color=0x0F);

static int k_strlen(const char* s){ int i=0; while(s[i]) i++; return i; }

static inline uint8_t inb(uint16_t p){ uint8_t r; asm volatile("inb %1,%0":"=a"(r):"Nd"(p)); return r; }
static inline void outb(uint16_t p, uint8_t v){ asm volatile("outb %0,%1"::"a"(v),"Nd"(p)); }

user current_user = {0, "root", true};

static char scancode_map[128]={
 0,0,'1','2','3','4','5','6','7','8','9','0','-','=',0,0,
 'q','w','e','r','t','y','u','i','o','p','[',']',0,0,
 'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\',
 'z','x','c','v','b','n','m',',','.','/',0,0,0,' ',0
};

static char kbd_getchar(){
 while(1){
  // 1. serial 0x3F8 (websocket)
  if(inb(0x3F8+5) & 1){
   char c = inb(0x3F8);
   if(c=='\r' || c=='\n') return '\n';
   if(c==127 || c==8) return '\b'; // both DEL and BS from browser
   return c;
  }
  // 2. PS/2 keyboard 0x60 (real qemu)
  if(inb(0x64)&1){
   uint8_t sc = inb(0x60);
   if(sc & 0x80) continue;
   if(sc < 128 && scancode_map[sc]) return scancode_map[sc];
   if(sc==0x1C) return '\n';
   if(sc==0x0E) return '\b';
  }
 }
}

static void kbd_getline(char* buf, int max, bool echo){
 int i=0;
 while(i<max-1){
  char c = kbd_getchar();
  if(c=='\n'){ buf[i]=0; putchar('\n'); break; }
  if(c=='\b'){
   if(i>0){
    i--;
    putchar('\b'); putchar(' '); putchar('\b');
   }
   continue;
  }
  buf[i++]=c;
  if(echo) putchar(c); else putchar('*');
 }
 buf[i]=0;
}

static bool check_pass(const char* user, const char* pass){
 auto* f = fs_open("/etc/shadow");
 if(!f) return false;
 char* data = (char*)f->data;
 char line[64]; int li=0;
 for(int i=0;;i++){
  char c = i<(int)f->size? data[i] : '\n';
  if(c=='\n' || c==0){
   line[li]=0;
   char* sep=0; for(int k=0;line[k];k++) if(line[k]==':'){ sep=&line[k]; break; }
   if(sep){
    *sep=0;
    if(k_strlen(user)==k_strlen(line)){
     bool match=true; for(int k=0;line[k];k++) if(line[k]!=user[k]) match=false;
     if(match){
      const char* real=sep+1; bool ok=true; int j=0;
      for(;pass[j]&&real[j];j++) if(pass[j]!=real[j]) ok=false;
      if(pass[j]!=real[j]) ok=false; return ok;
     }
    }
   }
   li=0; if(c==0) break;
  } else if(li<63) line[li++]=c;
  if(data[i]==0) break;
 }
 return false;
}

bool user_init(){ current_user.uid=0; return true; }

void login_loop(){
 char uname[32]; char pass[32];
 while(1){
  print("\npizza-os login: ", 0x0B);
  kbd_getline(uname,32,true);
  if(k_strlen(uname)==0) continue;
  print("Password: ", 0x0B);
  kbd_getline(pass,32,false);
  if(check_pass(uname,pass)){
   current_user.uid = (uname[0]=='r')?0:1000;
   for(int i=0; uname[i] && i<31; i++){ current_user.name[i]=uname[i]; current_user.name[i+1]=0; }
   current_user.is_root = (current_user.uid==0);
   print("\n[OK] Welcome ",0x0A); print(current_user.name,0x0F); print("\n",0x07);
   break;
  } else {
   print("\nLogin incorrect\n",0x0C);
  }
 }
 print("\n[",0x08); print(current_user.name,0x0A); print("@pizza-os ~]$ ",0x08);
}