global init_protect

extern init_8259A
;-------software int-----
extern divide_error
extern single_step_exception
extern nmi
extern breakpoint_exception
extern overflow
extern bounds_check
extern inval_opcode
extern copr_not_available
extern double_fault
extern copr_seg_overrun
extern inval_tss
extern segment_not_present
extern stack_exception
extern general_protection
extern page_fault
extern copr_error
;------hardware int-------
extern hwint00
extern hwint01
extern hwint02
extern hwint03
extern hwint04
extern hwint05
extern hwint06
extern hwint07
extern hwint08
extern hwint09
extern hwint10
extern hwint11
extern hwint12
extern hwint13
extern hwint14
extern hwint15

;---other type int-------
extern sys_call

%include "global.inc"


;void init_protect();

init_protect:
	push ebp
	mov ebp , esp

	call init_8259A

;----register software int--------
	push dword PRIVILEGE_KRNL
	push dword divide_error
	push dword DA_386IGATE
	push dword INT_VECTOR_DIVIDE
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword single_step_exception
	push dword DA_386IGATE
	push dword INT_VECTOR_DEBUG
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword nmi
	push dword DA_386IGATE
	push dword INT_VECTOR_NMI
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword breakpoint_exception
	push dword DA_386IGATE
	push dword INT_VECTOR_BREAKPOINT
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword overflow
	push dword DA_386IGATE
	push dword INT_VECTOR_OVERFLOW
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword bounds_check
	push dword DA_386IGATE
	push dword INT_VECTOR_BOUNDS
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword inval_opcode
	push dword DA_386IGATE
	push dword INT_VECTOR_INVAL_OP
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword copr_not_available
	push dword DA_386IGATE
	push dword INT_VECTOR_COPROC_NOT
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword double_fault
	push dword DA_386IGATE
	push dword INT_VECTOR_DOUBLE_FAULT
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword copr_seg_overrun
	push dword DA_386IGATE
	push dword INT_VECTOR_COPROC_SEG
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword inval_tss
	push dword DA_386IGATE
	push dword INT_VECTOR_INVAL_TSS
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword segment_not_present
	push dword DA_386IGATE
	push dword INT_VECTOR_SEG_NOT
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword stack_exception
	push dword DA_386IGATE
	push dword INT_VECTOR_STACK_FAULT
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword general_protection
	push dword DA_386IGATE
	push dword INT_VECTOR_PROTECTION
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword page_fault
	push dword DA_386IGATE
	push dword INT_VECTOR_PAGE_FAULT
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword copr_error
	push dword DA_386IGATE
	push dword INT_VECTOR_COPROC_ERR
	call init_idt_descriptor
	add esp , 16

;----register hardware int-------
;----master------
	push dword PRIVILEGE_KRNL
	push dword hwint00
	push dword DA_386IGATE
	push dword INT_VECTOR_IRQ0 + 0
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword hwint01
	push dword DA_386IGATE
	push dword INT_VECTOR_IRQ0 + 1
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword hwint02
	push dword DA_386IGATE
	push dword INT_VECTOR_IRQ0 + 2
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword hwint03
	push dword DA_386IGATE
	push dword INT_VECTOR_IRQ0 + 3
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword hwint04
	push dword DA_386IGATE
	push dword INT_VECTOR_IRQ0 + 4
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword hwint05
	push dword DA_386IGATE
	push dword INT_VECTOR_IRQ0 + 5
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword hwint06
	push dword DA_386IGATE
	push dword INT_VECTOR_IRQ0 + 6
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword hwint07
	push dword DA_386IGATE
	push dword INT_VECTOR_IRQ0 + 7
	call init_idt_descriptor
	add esp , 16

;------slave----

	push dword PRIVILEGE_KRNL
	push dword hwint08
	push dword DA_386IGATE
	push dword INT_VECTOR_IRQ8 + 0
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword hwint09
	push dword DA_386IGATE
	push dword INT_VECTOR_IRQ8 + 1
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword hwint10
	push dword DA_386IGATE
	push dword INT_VECTOR_IRQ8 + 2
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword hwint11
	push dword DA_386IGATE
	push dword INT_VECTOR_IRQ8 + 3
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword hwint12
	push dword DA_386IGATE
	push dword INT_VECTOR_IRQ8 + 4
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword hwint13
	push dword DA_386IGATE
	push dword INT_VECTOR_IRQ8 + 5
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword hwint14
	push dword DA_386IGATE
	push dword INT_VECTOR_IRQ8 + 6
	call init_idt_descriptor
	add esp , 16

	push dword PRIVILEGE_KRNL
	push dword hwint15
	push dword DA_386IGATE
	push dword INT_VECTOR_IRQ8 + 7
	call init_idt_descriptor
	add esp , 16

;-------register other type of int----------------------
;register syscall 0x90 ;can be used by user
	push dword PRIVILEGE_USER
	push dword sys_call
	push dword DA_386IGATE
	push dword INT_VECTOR_SYS_CALL
	call init_idt_descriptor
	add esp , 16


	mov esp , ebp
	pop ebp
	ret












;void init_idt_descriptor(dword vector , dword descriptor_type , dword handler , dword privilege);
;in fact we use desc_type -> low 8 byte; privilege -> low 8 byte

;%macro GATE 0
;	dw 0 ; offst_low

;	dw 0 ; selector
;	db 0 ; dcount only useful in call gate

;	db 0 ; attr
;	dw 0 ; offset_high


;%endmacro


init_idt_descriptor:
	push ebp
	mov ebp , esp

	xor eax , eax
	mov dword eax , [ebp + 8]	;vector -> al

	xor dl , dl
	mov byte dl , DESCRIPTOR_LEN
	mul dl
	;real address = vector * DESCRIPTOR_LEN -> eax

	xor edx , edx
	mov edx , eax
	;put in edx


	;[idt + edx] -> idt[vector] start location

	;---set offset---------
	xor eax , eax
	mov dword eax , [ebp + 16]	;handler -> eax
	mov word [idt + edx] , ax
	shr eax , 16
	mov word [idt + edx + 6] , ax

	;---set selector-------
	mov word [idt + edx + 2] , SELECTOR_KERNEL_CS

	;---set attr-----------
	xor eax , eax
	mov dword eax , [ebp + 12] ;descriptor_type -> eax only use ax

	xor ecx , ecx
	mov dword ecx , [ebp + 20] ;privilege -> ecx only use cx

	shl cl , 5  ;privilege in fact 2bit in bite 6 and 5
	or	al , cl	;combing attr and privilege

	mov byte [idt + edx + 5] , al




	mov esp , ebp
	pop ebp
	ret























