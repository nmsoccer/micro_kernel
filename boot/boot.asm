	org 0x7C00
;-------------------------
top_stack	equ 0x7C00
	
;-------------------------
;-----------FAT Head------
	jmp short LABEL_START
	nop

%include"fat12hdr.inc"
;-------------------------
LABEL_START:
	mov ax , cs
	mov ds , ax
	mov ss , ax
	mov sp , top_stack

	xor ax , ax
	mov ax , base_loader
	mov es , ax

	call CLRSCREEN

;	xor edx , edx
;	xor esi , esi
;	mov esi , root_message
;	mov edx , (80 * 0 + 0) * 2
;	call DISPSTR
;	nop
	
	xor ah , ah
	xor dl , dl
	int 0x13

	call GETROOTSEC
	
;	xor al , al
;	mov byte al , [item_message + 16]
;	add al , [rootsec]
;	mov [item_message + 16] , al
;	xor esi , esi
;	mov esi , item_message
;	mov edx , (80 * 1 + 0) * 2
;	call DISPSTR


	call SEARCHFILE
	
	mov edx , (80 * 0 + 0) * 2
	
	cmp word [loaderentry] , 0x0000
	jz ending

	mov esi , load_message
	call DISPSTR

	call LOADFILE

	jmp base_loader:offset_loader
	
ending:
	mov esi , nofile_message
	call DISPSTR
	jmp $

;-----------------------
;change-eax,ebx,ecx,edx
CLRSCREEN:

	mov ah , 0x06;Function
	mov al , 0x00;Number scroll
	mov bh , 00000111b;attr
	mov cx , 0x0000;start row:column
	mov dx , 0x184F;end row:column
	int 0x10

	ret
;-----------------------
;str--esi addr--edx
;change-gs,eax,ebx	
DISPSTR:

	mov ax , 0xB800
	mov gs , ax

	mov ah , 00001100b

.1:
	lodsb
	cmp al , 0x00
	jz  .2
	mov [gs:edx] , ax
	add edx , 2
	jmp .1	
	
.2:     ;Set cursor
;	mov ax , dx
;	mov bl , 160
;	div bl      ;row-al column-ah
;	xor dx , dx
;	mov dh , al
;	add dh , 1
;	mov dh , 0x01
;	mov dl , 0x00
;	mov ah , 0x02
;	mov bh , 0x00
;	int 0x10

	ret
;---------------vars------------
rootitem 	db 0
loaderentry	dw 0

load_message	db "Booting.." , 0x00
nofile_message	db "No loader!"   , 0x00

file_name:	db "LOADER  BIN"
;-------------------------------
LOADFILE:

	mov ax , base_fat
	mov fs , ax
	mov es , ax
	mov al , 0x01;start sector
	mov cl , 0x01;number of sector
	call READSECTOR


	mov ax , base_loader
	mov es , ax

START_LOADING:
	mov di , offset_loader;in circle di-->0x100

	mov dx , 0x0011;17
	add dl , rootsec;14
	add dx , [loaderentry];first loadentry=5 second=6 third 7
	mov ax , dx
	mov cl , 0x02;once read two sectors	
	call READSECTOR;Load entry->sector

;get location
	mov word ax , [loaderentry];using only low 8 bit
	

	mov bl , 0x03
	mul bl
	mov bl , 0x02
	div bl;al-shang ah-yushu
;al is the number in FAT

;get next entry
	cmp ah , 0x00

	jnz NOT_EVEN_ENTRY

	add di , ax
;	xor cx , cx
	mov cx , [fs:di]
	and cx , 0x0FFF
	mov [loaderentry] , cx

	jmp CONTROL

NOT_EVEN_ENTRY:
	xor ah , ah
	add di , ax


	mov cx , [fs:di]
	shr cx , 4


	mov word [loaderentry] , cx

CONTROL:

	cmp word [loaderentry] , 0x0FF8
	jae END_LOADING



	mov ax , es
	add ax , 0x20;0x9000 in fact is 0x90000 !!!!!
		     ;so 0x90200 write as 0x9020	
	mov es , ax;set memory addr

	jmp START_LOADING

	
	
END_LOADING:

	ret
;	jmp base_loader:offset_loader	


;-------------------------------
;change-eax,ecx,esi,edi,[rootitem]
;return-[loaderentry]
;very well
SEARCHFILE:
;	mov dl , [rootitem] 

start_search_file:
	mov al , start_rootsec;19
	mov cl , rootsec
	call READSECTOR

	mov si , file_name
	mov di , offset_loader
;----Checking item-----
search_file:		 
	mov ecx , 11

compare:
	lodsb
	cmp al , [es:di]
	jne not_match
	inc di
	loop compare
		
	jmp match


not_match:

	sub byte [rootitem] , 1
	cmp byte [rootitem] , 0
	jz  end_search_file	
	 
	and di , 0xFFE0	
	add di , 32
	mov si , file_name
	jmp search_file

match:
	and di , 0xFFE0;return to the head of item
	add di , 0x001A
	mov ax , [es:di]
	mov [loaderentry] , ax

;$
;	sub al , 1
;	add byte [load_message] , al
;	jmp end_search_file
;------Checking item end-----	
end_search_file:
;	mov  [rootitem] , dl;[rootitem]->header of target in table
;$
;	mov dl , [rootitem]
	

	ret
;-------------------------------
;change-eax,ebx,ecx,edx,esi,edi
;Get number of items of root and put it in [rootitem]
;check well!
GETROOTSEC:
	mov si , start_rootsec
;	start_rootsec == 19
	mov di , 1

	mov dx , 0;record number of item	
	mov bx , offset_loader

start_search_item:
	mov ax , si
	mov cx , di

	call READSECTOR;in READSECTOR bx=offset_loader
	
	mov ecx , 0x00000010;max items in a sector

search_item:
	mov ah , [es:bx]
	cmp ah , 0x00
	je  end_search_item
	add bx , 32
	inc dl
	loop search_item

	inc si
	mov di , 1
	mov bx , offset_loader
	jmp start_search_item	
	
end_search_item:
;	cmp dl , 0



	mov [rootitem] , dl



;	jz  no_item

;	mov ax , dx
;	mov bl , 0x10
;	div bl
;	cmp ah , 0x00
;	jz  full_sector
;	inc al

;full_sector:
;	mov [rootsec] , al  

;no_item:

	ret

;-------------------------------
;al--start sector cl--number of sectors
;destination--es:bx
;change--ebx,edx
READSECTOR:
;	push ebx
;	push edx

	and ax , 0x00FF

	mov bl , [BPB_SecPerTrk]
;	[BPB_SecPerTrk] == 18
	div bl

	;al -- Quotient
	;ch -- Cylinder
	mov ch , al
	shr ch , 1
	;dh -- Head
	mov dh , al                        
	and dh , 1
	;al -- number
	mov al , cl
	;cl -- start
	mov cl , ah
	add cl , 1
	;ah -- function dl --A
	mov ah , 0x02
	mov dl , [BS_DrvNum]
;	[BS_DrvNum] == 0

	;bx -- offset_loader
	mov bx , offset_loader

	int 0x13	


;	pop  edx
;	pop  ebx
	ret
;-----------------------------------
	times	510-($-$$) db 0
	dw 0xaa55
