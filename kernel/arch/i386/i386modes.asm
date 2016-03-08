extern kernel_pmode
global _386_pmode
_386_pmode:
  cli
  mov eax, cr0
  or eax, 1
  mov cr0, eax
  jmp 08h:kernel_pmode
