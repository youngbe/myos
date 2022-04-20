	.file	"memmove.c"
	.text
	.p2align 4
	.globl	memmove
	.type	memmove, @function
memmove:
	movq	%rsi, %r8
	cmpq	%rsi, %rdi
	jb	.L5
	leaq	(%rsi,%rdx), %rax
	cmpq	%rax, %rdi
	jb	.L30
.L5:
	testq	%rdx, %rdx
	je	.L4
	movq	%r8, %rax
	leaq	1(%r8), %rsi
	orq	%rdi, %rax
	testb	$7, %al
	movq	%rdi, %rax
	sete	%cl
	subq	%rsi, %rax
	cmpq	$6, %rax
	seta	%al
	testb	%al, %cl
	je	.L12
	leaq	-1(%rdx), %rax
	cmpq	$6, %rax
	jbe	.L12
	movq	%rdx, %rsi
	xorl	%eax, %eax
	andq	$-8, %rsi
	.p2align 4,,10
	.p2align 3
.L8:
	movq	(%r8,%rax), %rcx
	movq	%rcx, (%rdi,%rax)
	addq	$8, %rax
	cmpq	%rax, %rsi
	jne	.L8
	movq	%rdx, %rax
	andq	$-8, %rax
	testb	$7, %dl
	je	.L4
	movzbl	(%r8,%rax), %ecx
	movb	%cl, (%rdi,%rax)
	leaq	1(%rax), %rcx
	cmpq	%rdx, %rcx
	jnb	.L4
	movzbl	1(%r8,%rax), %ecx
	movb	%cl, 1(%rdi,%rax)
	leaq	2(%rax), %rcx
	cmpq	%rdx, %rcx
	jnb	.L4
	movzbl	2(%r8,%rax), %ecx
	movb	%cl, 2(%rdi,%rax)
	leaq	3(%rax), %rcx
	cmpq	%rdx, %rcx
	jnb	.L4
	movzbl	3(%r8,%rax), %ecx
	movb	%cl, 3(%rdi,%rax)
	leaq	4(%rax), %rcx
	cmpq	%rdx, %rcx
	jnb	.L4
	movzbl	4(%r8,%rax), %ecx
	movb	%cl, 4(%rdi,%rax)
	leaq	5(%rax), %rcx
	cmpq	%rdx, %rcx
	jnb	.L4
	movzbl	5(%r8,%rax), %ecx
	movb	%cl, 5(%rdi,%rax)
	leaq	6(%rax), %rcx
	cmpq	%rdx, %rcx
	jnb	.L4
	movzbl	6(%r8,%rax), %edx
	movb	%dl, 6(%rdi,%rax)
.L4:
	movq	%rdi, %rax
	ret
	.p2align 4,,10
	.p2align 3
.L30:
	cmpq	%rdi, %rsi
	jnb	.L4
	testq	%rdx, %rdx
	je	.L4
	.p2align 4,,10
	.p2align 3
.L11:
	subq	$1, %rdx
	movzbl	(%r8,%rdx), %eax
	movb	%al, (%rdi,%rdx)
	jne	.L11
	movq	%rdi, %rax
	ret
	.p2align 4,,10
	.p2align 3
.L12:
	xorl	%eax, %eax
	.p2align 4,,10
	.p2align 3
.L7:
	movzbl	(%r8,%rax), %ecx
	movb	%cl, (%rdi,%rax)
	addq	$1, %rax
	cmpq	%rax, %rdx
	jne	.L7
	movq	%rdi, %rax
	ret
	.size	memmove, .-memmove
	.ident	"GCC: (Ubuntu 12-20220319-1ubuntu1) 12.0.1 20220319 (experimental) [master r12-7719-g8ca61ad148f]"
	.section	.note.GNU-stack,"",@progbits
