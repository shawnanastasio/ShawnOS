section .text

global _i386_enter_pmode
_i386_enter_pmode:
  cli
  mov eax, cr0
  or eax, 1
  mov cr0, eax
  ret
