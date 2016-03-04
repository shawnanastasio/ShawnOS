struct gdt_entry {
  unsigned short limit_low;
  unsigned short base_low;
  unsigned char base_middle;
  unsigned char access;
  unsigned char granularity;
  unsigned char base_high;
} __attribute__((packed));

struct gdt_ptr {
  unsigned short limit;
  unsigned int base;
} __attribute__((packed));

struct gdt_entry gdt[4];
struct gdt_ptr gp;

extern void _gdt_flush();

void _gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran) {
  gdt[num].base_low = (base & 0xFFFF);
  gdt[num].base_middle = (base >> 16) & 0xFF;
  gdt[num].base_high = (base >> 24) & 0xF;

  gdt[num].limit_low = (limit & 0xFFFF);
  gdt[num].granularity = ((limit >> 16) & 0x0F);

  gdt[num].granularity |= (gran & 0xF0);
  gdt[num].access = access;
}

void gdt_install() {
  gp.limit = (sizeof(struct gdt_entry) * 3) - 1;
  gp.base = (unsigned int)&gdt;

  /**
   * Load up GDT with required gates
   * gate 0 - NULL
   * gate 1 - kernel code segment descriptor
   * gate 2 - data segment descriptor
   * TODO: gate 3 - TSS segment descriptor
   */
  _gdt_set_gate(0, 0, 0, 0, 0);
  _gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
  _gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

  _gdt_flush();
}
