#pragma once
#include <stdint.h>

struct file {
  char path[64];
  uint32_t size;
  uint8_t* data;
};

bool fs_init(uint32_t multiboot_info_ptr);
file* fs_open(const char* path);
void fs_list();