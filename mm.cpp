#include "mm.h"

extern void print(const char*, unsigned char);
extern void print_hex(uint64_t, unsigned char);

#define MB2_MAGIC 0x36d76289

static MemRegion regions[64];
static int region_count = 0;
static uint64_t next_free = 0;
static uint64_t free_end = 0;
static bool has_real_map = false;

bool mm_init(uint32_t magic, uint32_t mb_info_ptr) {
  if (magic != MB2_MAGIC || mb_info_ptr == 0) {
    next_free = 0x1000000;   
    free_end  = 0x4000000;  
    has_real_map = false;
    return false;
  }

  uint8_t* ptr = (uint8_t*)(uintptr_t)mb_info_ptr;
  uint32_t total_size = *(uint32_t*)ptr;
  uint8_t* tag = ptr + 8; 
  while ((uintptr_t)tag < (uintptr_t)ptr + total_size) {
    uint32_t type = *(uint32_t*)tag;
    uint32_t size = *(uint32_t*)(tag + 4);
    if (type == 0) break;

    if (type == 6) { // memory map tag
      uint32_t entry_size = *(uint32_t*)(tag + 8);
      uint8_t* entry = tag + 16;
      uint8_t* map_end = tag + size;
      while (entry < map_end && region_count < 64) {
        uint64_t base   = *(uint64_t*)entry;
        uint64_t length = *(uint64_t*)(entry + 8);
        uint32_t mtype  = *(uint32_t*)(entry + 16);
        regions[region_count++] = {base, length, mtype};
        entry += entry_size;
      }
    }
    tag += (size + 7) & ~7u; 
  }

  uint64_t best_base = 0, best_len = 0;
  for (int i = 0; i < region_count; i++) {
    if (regions[i].type == 1 && regions[i].base >= 0x100000 && regions[i].length > best_len) {
      best_base = regions[i].base;
      best_len  = regions[i].length;
    }
  }

  if (best_len == 0) {
    next_free = 0x1000000;
    free_end  = 0x4000000;
    has_real_map = false;
    return false;
  }

  next_free = best_base;
  free_end  = best_base + best_len;
  has_real_map = true;
  return true;
}

void* mm_alloc_pages(uint32_t count) {
  uint64_t size = (uint64_t)count * 4096;
  if (next_free + size > free_end) return nullptr; 
  void* result = (void*)(uintptr_t)next_free;
  next_free += size;
  return result;
}

uint64_t mm_total_usable_bytes() {
  if (!has_real_map) return free_end - next_free;
  uint64_t total = 0;
  for (int i = 0; i < region_count; i++)
    if (regions[i].type == 1) total += regions[i].length;
  return total;
}

void mm_print_map() {
  if (!has_real_map) {
    print(" MEMORY MAP: (fallback range, no multiboot2 info)\n", 0x0E);
    print("  usable: 0x", 0x07); print_hex(next_free, 0x0F);
    print(" - 0x", 0x07); print_hex(free_end, 0x0F);
    print("\n", 0x07);
    return;
  }
  print(" MEMORY MAP:\n", 0x0B);
  for (int i = 0; i < region_count; i++) {
    print("  base=0x", 0x07);
    print_hex(regions[i].base, 0x0F);
    print(regions[i].type == 1 ? "  [usable]\n" : "  [reserved]\n",
          regions[i].type == 1 ? 0x0A : 0x08);
  }
}