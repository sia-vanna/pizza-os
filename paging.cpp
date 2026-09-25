#include "paging.h"
#define FLAG_P 1
#define FLAG_RW 2
static uint32_t* page_dir = (uint32_t*)0x1000;
static uint32_t* first_pt = (uint32_t*)0x2000;

void map_page(uint32_t virt, uint32_t phys, uint32_t flags){
  uint32_t pd = virt>>22;
  uint32_t pt = (virt>>12)&0x3FF;
  if(!(page_dir[pd]&FLAG_P)){
    // alloc new pt at heap (simplified - use next free 4K)
    extern uint32_t heap_ptr; // hack
    uint32_t* new_pt = (uint32_t*)0x3000; // for now second table
    page_dir[pd]=(uint32_t)new_pt | FLAG_P | FLAG_RW;
  }
  uint32_t* pt_addr = (uint32_t*)(page_dir[pd]&~0xFFF);
  pt_addr[pt]= (phys&~0xFFF) | flags;
}

void paging_init(){
  for(int i=0;i<1024;i++) page_dir[i]=0;
  for(int i=0;i<1024;i++) first_pt[i]= (i*0x1000) | FLAG_P | FLAG_RW; // identity 0-4MB
  page_dir[0]= (uint32_t)first_pt | FLAG_P | FLAG_RW;
  // map 0xC0000000 -> 0 for higher half optional
  // page_dir[768]= (uint32_t)first_pt | FLAG_P | FLAG_RW;
  asm volatile("mov %0, %%cr3"::"r"(page_dir));
  uint32_t cr0; asm volatile("mov %%cr0,%0":"=r"(cr0)); cr0|=0x80000000; asm volatile("mov %0,%%cr0"::"r"(cr0));
}