	.file	"memcmp2.c"
	.text
	.p2align 4
	.globl	memcmp
	.type	memcmp, @function
memcmp:
	pushq	%r15
	movq	%rdi, %rax
	movq	%rsi, %rcx
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	cmpq	$15, %rdx
	jbe	.L2
	testb	$7, %sil
	jne	.L6
	movq	%rsi, %r8
	jmp	.L7
	.p2align 4,,10
	.p2align 3
.L92:
	testb	$7, %cl
	je	.L91
.L6:
	movzbl	(%rax), %edi
	movzbl	(%rcx), %r8d
	addq	$1, %rax
	addq	$1, %rcx
	subq	%r8, %rdi
	je	.L92
.L1:
	popq	%rbx
	movl	%edi, %eax
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
.L28:
	movq	(%rcx), %r14
	movq	0(%r13), %r11
	movq	%r13, %r12
	addq	$1, %rsi
	movq	8(%r13), %rbp
	leaq	-8(%r8), %r13
.L31:
	movl	%edi, %ecx
	movq	%rbp, %rbx
	movq	16(%r12), %r10
	movq	16(%r13), %r15
	shrq	%cl, %r11
	movl	%r9d, %ecx
	salq	%cl, %rbx
	orq	%rbx, %r11
	cmpq	%r14, %r11
	je	.L30
	movq	%r11, -8(%rsp)
	leaq	-8(%rsp), %rsi
	leaq	-16(%rsp), %rcx
	movq	%r14, -16(%rsp)
	.p2align 4,,10
	.p2align 3
.L37:
	movzbl	(%rsi), %edi
	movzbl	(%rcx), %r9d
	addq	$1, %rsi
	addq	$1, %rcx
	cmpb	%r9b, %dil
	je	.L37
	.p2align 4,,10
	.p2align 3
.L88:
	subl	%r9d, %edi
	movslq	%edi, %rcx
	testq	%rcx, %rcx
	jne	.L1
.L24:
	movq	%rdx, %rcx
	andl	$7, %edx
	andq	$-8, %rcx
	addq	%rcx, %rax
	addq	%r8, %rcx
.L2:
	testq	%rdx, %rdx
	je	.L57
	movzbl	(%rax), %esi
	movzbl	(%rcx), %edi
	subq	%rdi, %rsi
	jne	.L42
	cmpq	$1, %rdx
	je	.L57
	movzbl	1(%rax), %esi
	movzbl	1(%rcx), %edi
	subq	%rdi, %rsi
	jne	.L42
	cmpq	$2, %rdx
	je	.L57
	movzbl	2(%rax), %esi
	movzbl	2(%rcx), %edi
	subq	%rdi, %rsi
	jne	.L42
	cmpq	$3, %rdx
	je	.L57
	movzbl	3(%rax), %esi
	movzbl	3(%rcx), %edi
	subq	%rdi, %rsi
	jne	.L42
	cmpq	$4, %rdx
	je	.L57
	movzbl	4(%rax), %esi
	movzbl	4(%rcx), %edi
	subq	%rdi, %rsi
	jne	.L42
	cmpq	$5, %rdx
	je	.L57
	movzbl	5(%rax), %esi
	movzbl	5(%rcx), %edi
	subq	%rdi, %rsi
	jne	.L42
	cmpq	$6, %rdx
	je	.L57
	movzbl	6(%rax), %esi
	movzbl	6(%rcx), %edi
	subq	%rdi, %rsi
	jne	.L42
	cmpq	$7, %rdx
	je	.L57
	movzbl	7(%rax), %esi
	movzbl	7(%rcx), %edi
	subq	%rdi, %rsi
	jne	.L42
	cmpq	$8, %rdx
	je	.L57
	movzbl	8(%rax), %esi
	movzbl	8(%rcx), %edi
	subq	%rdi, %rsi
	jne	.L42
	cmpq	$9, %rdx
	je	.L57
	movzbl	9(%rax), %esi
	movzbl	9(%rcx), %edi
	subq	%rdi, %rsi
	jne	.L42
	cmpq	$10, %rdx
	je	.L57
	movzbl	10(%rax), %esi
	movzbl	10(%rcx), %edi
	subq	%rdi, %rsi
	jne	.L42
	cmpq	$11, %rdx
	je	.L57
	movzbl	11(%rax), %esi
	movzbl	11(%rcx), %edi
	subq	%rdi, %rsi
	jne	.L42
	cmpq	$12, %rdx
	je	.L57
	movzbl	12(%rax), %esi
	movzbl	12(%rcx), %edi
	leaq	13(%rax), %r9
	leaq	13(%rcx), %r8
	subq	%rdi, %rsi
	jne	.L42
	cmpq	$13, %rdx
	je	.L57
	movzbl	(%r9), %esi
	movzbl	(%r8), %edi
	addq	$14, %rax
	addq	$14, %rcx
	subq	%rdi, %rsi
	jne	.L42
	cmpq	$14, %rdx
	je	.L57
	movzbl	(%rax), %esi
	movzbl	(%rcx), %eax
	xorl	%edi, %edi
	subq	%rax, %rsi
	je	.L1
.L42:
	movl	%esi, %edi
	popq	%rbx
	popq	%rbp
	movl	%edi, %eax
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
	.p2align 4,,10
	.p2align 3
.L57:
	xorl	%edi, %edi
	popq	%rbx
	popq	%rbp
	movl	%edi, %eax
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret
	.p2align 4,,10
	.p2align 3
.L91:
	addq	%rdx, %rsi
	movq	%rcx, %r8
	movq	%rsi, %rdx
	subq	%rcx, %rdx
.L7:
	movq	%rdx, %rsi
	movq	%rax, %rdi
	shrq	$3, %rsi
	movq	%rsi, %r10
	andl	$3, %r10d
	andl	$7, %edi
	jne	.L8
	cmpq	$1, %r10
	je	.L9
	cmpq	$3, %r10
	je	.L10
	testq	%r10, %r10
	je	.L11
	movq	(%rax), %rbx
	movq	(%rcx), %r11
	leaq	-16(%rax), %rdi
	leaq	-16(%r8), %rcx
	addq	$2, %rsi
.L12:
	movq	24(%rdi), %r10
	movq	24(%rcx), %r9
	cmpq	%r11, %rbx
	je	.L21
	movq	%rbx, -8(%rsp)
	leaq	-8(%rsp), %rsi
	leaq	-16(%rsp), %rcx
	movq	%r11, -16(%rsp)
	.p2align 4,,10
	.p2align 3
.L22:
	movzbl	(%rsi), %edi
	movzbl	(%rcx), %r9d
	addq	$1, %rsi
	addq	$1, %rcx
	cmpb	%r9b, %dil
	je	.L22
	jmp	.L88
	.p2align 4,,10
	.p2align 3
.L8:
	sall	$3, %edi
	movl	$64, %r9d
	movq	%rax, %r13
	subl	%edi, %r9d
	andq	$-8, %r13
	cmpq	$1, %r10
	je	.L27
	cmpq	$3, %r10
	je	.L28
	testq	%r10, %r10
	je	.L29
	movq	(%rcx), %r15
	movq	0(%r13), %rbp
	leaq	-8(%r13), %r12
	addq	$2, %rsi
	movq	8(%r13), %r10
	leaq	-16(%r8), %r13
.L30:
	movl	%edi, %ecx
	movq	%r10, %r11
	movq	24(%r12), %rbx
	movq	24(%r13), %r14
	shrq	%cl, %rbp
	movl	%r9d, %ecx
	salq	%cl, %r11
	orq	%r11, %rbp
	cmpq	%r15, %rbp
	je	.L38
	movq	%rbp, -8(%rsp)
	leaq	-8(%rsp), %rsi
	leaq	-16(%rsp), %rcx
	movq	%r15, -16(%rsp)
	.p2align 4,,10
	.p2align 3
.L39:
	movzbl	(%rsi), %edi
	movzbl	(%rcx), %r9d
	addq	$1, %rsi
	addq	$1, %rcx
	cmpb	%r9b, %dil
	je	.L39
	jmp	.L88
.L9:
	movq	(%rax), %r10
	movq	(%rcx), %r9
	subq	$1, %rsi
	je	.L16
	leaq	8(%rax), %rdi
	leaq	8(%r8), %rcx
.L23:
	movq	(%rdi), %rbx
	movq	(%rcx), %r11
	cmpq	%r9, %r10
	je	.L15
	movq	%r10, -8(%rsp)
	leaq	-8(%rsp), %rsi
	leaq	-16(%rsp), %rcx
	movq	%r9, -16(%rsp)
	.p2align 4,,10
	.p2align 3
.L17:
	movzbl	(%rsi), %edi
	movzbl	(%rcx), %r9d
	addq	$1, %rsi
	addq	$1, %rcx
	cmpb	%r9b, %dil
	je	.L17
	jmp	.L88
.L11:
	testq	%rsi, %rsi
	je	.L24
	movq	(%rax), %rbx
	movq	(%rcx), %r11
	movq	%rax, %rdi
	movq	%r8, %rcx
.L15:
	movq	8(%rdi), %r10
	movq	8(%rcx), %r9
	cmpq	%r11, %rbx
	je	.L13
	movq	%rbx, -8(%rsp)
	leaq	-8(%rsp), %rsi
	leaq	-16(%rsp), %rcx
	movq	%r11, -16(%rsp)
	.p2align 4,,10
	.p2align 3
.L19:
	movzbl	(%rsi), %edi
	movzbl	(%rcx), %r9d
	addq	$1, %rsi
	addq	$1, %rcx
	cmpb	%r9b, %dil
	je	.L19
	jmp	.L88
.L10:
	movq	(%rax), %r10
	movq	(%rcx), %r9
	leaq	-8(%rax), %rdi
	leaq	-8(%r8), %rcx
	addq	$1, %rsi
.L13:
	movq	16(%rdi), %rbx
	movq	16(%rcx), %r11
	cmpq	%r9, %r10
	je	.L12
	movq	%r10, -8(%rsp)
	leaq	-8(%rsp), %rsi
	leaq	-16(%rsp), %rcx
	movq	%r9, -16(%rsp)
	.p2align 4,,10
	.p2align 3
.L20:
	movzbl	(%rsi), %edi
	movzbl	(%rcx), %r9d
	addq	$1, %rsi
	addq	$1, %rcx
	cmpb	%r9b, %dil
	je	.L20
	jmp	.L88
.L29:
	testq	%rsi, %rsi
	je	.L24
	movq	(%rcx), %r15
	movq	0(%r13), %rbx
	leaq	8(%r13), %r12
	movq	8(%r13), %r11
	movq	%r8, %r13
.L32:
	movl	%edi, %ecx
	movq	%r11, %r10
	movq	8(%r12), %rbp
	movq	8(%r13), %r14
	shrq	%cl, %rbx
	movl	%r9d, %ecx
	salq	%cl, %r10
	orq	%r10, %rbx
	cmpq	%r15, %rbx
	je	.L31
	movq	%rbx, -8(%rsp)
	leaq	-8(%rsp), %rsi
	leaq	-16(%rsp), %rcx
	movq	%r15, -16(%rsp)
	.p2align 4,,10
	.p2align 3
.L36:
	movzbl	(%rsi), %edi
	movzbl	(%rcx), %r9d
	addq	$1, %rsi
	addq	$1, %rcx
	cmpb	%r9b, %dil
	je	.L36
	jmp	.L88
.L27:
	movq	0(%r13), %r10
	movq	8(%r13), %rbx
	movq	(%rcx), %r14
	subq	$1, %rsi
	je	.L33
	leaq	16(%r13), %r12
	leaq	8(%r8), %r13
.L40:
	movl	%edi, %ecx
	movq	%rbx, %rbp
	movq	(%r12), %r11
	movq	0(%r13), %r15
	shrq	%cl, %r10
	movl	%r9d, %ecx
	salq	%cl, %rbp
	orq	%rbp, %r10
	cmpq	%r14, %r10
	je	.L32
	movq	%r10, -8(%rsp)
	leaq	-8(%rsp), %rsi
	leaq	-16(%rsp), %rcx
	movq	%r14, -16(%rsp)
	.p2align 4,,10
	.p2align 3
.L34:
	movzbl	(%rsi), %edi
	movzbl	(%rcx), %r9d
	addq	$1, %rsi
	addq	$1, %rcx
	cmpb	%r9b, %dil
	je	.L34
	jmp	.L88
.L16:
	cmpq	%r9, %r10
	je	.L24
	movq	%r10, -8(%rsp)
	leaq	-8(%rsp), %rsi
	leaq	-16(%rsp), %rcx
	movq	%r9, -16(%rsp)
	.p2align 4,,10
	.p2align 3
.L25:
	movzbl	(%rsi), %edi
	movzbl	(%rcx), %r9d
	addq	$1, %rsi
	addq	$1, %rcx
	cmpb	%r9b, %dil
	je	.L25
	jmp	.L88
.L33:
	movl	%edi, %ecx
	shrq	%cl, %r10
	movl	%r9d, %ecx
	salq	%cl, %rbx
	orq	%rbx, %r10
	cmpq	%r14, %r10
	je	.L24
	movq	%r10, -8(%rsp)
	leaq	-8(%rsp), %rsi
	leaq	-16(%rsp), %rcx
	movq	%r14, -16(%rsp)
	.p2align 4,,10
	.p2align 3
.L41:
	movzbl	(%rsi), %edi
	movzbl	(%rcx), %r9d
	addq	$1, %rsi
	addq	$1, %rcx
	cmpb	%r9b, %dil
	je	.L41
	jmp	.L88
.L38:
	subq	$4, %rsi
	je	.L33
	addq	$32, %r12
	addq	$32, %r13
	jmp	.L40
.L21:
	subq	$4, %rsi
	je	.L16
	addq	$32, %rdi
	addq	$32, %rcx
	jmp	.L23
	.size	memcmp, .-memcmp
	.ident	"GCC: (Ubuntu 12-20220319-1ubuntu1) 12.0.1 20220319 (experimental) [master r12-7719-g8ca61ad148f]"
	.section	.note.GNU-stack,"",@progbits
