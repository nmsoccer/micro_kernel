


global dispstr


;int dispstr(char *str , int address);

;[ebp + 8] = &str [ebp + 12] = address
;return eax
;gs->video memory address
dispstr:
	push ebp
	mov ebp , esp

	xor esi , esi
	mov esi , [ebp + 8];esi->str

	xor edi , edi
	mov edi , [ebp + 12];edi -> videoaddr
	
	xor edx , edx



.1:
	lodsb
	cmp al , 0x0D
	je  .2
	cmp al , 0x00
	je  .3
	mov ah , 00000111b
	mov [gs:edi] , ax
	add edi , 2
	jmp .1
.2:			;Wrap	
	xor ax , ax
	mov eax , edi
	mov dl , 160
	div dl   	;ah: column , al: row

	add al , 1	;(row+1) *160	
	mul dl
	movzx edi , ax 
	jmp .1
.3:

	mov eax , edi;eax -> next video



	mov esp , ebp
	pop ebp
	ret



