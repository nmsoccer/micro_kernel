global clock_handler

extern dispstr , delay , disphexb , disphexdw, schedule

;from C
extern proc_table

%include "global.inc"

clock_int_msg:
	db "^" , 0x00
reenter_msg:
	db "!" , 0x00
;-------CLOCK HANDLE--------
;void clock_handler(int irq);
;Such function deals the inner logic in INT of clock

clock_handler:
;	mov dword [proc_table] , 1
;	ret

	push ebp
	mov ebp , esp

	inc dword [ticks]

	cmp dword [k_reenter] , 1
	ja .end; if k_reenter >1 do not use schedule

;--------schedule--------

	call schedule


;------------------

;	M_dispstr reenter_msg , [videoaddr]
;	mov [videoaddr] , eax
.end:

	mov esp , ebp
	pop ebp
	ret




;------process schedule--------

schedule0:

	push ebx

	xor eax , eax
	xor ecx , ecx
	xor edx , edx

	M_multib [proc_table] , 4
	mov ebx , [proc_table + eax]

	cmp dword [ebx + PCB_SLICES] , 0
	je .1 ;if slices=0 jmp .1
	dec dword [ebx + PCB_SLICES]
	cmp dword [ebx + PCB_SLICES] , 0
	jne .1 ;if slices != 0 jmp .1 else priority = 0
	mov dword [ebx + PCB_PRIORITY] , 0

;---------mainly logic-------
.1:
;	M_disphexb [ebx + PCB_PRIORITY] , [videoaddr]
;	mov [videoaddr] , eax
	mov edx , [proc_table] ; store current index
	mov ecx , [proc_table] ; change index
	inc ecx



.2:
	cmp dword ecx , NUM_TASK
	jbe .3 ;if ecx <= NUM_TASK jmp .3
	 ;else ecx = 1
	mov ecx , 1
.3:
	cmp ecx , edx
	je	.out ;if ecx = edx while ends
	push edx ;M_multib will change edx
	push ecx


	M_multib [proc_table] , 4
	mov ebx , [proc_table + eax] ;ebx = index of process (ready to run)

	M_multib cl , 4
	mov ecx , [proc_table + eax] ;ecx = index of process that compares to ebx


	xor eax , eax
	mov eax , [ebx + PCB_PRIORITY]
	cmp eax , [ecx + PCB_PRIORITY]
	ja .4		; if ebx -> process.priority > ecx -> process.priority jmp .4 (while)

	pop ecx
	mov [proc_table] , ecx ;else set ecx -> porcess (ready to run)
	jmp .5
.4:
	pop ecx
.5:
	pop edx

	inc ecx
	jmp .2

;.x:


;	add dword [proc_table] , 1
;	cmp dword [proc_table] , NUM_TASK
;	jbe .out
;	mov dword [proc_table] , 1
;	jmp .out

.out:

;	M_disphexd [proc_table] , (80 * 2 + 0) * 2

	pop ebx
	ret


