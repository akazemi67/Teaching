
; The Linux/x86-64 kernel expects the system call parameters in
;    registers according to the following table:
;
;     syscall number  rax
;     arg 1       rdi
;     arg 2       rsi
;     arg 3       rdx
;     arg 4       r10
;     arg 5       r8
;     arg 6       r9

global	_start
section	.text

_start:

	; ssize_t write(int fd, const void *buf, size_t count)
	mov	rax,1	 		        ; write(2)
	mov	rdi,1			        ; fd
	mov	rsi, msg		        ; buffer
	mov	rdx, msg_size 	        ; count
	syscall

    ; Now calling our syscall
    mov rax, 488
    mov rdi, msg
    syscall

	; exit(result)
	mov	rdi,0			; result
	mov	rax,60			; exit(2)
	syscall

msg:	db "Hello World!",10
msg_size EQU $ - msg

