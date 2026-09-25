#include "user.h"
#include "fs.h"
#include <stdint.h>
extern void print(const char* s, unsigned char c=0x0F);
extern void clear(unsigned char c=0x0F);
extern void putchar(char c, unsigned char color=0x0F);
user current_user={0,"root",true};
static int slen(const char* s){int i=0;while(s[i])i++;return i;}
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
      if(i==0) continue; // <-- ignore ghost newline, stay on same prompt
      b[i]=0; putchar('\n',0x0F); break;
    }
    if(c=='\b'){ if(i>0){ i--; putchar('\b',0x0F); putchar(' ',0x0F); putchar('\b',0x0F);} continue; }
    if(c<32 || c>126) continue; // ignore control
    b[i++]=c; putchar(echo?c:'*',0x0F);
  } b[i]=0;
}
bool user_init(){ return true; }
void login_loop(){
  char u[32],p[32],cmd[64];
  // no leading \n!
  print("PizzaOS 1.0 (tty1)\n",0x08);
  while(1){
    print("pizza-os login: ",0x0F); kgetline(u,32,true);
    print("Password: ",0x0F); kgetline(p,32,false);
    bool ok = (slen(u)==5 && u[0]=='v'&&u[1]=='a'&&u[2]=='n'&&u[3]=='n'&&u[4]=='a' && slen(p)==5 && p[0]=='p'&&p[1]=='i'&&p[2]=='z'&&p[3]=='z'&&p[4]=='a') ||
              (slen(u)==4 && u[0]=='r'&&u[1]=='o'&&u[2]=='o'&&u[3]=='t' && slen(p)==4 && p[0]=='r'&&p[1]=='o'&&p[2]=='o'&&p[3]=='t');
    if(ok){
      print("Welcome ",0x0A); print(u,0x0F); print("!\n",0x0A);
      break;
    } else print("Login incorrect\n",0x0C);
  }
  while(1){ print(u,0x0A); print("@pizza-os$ ",0x08); kgetline(cmd,64,true); if(cmd[0]==0) continue; print("you: ",0x08); print(cmd,0x0F); print("\n",0x0F); }
}