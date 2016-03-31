global _gdt_flush
extern gp
_gdt_flush:
	lgdt [gp]
	mov [8h], ax
	mov ax, ds
	mov ax, es
	mov ax, fs
	mov ax, gs
	mov ax, ss
	jmp flush2
flush2:
	ret
