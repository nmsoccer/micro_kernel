;common functions
global disphexdw , disphexw , disphexb

;dword disphex(dword data ,  dword address); only al used
;dword disphexw(dword data , dword address); only eax used
;dword disphexdw(dword data , dword address);all  eax  used




  ;-------------DISPHEXDW----------
;dword disphexdw(dword data , dword address); eax used
		;data-eax address-edi
disphexdw:
	push ebp
	mov ebp  ,  esp
	push edx

	xor edx , edx
	mov dword edx , [ebp + 8]

	xor edi , edi
	mov dword edi , [ebp + 12]

	mov eax , edx
	shr eax , 16
	push edi
	push eax
	call disphexw
	add esp , 8
	mov edi , eax

	sub edi , 2
	mov eax , edx
	push edi
	push eax
	call disphexw
	add esp , 8

	;eax ->(from disphexw) location after 'h'



	pop edx
	mov esp , ebp
	pop ebp
	ret
		

;-------------DISPINTW----------------
;disphexw(dword data , dword address); only ax used
		;change edx
		;data-ax address-edi
		;return eax->next location of 'h'
disphexw:
	push ebp
	mov ebp , esp
	push edx

	xor edx , edx
	mov dword edx , [ebp + 8]

	xor edi , edi
	mov edi , [ebp + 12]

	mov eax , edx
	shr ax , 8
	push edi
	push eax
	call disphexb
	add esp , 8
	mov edi , eax

	sub edi , 2
	mov eax , edx
	push edi
	push eax
	call disphexb
	add esp , 8
	
	;eax -> (from disphex)->next location after 'h' 
	
	pop edx
	mov esp , ebp
	pop ebp
	ret


;-------------DISPHEX-----------------
;disphex(dword data , dword address); only al used 
		;realization:
		;data-al  address-edi
		;change edx
		;warning:last number may be changed!so edi should be added 2
		;return eax ->the next location of  'h'
disphexb:
	push ebp
	mov ebp , esp
	push edx

	xor eax , eax
	mov dword eax , [ebp + 8]

	xor edi , edi
	mov dword edi , [ebp + 12]


	mov dl , al
	
;	xor ax , ax
;	mov ax , selector_video
;	mov gs , ax	

	mov al ,dl
	shr al , 4
	and al , 00001111b
	cmp al , 9
	ja .1
	add al , '0'
	jmp .2
.1:	
	sub al , 0x0A
	add al , 'A'
.2:
	mov ah , 00000100b
	mov [gs:edi] , ax
	add edi , 2
		
	mov al , dl
	and al , 00001111b
	cmp al , 9
	ja .3
	add al , '0'
	jmp .4
.3:	
	sub al , 0x0A
	add al , 'A'
.4:
	mov ah , 00000100b
	mov [gs:edi] , ax
	add edi , 2
	mov ah , 00000101b
	mov al , 'h'
	mov [gs:edi] , ax

	add edi , 2
	xor eax , eax
	mov eax , edi
	
	pop edx
	mov esp , ebp
	pop ebp
	ret
