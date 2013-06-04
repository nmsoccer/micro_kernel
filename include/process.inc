;This headerfile contains the struct of process control block (PCB)

%include "const.inc"
%include "protect.inc"

;----will not use any more--
%define NUM_TASK  5	;------
;------------------------------------

%define PCB_LEN (19 * 4 + DESCRIPTOR_LEN * LDT_SIZE + 40) ; 110B when LDT_SIZE=2


;------PCB STRUCTURE--------
%ifdef USE_STC_PCB

STRUC	stc_pcb

.stackframe:
	
;01low -> high: gs , fs , es , ds , edi , esi , ebp -> They are pushed by save()
;02low -> high: kernel_esp ->popad will ignore it
;03low -> high: ebx , edx , ecx , eax -> as 01 
;04low -> high: retaddr -> return addr for kernel.asm::save()
;05low -> high: eip , cs , eflags , esp , ss -> pushed by CPU during interrupt

.gs:
	resd 1
.fs:
	resd 1
.es:
	resd 1
.ds:
	resd 1
.edi:
	resd 1
.esi:
	resd 1
.ebp:
	resd 1	
.kernel_esp:
	resd 1
.ebx:
	resd 1	
.edx:
	resd 1
.ecx:
	resd 1
.eax:
	resd 1
.retaddr:
	resd 1
.eip:
	resd 1
.cs:
	resd 1
.eflags:
	resd 1
.esp:
	resd 1
.ss:
	resd 1					


.sel_ldt:
	resd 1
;selector for ldt

.ldt:
	resb DESCRIPTOR_LEN * LDT_SIZE
;descriptors in ldt

;进程需要的时间片
.slices: 
	resd 1
	
;进程的优先级
.priority: 

	resd 1
	
.pid:
	resd 1
;process id

.parent_pid:
	resd 1

.p_tty:
	resd 1
	
.p_flag:
	resd 1		

.p_name:
	resb 16
;process name

ENDSTRUC

%endif



;----------PCB-----------

PCB_STACKBASE	equ	0
PCB_GS	equ	PCB_STACKBASE
PCB_FS	equ	PCB_GS		+ 4
PCB_ES		equ	PCB_FS	+ 4
PCB_DS		equ	PCB_ES	+ 4
PCB_EDI		equ	PCB_DS	+ 4
PCB_ESI		equ	PCB_EDI	+ 4
PCB_EBP		equ	PCB_ESI	+ 4
KERNELESP	equ	PCB_EBP	+ 4
PCB_EBX		equ	KERNELESP	+ 4
PCB_EDX		equ	PCB_EBX	+ 4
PCB_ECX		equ	PCB_EDX	+ 4
PCB_EAX		equ	PCB_ECX	+ 4
RETADDR		equ	PCB_EAX	+ 4
PCB_EIP		equ	RETADDR	+ 4
PCB_CS		equ	PCB_EIP	+ 4
PCB_EFLAGS	equ	PCB_CS	+ 4
PCB_ESP		equ	PCB_EFLAGS	+ 4
PCB_SS		equ	PCB_ESP	+ 4
PCB_STACKTOP	equ	PCB_SS	+ 4

PCB_SEL_LDT	equ	PCB_STACKTOP
PCB_LDT		equ	PCB_SEL_LDT	+ 4

PCB_SLICES	equ 	PCB_LDT + DESCRIPTOR_LEN * LDT_SIZE
PCB_PRIORITY	equ	PCB_SLICES + 4

PCB_PID		equ PCB_PRIORITY + 4
PCB_PARENT_PID	equ PCB_PID + 4

PCB_P_TTY	equ PCB_PARENT_PID + 4
PCB_P_FLAG	equ PCB_P_TTY + 4
