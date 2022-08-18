bits 16

section _TEXT class=CODE

	;; int 10h ah=0Eh
	;; args: character, page
	;; void _cdecl x86_div64_32(uint64_t dividend, uint32_t divisor, uint64_t *quotientOut, uint32_t *remainderOut);


	;;  unsigned 4 byte divide
	;;  input DX;AX	Dividend
global __U4D
__U4D:
	shl edx, 16 		; dx to upper half of edx
	mov dx, ax		; edx - dividend
	mov eax, edx		; eax - dividend
	xor edx, edx

	shl ecx, 16		; cx to upper half of ecx
	mov cx, bx		; ecx - divisor 

	div ecx 		; eax - quot, edx -remainder
	mov ebx, edx
	mov ecx, edx
	shr ecx, 16

	mov edx, eax
	shr edx, 16

	ret

global __U4M
__U4M:
	shl edx, 16 		;dx to upper half of edx
	mov dx, ax 		; m1 in edx
	mov eax, edx 		; m2 in edx

	shl ecx, 16 		; cx to upper half of ecx
	mov cx, bx		; m2 in ecx

	mul ecx			; result in edx, eax (we only need eax)
	mov edx, eax		;move upper half to dx
	shr edx, 16
	ret
	
global _x86_div64_32
_x86_div64_32:	
	;;  make new call frame
	push bp			; save old call frame
	mov bp, sp		; initialize new call frame

	push bx
	;;  divide upper 32 bits
	mov eax, [bp+8] 	; eax <= upper 32 bits of dividend
	mov ecx, [bp+12]	; ecx <= divisor
	xor edx, edx
	div ecx			; eax - quotient, edx - remainderx

	;;  store upper 32 bit of quotient
	mov bx, [bp + 16]
	mov [bx+4] , eax

	;;  divide lower 32-bits
	mov eax, [bp+4]   	; eax <- lower 32 bit of dividend
	                        ; edx <- old remainder 
	div ecx	 

	;;  store results
	mov [bx], eax
	mov bx, [bp+18]
	mov [bx], edx

	pop bx
	;;  restore old call frame
	mov sp, bp
	pop bp
	ret
	
global _x86_Video_Write_Character_Teletype
_x86_Video_Write_Character_Teletype:
	;;  make new call frame
	push bp			; save old call frame
	mov bp, sp		; initialize new call frame

	;; save bp

	;;  [bp + 0] - return address (small memory model => 2 bytes)
	;;  [bp + 2] - first argument (character)
	;;  [bp + 4] - second argument (page)
	;;  note: bytes are converted to words (you can't push a single byte on a stack 
	mov ah, 0Eh
	mov al, [bp+4]
	mov bh, [bp+6]

	int 10h

	;;  restore bh
	pop bx

	;;  restore old call frame

	mov sp, bp
	pop bp
	ret 

;void _cdecl x86_Disk_Reset(uint8_t drive);
global _x86_Disk_Reset
_x86_Disk_Reset:
	;;  make new call frame
	push bp			; save old call frame
	mov bp, sp		; initialize new call frame

	mov ah, 0
	mov dl, [bp+4] 		; dl - drive
	stc
	int 13h

	mov ax, 1
	sbb ax, 0		; 1 on success, 0 on failure
	
	;;  restore old call frame
	mov sp, bp
	pop bp
	ret 

;void _cdecl x86_Disk_Read(uint8_t drive,
		          ;uint16_t cylinder,
	                  ;uin16_t sector, 
			  ;uint16_t head,
			  ;uint8_t count,
			  ;uint8_t far *dataOut);

 global _x86_Disk_Read
_x86_Disk_Read:	
	;;  make new call frame
	push bp			; save old call frame
	mov bp, sp		; initialize new call frame

	;;  save modified regs
	push bx
	push es
	;; setup args
	mov dl, [bp+4] 		; dl - drive

	mov ch, [bp+6] 		; ch - cylinder (lower 8 bits)
	mov cl, [bp+7]		; cl - cylinder to bits 6-7
	shl cl, 6

	mov dh, [bp+10]		; dh - head

	mov al, [bp+8]		
	and al, 3Fh
	or cl, al		; cl sectors to bits 0 -5

	mov al, [bp+12]		; al - count

	mov bx, [bp+16]		; es:bx - far pointer to data out
	mov es, bx
	mov bx, [bp+14]

	;;  call int13h
	;;  reset disk 
	mov ah, 02h  
	stc
	int 13h

	mov ax, 1
	sbb ax, 0		; 1 on success, 0 on failure

	;; restore regs
	pop es
	pop bx
	;;  restore old call frame
	mov sp, bp
	pop bp
	ret 

	;; bool _cdecl x86_Disk_Write(uint8_t drive, 
	;; 		  uint16_t cylinder,
	;; 		  uint16_t sector,			  
	;; 		  uint16_t head,
	;; 		  uint8_t sectorCount,
	;; 		  void far *dataIn)

 global _x86_Disk_Write
_x86_Disk_Write:	
	;;  make new call frame
	push bp			; save old call frame
	mov bp, sp		; initialize new call frame

	;;  save modified regs
	push bx
	push es
	;; setup args
	mov dl, [bp+4] 		; dl - drive

	mov ch, [bp+6] 		; ch - cylinder (lower 8 bits)
	mov cl, [bp+7]		; cl - cylinder to bits 6-7
	shl cl, 6

	mov dh, [bp+10]		; dh - head

	mov al, [bp+8]		
	and al, 3Fh
	or cl, al		; cl sectors to bits 0 -5

	mov al, [bp+12]		; al - count

	mov bx, [bp+16]		; es:bx - far pointer to data out
	mov es, bx
	mov bx, [bp+14]

	;;  call int13h
	;;  reset disk 
	mov ah, 03h  
	stc
	int 13h

	mov ax, 1
	sbb ax, 0		; 1 on success, 0 on failure

	;; restore regs
	pop es
	pop bx
	;;  restore old call frame
	mov sp, bp
	pop bp
	ret 	
	;; get number of cylinder, sectorsxs
;void _cdecl x86_Disk_GetDriveParams(uint8_t drive, 
				    ;uint8_t *driveTypeOut,
				    ;uint16_t *cylindersOut,
				    ;uint16_t *sectorsOut,
				    ;uint16_t *headsOut);

			  
global _x86_Disk_GetDriveParams
_x86_Disk_GetDriveParams:
	;;  make new call frame
	push bp			; save old call frame
	mov bp, sp		; initialize new call frame

	;;  save regs
	push es
	push bx
	push si
	push di

	;;  call int13h
	mov dl, [bp+4]		; dl - disk drive
	mov ah, 08h
	mov di, 0		; es:di - 0000:0000
	mov es, di
	stc
	int 13h

	;; return
	mov ax, 1
	sbb ax, 0

	;; out params
	mov si, [bp+6]		; drive type from bl
	mov [si], bl

	mov bl, ch		;cylinders - lower bits in ch
	mov bh, cl		;cylinders - upper bits in cl (6-7)
	shr bh, 6
	mov si, [bp+8] 		; cylindersOut
	mov [si], bx


	xor ch, ch 		;sectors - lower 5 bits in cl
	and cl, 3Fh
	mov si, [bp+10]
	mov [si], cx

	mov cl, dh		; heads -dh
	mov si, [bp+12]
	mov [si], cx
	
	;; restore regs
	pop di
	pop si
	pop es
	pop es
	
	;;  restore old call frame
	mov sp, bp
	pop bp
	ret 
