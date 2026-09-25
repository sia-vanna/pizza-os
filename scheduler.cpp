#include "scheduler.h"

static inline uint32_t get_cr3() {
    uint32_t cr3;
    asm volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

static Task tasks[MAX_TASKS];
static volatile int cur_task = 0;

void sched_init() {
    for(int i=0;i<MAX_TASKS;i++) tasks[i].alive=false;
    tasks[0].alive=true;
    tasks[0].cr3=get_cr3();
    cur_task=0;
}
int sched_add(void (*fn)()) {
    for(int i=1;i<MAX_TASKS;i++) if(!tasks[i].alive) {
        tasks[i].alive=true;
        tasks[i].cr3=get_cr3();
        uint32_t* top = (uint32_t*)(tasks[i].stack + sizeof(tasks[i].stack));
        *--top = 0x202;
        *--top = 0x08;
        *--top = (uint32_t)fn;
        *--top = 0;
        for(int k=0;k<8;k++) *--top=0;
        for(int k=0;k<4;k++) *--top=0x10;
        tasks[i].esp=(uint32_t)top;
        return i;
    }
    return -1;
}
extern "C" uint32_t isr_timer_cpp(uint32_t old_esp) {
    asm volatile("movb $0x20,%%al; outb %%al,$0x20":::"al");
    tasks[cur_task].esp=old_esp;
    int next=cur_task;
    for(int j=0;j<MAX_TASKS;j++){ next=(next+1)%MAX_TASKS; if(tasks[next].alive){cur_task=next;break;}}
    return tasks[cur_task].esp;
}
int sched_get_cur(){ return cur_task; }
void sched_yield(){ asm volatile("int $32"); }
