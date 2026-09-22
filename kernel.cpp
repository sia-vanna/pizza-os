#include "mm.h"
static volatile unsigned short* vga = (unsigned short*)0xB8000;
static int cx = 0, cy = 0;
void clear(unsigned char color=0x0F){
 for(int i=0;i<80*25;i++) vga[i] = (color<<8) | ' ';
 cx=0; cy=0;
}
static inline void outb(unsigned short port, unsigned char val){
 asm volatile("outb %0, %1" :: "a"(val), "Nd"(port));
}
static inline void serial(char c){
 outb(0xE9, c);
 asm volatile("outb %0, %1" :: "a"(c), "d"((unsigned short)0x3F8));
}
void putchar(char c, unsigned char color=0x0F){
 if(c=='\n'){ cx=0; cy++; serial('\n');
 if(cy>=25){ cy=24; for(int y=1;y<25;y++) for(int x=0;x<80;x++) vga[(y-1)*80+x]=vga[y*80+x];
 for(int x=0;x<80;x++) vga[24*80+x]=(color<<8)|' '; } return; }
 if(cx>=80){ cx=0; cy++; } if(cy>=25) clear();
 vga[cy*80+cx] = (color<<8) | (unsigned char)c; cx++; serial(c);
}
void print(const char* s, unsigned char color=0x0F){ while(*s) putchar(*s++, color); }
void print_int(int n, unsigned char color){
 if(n==0){ putchar('0', color); return; }
 bool neg=false; if(n<0){neg=true; n=-n;}
 char rev[12]; int r=0; while(n>0){ rev[r++]='0'+n%10; n/=10; }
 if(neg) rev[r++]='-';
 char buf[12]; int i=0; while(r--) buf[i++]=rev[r]; buf[i]=0; print(buf, color);
}
void print_hex(uint64_t v, unsigned char color){
 char buf[17]; for(int j=15;j>=0;j--){ int nib = v & 0xF; buf[j] = nib < 10? '0'+nib : 'a'+nib-10; v >>= 4; } buf[16]=0; print(buf, color);
}
void print_dollars(int cents, unsigned char color){
 int dollars = cents / 100; int rem = cents % 100;
 print("$", color); print_int(dollars, color); putchar('.', color);
 if(rem<10) putchar('0', color); print_int(rem, color);
}
struct Order { const char* pizza; int price_cents; int cost_cents; };

extern "C" void kernel_main(uint32_t magic, uint32_t mb_info_ptr){
 clear();
 print("================================================================================\n", 0x0C);
 print(" hacking-pizza.ca // impartial OS // v0.2 // pizza-os.bin @ 1MB\n", 0x0E);
 print("================================================================================\n\n", 0x0C);
 print("[OK] ", 0x0A); print("C++ kernel loaded\n", 0x07);
 print("[OK] ", 0x0A); print("VGA 0xB8000 initialized (80x25)\n", 0x07);
 print("[OK] ", 0x0A); print("No telemetry. No account. No feed.\n\n", 0x08);

 bool is_mb2 = (magic == 0x36d76289);
 print("[INFO] boot: ", 0x0B);
 print(is_mb2? "multiboot2 (GRUB)\n" : "PVH / direct (fallback)\n", 0x07);

 bool real_map = mm_init(magic, mb_info_ptr);
 if (real_map) {
  print("[OK] ", 0x0A); print("Real memory map found\n", 0x07);
 } else {
  print("[OK] ", 0x0A); print("Using fallback allocator 16MB-64MB\n", 0x07);
 }
 mm_print_map();
 print("\n[TEST] ", 0x0B); print("Bump allocator test:\n", 0x07);
 void* p1 = mm_alloc_pages(1);
 void* p2 = mm_alloc_pages(1);
 print(" alloc1 = 0x", 0x07); print_hex((uint64_t)(uintptr_t)p1, 0x0F); print("\n", 0x07);
 print(" alloc2 = 0x", 0x07); print_hex((uint64_t)(uintptr_t)p2, 0x0F); print("\n\n", 0x07);

 Order sample[] = { {"Pepperoni 14\" ", 1899, 620}, {"Meat Lovers ", 2199, 780}, {"Cheese 12\" ", 1599, 410}, {"Veggie Deluxe", 2099, 690}, };
 int n=4;
 print(" ORDERS IN RAM:\n", 0x0B);
 print(" ---------------------------------\n", 0x08);
 for(int i=0;i<n;i++){ print(" ", 0x07); print(sample[i].pizza, 0x0F); print(" -> ", 0x08); print_dollars(sample[i].price_cents, 0x0F); print("\n", 0x0F); }
 print(" ---------------------------------\n\n", 0x08);
 int revenue=0, cost=0; for(int i=0;i<n;i++){ revenue+=sample[i].price_cents; cost+=sample[i].cost_cents; }
 int profit = revenue - cost;
 print(" REVENUE: ", 0x07); print_dollars(revenue, 0x0A); print("\n", 0x07);
 print(" COST: ", 0x07); print_dollars(cost, 0x0C); print("\n", 0x07);
 print(" PROFIT: ", 0x07); print_dollars(profit, 0x0E); print(" (", 0x08); print_int((profit*100)/revenue, 0x08); print("% margin)", 0x08);
 print("\n\n------------------------------------\n", 0x08);
 print("pizza-os> ", 0x0A); print("_", 0x0F);
 while(1){ __asm__ volatile("hlt"); }
}