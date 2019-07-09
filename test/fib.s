.intel_syntax noprefix
# start Function
.text
.global fib
fib:
	push rbp
	mov rbp, rsp
	sub rsp, 4
# start Var n
	mov rax, rbp
	sub rax, 4
	push rax
# end Var n
	mov [rax], edi
# start NBLOCK
# start NIFELSE
# start NEQ
# start NVAR
# start Var n
	mov rax, rbp
	sub rax, 4
	push rax
# end Var n
	pop rax
	mov eax, [rax]
	push rax
# end NVAR
# start NINT
	push 0
# end NINT
	pop rdi
	pop rax
	cmp eax, edi
	sete al
	movzb rax, al
	push rax
# end NEQ
	pop rax
	cmp rax, 0
	je .Lelse1
# start NBLOCK
# start NRETURN
# start NINT
	push 1
# end NINT
	pop rax
	jmp .Lend0
# end NRETURN
# end NBLOCK
	jmp .Lend2
.Lelse1:
# start NIFELSE
# start NEQ
# start NVAR
# start Var n
	mov rax, rbp
	sub rax, 4
	push rax
# end Var n
	pop rax
	mov eax, [rax]
	push rax
# end NVAR
# start NINT
	push 1
# end NINT
	pop rdi
	pop rax
	cmp eax, edi
	sete al
	movzb rax, al
	push rax
# end NEQ
	pop rax
	cmp rax, 0
	je .Lelse3
# start NBLOCK
# start NRETURN
# start NINT
	push 1
# end NINT
	pop rax
	jmp .Lend0
# end NRETURN
# end NBLOCK
	jmp .Lend4
.Lelse3:
# start NBLOCK
# start NRETURN
# start NPLUS
# start NCALL
# start NMINUS
# start NVAR
# start Var n
	mov rax, rbp
	sub rax, 4
	push rax
# end Var n
	pop rax
	mov eax, [rax]
	push rax
# end NVAR
# start NINT
	push 1
# end NINT
	pop rdi
	pop rax
	sub eax, edi
	push rax
# end NMINUS
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
# end NCALL
# start NCALL
# start NMINUS
# start NVAR
# start Var n
	mov rax, rbp
	sub rax, 4
	push rax
# end Var n
	pop rax
	mov eax, [rax]
	push rax
# end NVAR
# start NINT
	push 2
# end NINT
	pop rdi
	pop rax
	sub eax, edi
	push rax
# end NMINUS
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
# end NCALL
	pop rdi
	pop rax
	add eax, edi
	push rax
# end NPLUS
	pop rax
	jmp .Lend0
# end NRETURN
# end NBLOCK
.Lend4:
# end NIFELSE
.Lend2:
# end NIFELSE
# end NBLOCK
.Lend0:
	leave
	ret
# end Function
# start Function
.text
.global main
main:
	push rbp
	mov rbp, rsp
	sub rsp, 0
# start NBLOCK
# start NRETURN
# start NCALL
# start NINT
	push 3
# end NINT
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
# end NCALL
	pop rax
	jmp .Lend5
# end NRETURN
# end NBLOCK
.Lend5:
	leave
	ret
# end Function
