#include "fs.h"
#include "mm.h"
#include <stdint.h>

extern void print(const char* s, unsigned char color=0x0F);
extern void print_dec(int n, unsigned char color=0x0F);

static int k_strlen(const char* s){ int i=0; while(s[i]) i++; return i; }
static void k_strcpy(char* d, const char* s){ while(*s) *d++=*s++; *d=0; }
static int k_strcmp(const char* a, const char* b){ while(*a && *a==*b){a++;b++;} return (unsigned char)*a - (unsigned char)*b; }

#define MAX_FILES 16
static file files[MAX_FILES];
static int file_count=0;

bool fs_init(uint32_t mbi){
  (void)mbi;
  file_count=0;
  k_strcpy(files[0].path, "/etc/passwd");
  const char* pw = "root:x:0:0:root:/root:/bin/sh\nvanna:x:1000:1000::/home/vanna:/bin/sh\n";
  files[0].size = k_strlen(pw);
  files[0].data = (uint8_t*)mm_alloc_pages(1);
  for(int i=0;i<files[0].size;i++) files[0].data[i]=pw[i];
  files[0].data[files[0].size]=0;
  k_strcpy(files[1].path, "/etc/shadow");
  const char* sh = "root:pizza\nvanna:pizza\n";
  files[1].size = k_strlen(sh);
  files[1].data = (uint8_t*)mm_alloc_pages(1);
  for(int i=0;i<files[1].size;i++) files[1].data[i]=sh[i];
  files[1].data[files[1].size]=0;
  file_count=2;
  print("fs: ramfs 2 files\n", 0x0A);
  return true;
}
file* fs_open(const char* path){
  for(int i=0;i<file_count;i++) if(k_strcmp(files[i].path, path)==0) return &files[i];
  return 0;
}
void fs_list(){
  for(int i=0;i<file_count;i++){ print(files[i].path,0x07); print("\n",0x07); }
}
