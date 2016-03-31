global _isr_debug
global _isr0
global _isr1
global _isr2
global _isr3
global _isr4
global _isr5
global _isr6
global _isr7
global _isr8
global _isr9
global _isr10
global _isr11
global _isr12
global _isr13
global _isr14
global _isr15
global _isr16
global _isr17
global _isr18
global _isr19
global _isr20
global _isr21
global _isr22
global _isr23
global _isr24
global _isr25
global _isr26
global _isr27
global _isr28
global _isr29
global _isr30
global _isr31

_isr_debug:
    cli
    ;push 0
    ret

; ISR 0: Divide By Zero
_isr0:
    cli
    push byte 0
    push byte 0
    jmp isr_common_stub

; ISR 1: Debugging Exception
_isr1:
	cli
	push byte 0
	push byte 1
	jmp isr_common_stub

; ISR 2: Non Maskable Interrupt Exception
_isr2:
	cli
	push byte 0
	push byte 2
	jmp isr_common_stub

; ISR 3: Breakpoint Exception
_isr3:
	cli
	push byte 0
	push byte 3
	jmp isr_common_stub

; ISR 4: Into Detected Overflow Exception
_isr4:
	cli
	push byte 0
	push byte 4
	jmp isr_common_stub

; ISR 5: Out of Bounds Exception
_isr5:
	cli
	push byte 0
	push byte 5
	jmp isr_common_stub

; ISR 6: Invalid Opcode Exception
_isr6:
	cli
	push byte 0
	push byte 6
	jmp isr_common_stub

; ISR 7: No Coprocessor Exception
_isr7:
	cli
	push byte 0
	jmp isr_common_stub
    push byte 7

; ISR 8: Double Fault Exception
_isr8:
	cli
	push byte 8
	jmp isr_common_stub

; ISR 9: Coprocessor Segment Overrun Exception
_isr9:
	cli
	push byte 0
	push byte 9
	jmp isr_common_stub

; ISR 10: Bad TSS Exception
_isr10:
	cli
	push byte 10
	jmp isr_common_stub

; ISR 11: Segment Not Present Exception
_isr11:
	cli
	push byte 11
	jmp isr_common_stub

; ISR 12: Stack Fault Exception
_isr12:
	cli
	push byte 12
	jmp isr_common_stub

; ISR 13: General Protection Fault Exception
_isr13:
	cli
	push byte 13
	jmp isr_common_stub

; ISR 14: Page Fault Exception
_isr14:
	cli
	push byte 14
	jmp isr_common_stub

; ISR 15: Unknown Interrupt Exception
_isr15:
	cli
	push byte 0
	push byte 15
	jmp isr_common_stub

; ISR 16: Coprocessor Fault Exception
_isr16:
	cli
	push byte 0
	push byte 16
	jmp isr_common_stub

; ISR 17: Alignment Check Exception (Intel 486 and above)
_isr17:
	cli
	push byte 0
	push byte 17
	jmp isr_common_stub

; ISR 18: Machine Check Exception (Pentium/586 and above)
_isr18:
	cli
	push byte 0
	push byte 18
	jmp isr_common_stub

; ISRs 19-31: Reserved Exceptions
_isr19:
	cli
	push byte 0
	push byte 19
	jmp isr_common_stub

_isr20:
	cli
	push byte 0
	push byte 20
	jmp isr_common_stub

_isr21:
	cli
	push byte 0
	push byte 21
	jmp isr_common_stub

_isr22:
	cli
	push byte 0
	push byte 22
	jmp isr_common_stub

_isr23:
	cli
	push byte 0
	push byte 23
	jmp isr_common_stub

_isr24:
	cli
	push byte 0
	push byte 24
	jmp isr_common_stub

_isr25:
	cli
	push byte 0
	push byte 25
	jmp isr_common_stub

_isr26:
	cli
	push byte 0
	push byte 26
	jmp isr_common_stub

_isr27:
	push byte 0
	push byte 27
	jmp isr_common_stub

_isr28:
	push byte 0
	push byte 28
	jmp isr_common_stub

_isr29:
	push byte 0
	push byte 29
	jmp isr_common_stub

_isr30:
	push byte 0
	push byte 30
	jmp isr_common_stub

_isr31:
	push byte 0
	push byte 31
	jmp isr_common_stub

extern _fault_handler
isr_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x8   ; Load the Kernel Data Segment descriptor!
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp   ; Push us the stack
    push eax
    mov eax, _fault_handler
    call eax       ; A special call, preserves the 'eip' register
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8 ; Cleans up the pushed error code and pushed ISR number
    iret    ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!
