.intel_syntax noprefix
# start Function
# prototype
# end Function
# start Function
.text
.global main
main:
	push rbp
	mov rbp, rsp
	sub rsp, 3
# start NBLOCK
# start NDEFVAR
# end NDEFVAR
# start NEXPR_STMT
# start NASSIGN
#   start lval
# start lval NDEREF
# start NVAR
# start Var msg
	mov rax, rbp
	sub rax, 3
	push rax
# end Var msg
# emit array var
# end NVAR
# end lval NDEREF
#   end lval
# start NINT
	push 104
# end NINT
	pop rdi
	pop rax
	mov [rax], dil
	push rdi
# end NASSIGN
# end NEXPR_STMT
# start NEXPR_STMT
# start NASSIGN
#   start lval
# start lval NDEREF
# start NPLUS
# start NVAR
# start Var msg
	mov rax, rbp
	sub rax, 3
	push rax
# end Var msg
# emit array var
# end NVAR
# start NMUL
# start NINT
	push 1
# end NINT
# start NINT
	push 1
# end NINT
	pop rdi
	pop rax
	imul edi
	push rax
# end NMUL
	pop rdi
	pop rax
	add rax, rdi
	push rax
# end NPLUS
# end lval NDEREF
#   end lval
# start NINT
	push 105
# end NINT
	pop rdi
	pop rax
	mov [rax], dil
	push rdi
# end NASSIGN
# end NEXPR_STMT
# start NEXPR_STMT
# start NASSIGN
#   start lval
# start lval NDEREF
# start NPLUS
# start NVAR
# start Var msg
	mov rax, rbp
	sub rax, 3
	push rax
# end Var msg
# emit array var
# end NVAR
# start NMUL
# start NINT
	push 2
# end NINT
# start NINT
	push 1
# end NINT
	pop rdi
	pop rax
	imul edi
	push rax
# end NMUL
	pop rdi
	pop rax
	add rax, rdi
	push rax
# end NPLUS
# end lval NDEREF
#   end lval
# start NINT
	push 0
# end NINT
	pop rdi
	pop rax
	mov [rax], dil
	push rdi
# end NASSIGN
# end NEXPR_STMT
# start NEXPR_STMT
# start NCALL
# start NVAR
# start Var msg
	mov rax, rbp
	sub rax, 3
	push rax
# end Var msg
# emit array var
# end NVAR
	pop rdi
	push r10
	push r11
	mov rax, 0
	sub rsp, 8
	call puts
	pop r11
	pop r10
	add rsp, 8
	push rax
# end NCALL
# end NEXPR_STMT
# start NRETURN
# start NINT
	push 0
# end NINT
	pop rax
	jmp .Lend0
# end NRETURN
# end NBLOCK
.Lend0:
	leave
	ret
# end Function
