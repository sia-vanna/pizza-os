#include "mm.h"
#include "fs.h"
#include "user.h"

static volatile unsigned short* vga = (unsigned short*)0xB8000;
static int cx = 0, cy = 0;

void clear(unsigned char color=0x0F){
    for(int i=0;i<80*25;i++) vga[i]=(color<<8)|' ';
    cx=0; cy=0;
}
static inline void outb(unsigned short p, unsigned char v){
    asm volatile("outb %0,%1"::"a"(v),"Nd"(p));
}
void putchar(char c, unsigned char color=0x0F){
    if(c=='\n'){
        cx=0; cy++;
        if(cy>=25){
            cy=24;
            for(int y=1;y<25;y++) for(int x=0;x<80;x++) vga[(y-1)*80+x]=vga[y*80+x];
            for(int x=0;x<80;x++) vga[24*80+x]=(color<<8)|' ';
        }
        outb(0x3F8, '\n');
        return;
    }
    if(cx>=80){ cx=0; cy++; }
    if(cy>=25) clear();
    vga[cy*80+cx]=(color<<8)|(unsigned char)c;
    cx++;
    outb(0x3F8, c);
}
void print(const char* s, unsigned char color=0x0F){ while(*s) putchar(*s++, color); }

extern "C" void kernel_main(uint32_t magic, uint32_t mb_info_ptr){
    clear();

    print(" Welcome to PizzaOS.\n",0x0F);
    print(" Quantum Slice. Extra Parmesan.\n\n",0x08);

    print(" Booting...\n",0x07);

    // init silently (no prints)
    mm_init(magic, mb_info_ptr);
    fs_init(mb_info_ptr);
    user_init();

    print("\n Ready.\n\n",0x0A);
    print("--------------------------------------------------------------------------------\n",0x08);

    // go straight to login -> pizza menu
    login_loop();

    while(1) __asm__ volatile("hlt");
}