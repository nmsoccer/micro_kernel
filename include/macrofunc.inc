;Here lists some functions which are written by MACRO INSTRUCTIONS
;even package some simple functions by MACRO INSTRUCTIONS
;关于宏函数的一些用法，与传统用法的区别在于形式更加简单，但是需要一次传入所有参数，而传统的可以分别传入，所以都有用处。这里做一个规定
;一般推荐用函数的宏形式，当参数很多或者一次性传参不方便时才考虑函数的原始形式。
;main函数及以后的程序

%macro M_base_selec 2 ; selector(seg_res) , table(gdt/ldt) ; return base adddress of seg -> eax
	push dword %2
	push dword %1
	call base_selec
	add esp , 8
%endmacro


%macro M_delay 1 		;delay time

	push dword %1
	call delay
	add esp , 4
	
%endmacro



%macro M_disable_irq 1 ; irq

	push dword %1
	call disable_irq
	add esp , 4
	
%endmacro

%macro M_dispstr 2	;add of str , [videoaddr]
;	extern dispstr ;extern once ; extern same labe more than once , complier will ignore it automatically 
	
	push dword %2
	push dword %1
	call dispstr
	add esp , 8
	
	;return eax -> next loaction of displaying
	;no responsible for return value
%endmacro	


%macro M_disphexd 2	;number , [videoaddr]

	push dword %2
	push dword %1
	call disphexdw
	add esp , 8
	
	;return eax -> next location of displaying
%endmacro

%macro M_disphexb 2 ;number , [videoaddr]
	
	push dword %2
	push dword %1
	call disphexb
	add esp , 8
	
%endmacro	

%macro M_enable_irq 1 ; irq

	push dword %1
	call enable_irq
	add esp , 4
	
%endmacro

%macro M_init_desc 5 ;selector , base of desc , limit of desc , attr of desc , table located in(gdt/ldt)

	push dword %5
	push dword %4
	push dword %3
	push dword %2
	push dword %1
	call init_desc
	add esp , 20
%endmacro


%macro M_m2md 2 	; dest , src ; %1(mem) = %2(mem) by dword

	xor eax , eax
	mov dword eax , %2
	mov dword %1 , eax
	
%endmacro


%macro M_memcpy 3 ; dest , src , number

	push dword %3
	push dword %2
	push dword %1
	call memcpy
	add esp , 12
	
%endmacro

%macro M_memset 4 ; dest , data , number , opt (opt =0 -> byte ; 1 -> word ; 2 -> dword)

	push dword %4
	push dword %3
	push dword %2
	push dword %1
	call memset
	add esp , 16

%endmacro

%macro M_multib 2	; ax = %1 * %2 ; byte * byte ;change eax , edx

	xor eax , eax
	mov byte al , %1
	mov byte dl , %2
	mul dl
	
	;result -> ax

%endmacro

%macro M_multiw 2 ; eax = %1 * %2 ; word * word ;change eax , edx
	
	xor eax , eax
	mov ax , %1
	
	xor edx , edx
	mov dx , %2
	mul dx
	;high 16bits in dx , low 16bits in ax
	
	shl edx , 16
	or  eax , edx
	;result ->  eax
%endmacro

%macro M_out_b 2 ; ID of I/O port , byte

	xor al , al
	mov al , %2
	out %1 , al

%endmacro

%macro M_out_w 2 ; ID of I/O port , word

	xor ax , ax
	mov ax , %2
	out %1 , ax

%endmacro

%macro M_set_irq_table 2 ; IRQ , handler

	push dword %2
	push dword %1
	call set_irq_table
	add esp , 8

%endmacro




%macro M_vir2phy 2	; base_seg , label ; get phy address of label ; return add -> eax
	
	xor eax , eax
	mov  dword eax , %1
	add  eax , %2
	
%endmacro












	