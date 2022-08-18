	org 0x0
	bits 16

%define ENDL 0x0D, 0x0A
	
start:
	 mov si, msg_hello	
	 call puts	

.halt:
	cli
	hlt

	;;  print a string to the screen
	;;  param
	;; ds:si points to string

puts:
	;;  save register we will modify

	push si
	push ax
	push bx
.loop:
	;; loads next char in al
	lodsb      
	;;  verify next char is null?	
	or al, al
	jz .done

	mov ah, 0x0e 		; calls bios interrupt
	mov bh, 0
	int 0x10
	jmp .loop
.done:
	pop bx
	pop ax
	pop si
	ret 


msg_hello: db  'Hello world from Kernel!', ENDL, 0

	
