#include <stdint.h>

struct tss {
  uint16_t back_link, :16;
  void *esp0;                         /* Ring 0 stack virtual address. */
  uint16_t ss0, :16;                  /* Ring 0 stack segment selector. */
  void *esp1;
  uint16_t ss1, :16;
  void *esp2;
  uint16_t ss2, :16;
  uint32_t cr3;
  void (*eip) (void);
  uint32_t eflags;
  uint32_t eax, ecx, edx, ebx;
  uint32_t esp, ebp, esi, edi;
  uint16_t es, :16;
  uint16_t cs, :16;
  uint16_t ss, :16;
  uint16_t ds, :16;
  uint16_t fs, :16;
  uint16_t gs, :16;
  uint16_t ldt, :16;
  uint16_t trace, bitmap;
};

struct tss * tss_return();
