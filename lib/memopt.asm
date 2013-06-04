global memcpy , memset


;-----memcpy----------
;memcpy(p_dest , p_src , int number)

;source-esi(ds) number-ecx destination-edi(es)
;change es
memcpy:
	push ebp
	mov ebp , esp

	push es

	xor ax , ax
	mov ax , ds
	mov es , ax
	xor ax , ax

	xor ecx , ecx	
	mov dword ecx , [ebp + 16]

	xor esi , esi
	mov dword esi , [ebp + 12]

	xor edi , edi
	mov dword edi , [ebp + 8]


.1:
	lodsb
	stosb	
	loop .1
	
	pop es	

	mov esp , ebp
	pop ebp
	ret


;------memset---------
;void memset(char * dest , int data , int number , int opt);
;set area from (dest) to (dest + number)(B/W/D) as data

;opt=0 set per byte; opt=1 set per word; opt=2 set per dword
memset:
	push ebp
	mov ebp , esp


	xor ecx , ecx
	mov dword ecx , [ebp + 16];number -> ecx

	xor eax , eax
	mov dword eax , [ebp + 12];data -> eax

	xor edi , edi
	mov dword edi , [ebp + 8];dest -> edi [es:edi]

	xor edx , edx
	mov dword edx , [ebp + 20];opt -> edx

	cmp  dl , 0
	jz .setb
	cmp dl , 1
	jz .setw
	cmp dl , 2
	jz .setd

	jmp .end

.setb:
	stosb	;use al
	loop .setb
	jmp .end

.setw:
	stosw	;use ax
	loop .setw
	jmp .end

.setd
	stosd	;use eax
	loop .setd

.end:

	mov esp , ebp
	pop ebp
