.global _gdt_flush
.extern gp
_gdt_flush:
	lgdt (gp)
	mov %ax, 0x10
	mov %ds, %ax
	mov %es, %ax
	mov %fs, %ax
	mov %gs, %ax
	mov %ss, %ax
	jmp flush2
flush2:
	ret
