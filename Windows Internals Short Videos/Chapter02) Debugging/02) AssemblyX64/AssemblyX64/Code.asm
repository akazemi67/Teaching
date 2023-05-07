.code

copy_str PROC PUBLIC src:QWORD, dst:QWORD

; This part counts the length of src, but it has a bug.
; Find and fix the bug for practice!:-)
	mov r8, rcx
	xor bl, bl
count_loop:
	inc bl
	inc r8
	mov al, byte ptr[r8]
	test al, al
	jnz count_loop

; This part is for Copying
	mov rsi, rcx
	mov rdi, rdx
	xor rcx, rcx
	mov cl, bl
	inc cl ; WHY?
	cld
	rep movsb

; Setting Return Value
	xor rax, rax
	mov al, bl
	ret
copy_str ENDP

end
