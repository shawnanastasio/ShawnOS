struct idt_entry {
    uint16_t base_lo;
    uint16_t sel;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_hi;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct idt_gate_debug {
    uint8_t num;
    uint64_t base;
    uint16_t sel;
    uint8_t flags;
};

void idt_install();
void _idt_set_gate(uint8_t num, uint32_t base, uint16_t sel,
                   uint8_t flags);
