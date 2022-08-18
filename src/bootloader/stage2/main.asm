bits 16

section _ENTRY class=CODE
%define ENDL 0x0D, 0x0A

extern _cstart_
global entry

entry:
	cli
	;; setup stack
	mov ax, ds
	mov ss, ax
	mov sp, 0
	mov bp, sp
	sti
	
	;;  expect boot drive in dl, send it as argument to cstart function
	xor dh, dh
	push dx
	call _cstart_

	cli
	hlt

puts:
	;;  save register we will modify
	push si
	push ax
.loop:
	;; loads next char in al
	lodsb      
	;;  verify next char is null?	
	or al, al
	jz .done1

	mov ah, 0x0e 		; calls bios interrupt
	int 0x10
	jmp .loop
.done1:
	pop ax
	pop  si
	ret 
