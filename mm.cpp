#include "mm.h"

struct Block { size_t size; bool free; Block* next; };
static Block* heap_head = nullptr;
static uint32_t heap_ptr = 0;
static uint32_t heap_end = 0;

struct MbTag { uint32_t type,size; };
struct MbMmapEntry { uint64_t base,len; uint32_t type,resv; } __attribute__((packed));

void* kmalloc(size_t sz){
  if(sz==0) return nullptr;
  sz = (sz+7)&~7;
  for(Block* b=heap_head;b;b=b->next){
    if(b->free && b->size>=sz){ b->free=false; return (void*)(b+1); }
  }
  if(heap_ptr+sizeof(Block)+sz > heap_end) return nullptr;
  Block* b = (Block*)heap_ptr;
  heap_ptr += sizeof(Block)+sz;
  b->size=sz; b->free=false; b->next=heap_head; heap_head=b;
  return (void*)(b+1);
}
void kfree(void* p){
  if(!p) return;
  Block* b = (Block*)p -1;
  b->free=true;
  // coalesce next free blocks
  for(Block* cur=heap_head; cur && cur->next; ){
    if(cur->free && cur->next->free){ cur->size+=sizeof(Block)+cur->next->size; cur->next=cur->next->next; }
    else cur=cur->next;
  }
}
void* krealloc(void* p, size_t s){
  if(!p) return kmalloc(s);
  Block* b=(Block*)p-1;
  if(b->size>=s) return p;
  void* np=kmalloc(s);
  if(np){ for(size_t i=0;i<b->size && i<s;i++) ((uint8_t*)np)[i]=((uint8_t*)p)[i]; kfree(p); }
  return np;
}

void mm_init(uint32_t magic, uint32_t info){
  // Find highest RAM from multiboot2 mmap tag=6
  uint32_t mem_top = 16*1024*1024; // fallback 16M
  auto* tag = (MbTag*)(info+8);
  while(tag->type!=0){
    if(tag->type==6){
      auto* entries = (uint8_t*)tag+16;
      uint32_t count = (tag->size-16)/24;
      for(uint32_t i=0;i<count;i++){
        auto* e = (MbMmapEntry*)(entries+i*24);
        if(e->type==1 && e->base>=0x100000){
          uint64_t top = e->base+e->len;
          if(top>mem_top && top<0xFFFFFFFF) mem_top=(uint32_t)top;
        }
      }
    }
    if(tag->type==3){ // module - ai_model.bin
      // tag+8 = mod_start, mod_end, string
    }
    tag = (MbTag*)((uint8_t*)tag + ((tag->size+7)&~7));
  }
  extern uint32_t _kernel_end; // define in linker.ld
  uint32_t k_end = (uint32_t)&_kernel_end;
  k_end = (k_end+0xFFF)&~0xFFF;
  heap_ptr = k_end;
  heap_end = mem_top;
  heap_head=nullptr;
}