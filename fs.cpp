#include "fs.h"
#include "mm.h"
#include <stdint.h>

static File files[16];
static int file_count=0;

bool fs_init(uint32_t mb_info){
 (void)mb_info;
 file_count=2;

 // 0: readme
 files[0].name="readme.txt";
 files[0].data=(uint8_t*)kmalloc(4096);
 if(!files[0].data) return false;
 const char* t="Welcome to PizzaOS FS\n";
 int i=0; for(; t[i]; i++) files[0].data[i]=(uint8_t)t[i];
 files[0].size=i;

 // 1: shadow - vanna:pizza and root:root
 files[1].name="shadow";
 files[1].data=(uint8_t*)kmalloc(4096);
 if(!files[1].data) return false;
 const char* s="vanna:pizza\nroot:root\n";
 int j=0; for(; s[j]; j++) files[1].data[j]=(uint8_t)s[j];
 files[1].size=j;
 files[1].data[files[1].size]=0;

 return true;
}

File* fs_find(const char* name){
 // strip leading / and etc/
 const char* n=name;
 if(n[0]=='/') n++;
 if(n[0]=='e' && n[1]=='t' && n[2]=='c' && n[3]=='/') n+=4;

 for(int i=0;i<file_count;i++){
   const char* a=files[i].name; const char* b=n;
   int k=0; for(; a[k] && b[k] && a[k]==b[k]; k++);
   if(a[k]==b[k]) return &files[i];
 }
 return nullptr;
}

int fs_list(File** out, int max){
 int n=file_count<max?file_count:max;
 for(int i=0;i<n;i++) out[i]=&files[i];
 return n;
}

void fs_list(){
 extern void print(const char*, unsigned char);
 for(int i=0;i<file_count;i++){
   print(files[i].name,0x0F);
   print("\n",0x0F);
 }
}