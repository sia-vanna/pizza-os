#pragma once
#include <stdint.h>
#define MAX_TASKS 8
struct Task { bool alive; uint32_t esp; uint32_t cr3; uint8_t stack[4096]; };
void sched_init();
int sched_add(void (*fn)());
int sched_get_cur();
void sched_yield();
