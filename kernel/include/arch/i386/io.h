static inline void IoWrite8(uint32_t port, uint8_t data)
{
    __asm__ volatile("outb %b0, %w1" : : "a" (data), "Nd" (port));
}

static inline uint8_t IoRead8(uint32_t port)
{
    uint8_t data;
    __asm__ volatile("inb %w1, %b0" : "=a" (data) : "Nd" (port));
    return data;
}

static inline void IoWrite16(uint32_t port, uint16_t data)
{
    __asm__ volatile("outw %w0, %w1" : : "a" (data), "Nd" (port));
}

static inline uint16_t IoRead16(uint32_t port)
{
    uint16_t data;
    __asm__ volatile("inw %w1, %w0" : "=a" (data) : "Nd" (port));
    return data;
}

static inline void IoWrite32(uint32_t port, uint32_t data)
{
    __asm__ volatile("outl %0, %w1" : : "a" (data), "Nd" (port));
}

static inline uint32_t IoRead32(uint32_t port)
{
    uint32_t data;
    __asm__ volatile("inl %w1, %0" : "=a" (data) : "Nd" (port));
    return data;
}

static inline void io_wait()
{
    __asm__ volatile("jmp 1f;1:jmp 1f;1:");
}
