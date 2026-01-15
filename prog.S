; func main
; v0 : int = alloc
; v1 : int = const 1
; store v0, v1
; v3 : int = const 10
; v2 : int = lt v0, v3
; b v2, L0, L1
; L0:
; v4 : int = const 1
; store v0, v4
; L1:
; L2:
; ret v0
;--------------
bits 64
section .text
global main
main:
	push rbp
	mov rbp, rsp
	mov rax, 1
	cmp rax, 10
	jl .L0
	jge .L1
.L0:
	mov rcx, 1
.L1:
	mov rax, rcx
	pop rbp
	ret
