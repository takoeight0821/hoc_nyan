.intel_syntax noprefix
.text
.global fib
fib:
	push rbp
	mov rbp, rsp
	sub rsp, 4
	mov [rbp-4], edi
	mov eax, [rbp-4]
	push rax
	push 0
	pop rdi
	pop rax
	cmp rax, rdi
	sete al
	movzb eax, al
	push rax
	pop rax
	cmp rax, 0
	je .Lelse1
	push 1
	pop rax
	jmp .Lend0
	pop rax
	jmp .Lend2
.Lelse1:
	mov eax, [rbp-4]
	push rax
	push 1
	pop rdi
	pop rax
	cmp rax, rdi
	sete al
	movzb eax, al
	push rax
	pop rax
	cmp rax, 0
	je .Lelse3
	push 1
	pop rax
	jmp .Lend0
	pop rax
	jmp .Lend4
.Lelse3:
	mov eax, [rbp-4]
	push rax
	push 1
	pop rdi
	pop rax
	sub eax, edi
	push rax
	pop rdi
	push r10
	push r11
	mov rax, 0
	sub rsp, 8
	call fib
	pop r11
	pop r10
	add rsp, 8
	push rax
	mov eax, [rbp-4]
	push rax
	push 2
	pop rdi
	pop rax
	sub eax, edi
	push rax
	pop rdi
	push r10
	push r11
	mov rax, 0
	sub rsp, 8
	call fib
	pop r11
	pop r10
	add rsp, 8
	push rax
	pop rdi
	pop rax
	add eax, edi
	push rax
	pop rax
	jmp .Lend0
	pop rax
.Lend4:
.Lend2:
	pop rax
.Lend0:
	leave
	ret
.text
.global main
main:
	push rbp
	mov rbp, rsp
	sub rsp, 0
	push 3
	pop rdi
	push r10
	push r11
	mov rax, 0
	sub rsp, 8
	call fib
	pop r11
	pop r10
	add rsp, 8
	push rax
	pop rax
	jmp .Lend5
	pop rax
.Lend5:
	leave
	ret
