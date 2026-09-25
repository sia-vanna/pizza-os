#pragma once
#include <stdint.h>
#include <stddef.h>
void mm_init(uint32_t magic, uint32_t mb_info);
void* kmalloc(size_t size);
void kfree(void* ptr);
void* krealloc(void* ptr, size_t size);

// C++ new/delete
inline void* operator new(size_t s){ return kmalloc(s); }
inline void* operator new[](size_t s){ return kmalloc(s); }
inline void operator delete(void* p) noexcept { kfree(p); }
inline void operator delete[](void* p) noexcept { kfree(p); }
inline void operator delete(void* p, size_t) noexcept { kfree(p); }