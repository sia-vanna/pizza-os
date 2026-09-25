#pragma once
#include <stdint.h>
void paging_init();
void map_page(uint32_t virt, uint32_t phys, uint32_t flags);