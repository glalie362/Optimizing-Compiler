bits 64
section .text
global main
main:
	push rbx
	push r12
	mov rcx, 0
	mov rdx, 0
.L1:
	mov rax, rdx
	cmp rax, 5
	setl al
	movzx rax, al
	mov r8, rax
	mov rax, r8
	test rax, rax
	jnz .L2
	jz .L3
.L2:
	mov r9, 0
.L4:
	mov rax, r9
	cmp rax, 5
	setl al
	movzx rax, al
	mov r10, rax
	mov rax, r10
	test rax, rax
	jnz .L5
	jz .L6
.L5:
	mov rax, rcx
	add rax, 1
	mov r11, rax
	mov rcx, r11
	mov rax, r9
	add rax, 1
	mov rbx, rax
	mov r9, rbx
	jmp .L4
.L6:
	mov rax, rdx
	add rax, 1
	mov r12, rax
	mov rdx, r12
	jmp .L1
.L3:
	mov rax, rcx
	jmp .L0
.L0:
	pop r12
	pop rbx
	ret
