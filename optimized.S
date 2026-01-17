; func main
; v0 : int = alloc
; v1 : int = const 0
; store v0, v1
; v2 : int = alloc
; v3 : int = const 0
; store v2, v3
; L1:
; v5 : int = const 5
; v4 : int = lt v2, v5
; b v4, L2, L3
; L2:
; v6 : int = alloc
; v7 : int = const 0
; store v6, v7
; L4:
; v9 : int = const 5
; v8 : int = lt v6, v9
; b v8, L5, L6
; L5:
; v11 : int = const 1
; v10 : int = add v0, v11
; store v0, v10
; v13 : int = const 1
; v12 : int = add v6, v13
; store v6, v12
; j L4
; L6:
; v15 : int = const 1
; v14 : int = add v2, v15
; store v2, v14
; j L1
; L3:
; ret v0
; L0:
;--------------
bits 64
section .text
global main
main:
	xor rcx, rcx
	xor rdx, rdx
.L1:
	cmp rdx, 5
	jge .L3
	xor r9, r9
.L4:
	cmp r9, 5
	jge .L6
	add rcx, 1
	add r9, 1
	jmp .L4
.L6:
	add rdx, 1
	jmp .L1
.L3:
	mov rax, rcx
	ret
