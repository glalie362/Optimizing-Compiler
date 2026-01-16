; func main_function
; v0 : int = alloc
; v1 : int = const 0
; store v0, v1
; v2 : int = alloc
; v3 : int = const 0
; store v2, v3
; L0:
; v5 : int = const 10
; v4 : int = lt v2, v5
; b v4, L1, L2
; L1:
; v6 : int = alloc
; v7 : int = const 0
; store v6, v7
; L3:
; v9 : int = const 10
; v8 : int = lt v6, v9
; b v8, L4, L5
; L4:
; v10 : int = add v0, v6
; store v0, v10
; v12 : int = const 1
; v11 : int = add v6, v12
; store v6, v11
; j L3
; L5:
; v14 : int = const 1
; v13 : int = add v2, v14
; store v2, v13
; j L0
; L2:
; ret v0
;--------------
bits 64
section .text
global main_function
main_function:
	push rbp
	mov rbp, rsp
	push rbx
	push r12
	sub rsp, 24
	xor rcx, rcx
	xor rdx, rdx
.L0:
	cmp rdx, 10
	jge .L2
	xor r9, r9
.L3:
	cmp r9, 10
	jge .L5
	add rcx, r9
	add r9, 1
	jmp .L3
.L5:
	add rdx, 1
	jmp .L0
.L2:
	mov rax, rcx
	jmp .epi
.epi:
	pop r12
	pop rbx
	pop rbp
	add rsp, 24
	ret
