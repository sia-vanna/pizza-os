#pragma once
#include <stdint.h>

struct MemRegion {
  uint64_t base;
  uint64_t length;
  uint32_t type; // 1 = usable
};


bool mm_init(uint32_t magic, uint32_t mb_info_ptr);

void* mm_alloc_pages(uint32_t count);
uint64_t mm_total_usable_bytes();
void mm_print_map();