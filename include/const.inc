%define GDT_SIZE  128
%define IDT_SIZE	  256
%define LDT_SIZE 2	

%define NUM_IRQ 16

;---------set 8259A-----------
%define INT_M_CTL 0x20
	; I/O port for interrupt controller <MASTER> 
	; Write into ICW1/OCW2
%define INT_M_CTLMASK 0x21 
	; setting bits in this port disabls ints <MASTER> 
	; Writ into ICW2/ICW3/ICW4/OCW1

%define INT_S_CTL 0xA0
	; I/O port for second interrupt controller<SLAVE>
	; Write into ICW1/OCW2
%define INT_S_CTLMASK 0xA1
	; setting bits in this port disables ints<SLAVE>
	; Writ into ICW2/ICW3/ICW4/OCW1

;----------INT VECTOR-----------
%define INT_VECTOR_IRQ0 0x20
%define INT_VECTOR_IRQ8 0x28

;----------INT LABEL---
%define EOI 0x20;EOI is in fact a OCW2

;---------TSS----------------

TSS_SP0	equ	4

;---------Clock Control(8253)------
%define TIMER0	0X40 ;I/O port for IRQ0
%define TIMER_MODE	0x43 ;I/O port for timer mode control
%define RATE_GENERATOR	0x34 
;00 - 11 - 010 - 0 bit0->二进制 bit1~bit3->RATE模式 bit4~bit5->读写锁 bit6~bit7->使用TIMER0

%define TIMER_FREQ 1193182
%define HZ	100 ;10ms










