#include "fs.h"
#include "mm.h"
#include <stdint.h>

static File files[16];
static int file_count=0;

bool fs_init(uint32_t mb_info){
  (void)mb_info;
  file_count=2;
  files[0].name="readme.txt";
  files[0].data=(uint8_t*)kmalloc(4096);
  if(!files[0].data) return false;
  const char* t="Welcome to PizzaOS FS\n";
  int i=0; for(; t[i]; i++) files[0].data[i]=(uint8_t)t[i];
  files[0].size=i;
  files[0].data[i]=0;

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
  const char* n=name;
  if(n[0]=='/') n++;
  if(n[0]=='e' && n[1]=='t' && n[2]=='c' && n[3]=='/') n+=4;
  for(int i=0;i<file_count;i++){
    const char* a=files[i].name;
    const char* b=n;
    int k=0; for(; a[k] && b[k] && a[k]==b[k]; k++);
    if(a[k]==0 && b[k]==0) return &files[i];
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

// --- needed for mttest ---
bool fs_create(const char* name){
  if(fs_find(name)) return true;
  if(file_count>=16) return false;
  files[file_count].name=name; // for quick test keep ptr, ideally strdup
  files[file_count].data=(uint8_t*)kmalloc(4096);
  if(!files[file_count].data) return false;
  files[file_count].data[0]=0;
  files[file_count].size=0;
  file_count++;
  return true;
}

int fs_append(const char* name, const char* data){
  asm volatile("cli");
  File* f = fs_find(name);
  if(!f){ asm volatile("sti"); return -1; }
  int len=0; while(data[len]) len++;
  if(f->size + len >= 4095){ asm volatile("sti"); return -1; }
  for(int i=0;i<len;i++) f->data[f->size+i]=(uint8_t)data[i];
  f->size += len;
  f->data[f->size]=0;
  asm volatile("sti");
  return len;
}

int fs_cat(const char* name){
  File* f = fs_find(name);
  if(!f) return -1;
  extern void print(const char*, unsigned char);
  // print without holding cli for whole time
  for(uint32_t i=0;i<f->size;i++){
    char s[2]; s[0]=(char)f->data[i]; s[1]=0;
    print(s,0x0F);
  }
  print("\n",0x0F);
  return 0;
}
