#include "user.h"
#include "fs.h"
#include "scheduler.h"
#include <stdint.h>

extern void print(const char* s, unsigned char c=0x0F);
extern void clear(unsigned char c=0x0F);
extern void putchar(char c, unsigned char color=0x0F);

user current_user={0,"root",true};

volatile int cntA=0, cntB=0;
static volatile unsigned short* vga=(unsigned short*)0xB8000;

static int slen(const char* s){int i=0;while(s[i])i++;return i;}
static int sstrncmp(const char* a,const char* b,int n){for(int i=0;i<n;i++){if(a[i]!=b[i]) return a[i]-b[i]; if(!a[i]) return 0;}return 0;}
static int sstrcmp(const char* a,const char* b){int i=0;while(a[i]&&b[i]){if(a[i]!=b[i]) return a[i]-b[i]; i++;} return a[i]-b[i];}
static inline uint8_t inb(uint16_t p){uint8_t r; asm volatile("inb %1,%0":"=a"(r):"Nd"(p)); return r;}

static char kget(){
  while(!(inb(0x3F8+5)&1)) asm volatile("pause");
  char c=inb(0x3F8);
  if(c=='\r') return '\n';
  return c;
}

static void kgetline(char* b,int max,bool echo){
  int i=0;
  while(i<max-1){
    char c=kget();
    if(c=='\n'){
      if(i==0) continue;
      b[i]=0; putchar('\n',0x0F); break;
    }
    if(c=='\b'){
      if(i>0){ i--; putchar('\b',0x0F); putchar(' ',0x0F); putchar('\b',0x0F);}
      continue;
    }
    if(c<32 || c>126) continue;
    b[i++]=c;
    putchar(echo?c:'*',0x0F);
  }
  b[i]=0;
}

bool user_init(){ return true; }

// ---- threads ----
void thread_a(){ while(1){ cntA++; for(volatile int i=0;i<8000000;i++) asm volatile("nop"); } }
void thread_b(){ while(1){ cntB++; for(volatile int i=0;i<8000000;i++) asm volatile("nop"); } }
void status_thread(){
  while(1){
    asm volatile("cli");
    vga[24*80+0]=(0x0C<<8)|'A';
    vga[24*80+1]=(0x0F<<8)|('0'+(cntA%10));
    vga[24*80+3]=(0x0A<<8)|'a';
    vga[24*80+4]=(0x0F<<8)|('0'+(cntB%10));
    vga[24*80+6]=(0x0E<<8)|('0'+(sched_get_cur()));
    asm volatile("sti");
    for(volatile int i=0;i<15000000;i++) asm volatile("nop");
  }
}

void login_loop(){
  char u[32],p[32],cmd[64];
  print("PizzaOS 1.0 (tty1)\n",0x08);
  while(1){
    print("pizza-os login: ",0x0F);
    kgetline(u,32,true);
    print("Password: ",0x0F);
    kgetline(p,32,false);
    bool ok = (slen(u)==5 && u[0]=='v'&&u[1]=='a'&&u[2]=='n'&&u[3]=='n'&&u[4]=='a' && slen(p)==5 && p[0]=='p'&&p[1]=='i'&&p[2]=='z'&&p[3]=='z'&&p[4]=='a') ||
              (slen(u)==4 && u[0]=='r'&&u[1]=='o'&&u[2]=='o'&&u[3]=='t' && slen(p)==4 && p[0]=='r'&&p[1]=='o'&&p[2]=='o'&&p[3]=='t');
    if(ok){ print("Welcome ",0x0A); print(u,0x0F); print("!\n",0x0A); break; }
    else print("Login incorrect\n",0x0C);
  }

  print("type help for cmds\n",0x07);

  while(1){
    print(u,0x0A); print("@pizza-os$ ",0x08);
    kgetline(cmd,64,true);
    if(cmd[0]==0) continue;

    if(sstrcmp(cmd,"help")==0){
      print("cmds: help clear ls cat <file> mt mttest\n",0x0F);
    }
    else if(sstrcmp(cmd,"clear")==0){ clear(); }
    else if(sstrcmp(cmd,"ls")==0){ fs_list(); }
    else if(sstrncmp(cmd,"cat ",4)==0){
      char* fname=cmd+4; while(*fname==' ') fname++;
      if(fs_cat(fname)<0) print("No such file\n",0x0C);
    }
    else if(sstrcmp(cmd,"mt")==0){
      print("Starting 3 threads...\n",0x0E);
      sched_init();
      sched_add(thread_a);
      sched_add(thread_b);
      sched_add(status_thread);
      asm volatile("sti");
      print("Bottom line A0 a0 spinning = working. Keep typing!\n",0x0A);
    }
    else if(sstrcmp(cmd,"mttest")==0){
      fs_create("a.log"); fs_create("b.log");
      print("Writing a.log + b.log 20 ticks...\n",0x0E);
      for(int i=0;i<20;i++){
        char b1[8]={'A','0'+(char)(cntA%10),'\n',0};
        char b2[8]={'b','0'+(char)(cntB%10),'\n',0};
        fs_append("a.log", b1);
        fs_append("b.log", b2);
        print(".",0x07);
        for(volatile int k=0;k<8000000;k++) asm volatile("nop");
      }
      print("\nDone. try: cat a.log, cat b.log\n",0x0A);
    }
    else{
      print("you: ",0x08); print(cmd,0x0F); print("\n",0x0F);
    }
  }
}