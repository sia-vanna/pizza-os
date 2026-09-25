#pragma once
#include <stdint.h>
struct File { const char* name; uint8_t* data; uint32_t size; };
bool fs_init(uint32_t mb_info);
File* fs_find(const char* name);
int fs_list(File** out, int max);
void fs_list();
bool fs_create(const char* name);
int fs_append(const char* name, const char* data);
int fs_cat(const char* name);
