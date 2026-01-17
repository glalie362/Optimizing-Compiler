; func main
; v0 : int = alloc
; v1 : int = const 0
; store v0, v1
; L1:
; v3 : int = const 5
; v2 : int = lt v0, v3
; b v2, L2, L3
; L2:
; v5 : int = const 1
; v4 : int = add v0, v5
; store v0, v4
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
.L1:
	cmp rcx, 5
	jge .L3
	add rcx, 1
	jmp .L1
.L3:
	mov rax, rcx
	ret
