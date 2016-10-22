section .text

global _gdt_flush
extern gp
_gdt_flush:
    lgdt [gp]
    jmp reloadSegments

reloadSegments:
    ; Reload CS register containing code selector:
    jmp 0x08:reload_CS ; 0x08 points at the new code selector

reload_CS:
    ; Reload data segment registers:
    mov   ax, 0x10 ; 0x10 points at the new data selector
    mov   ds, ax
    mov   es, ax
    mov   fs, ax
    mov   gs, ax
    mov   ss, ax
    ret
