section .text

global _idt_flush
extern idtp
_idt_flush:
  lidt [idtp]
  ret
