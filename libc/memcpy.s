	.file	"memcpy.c"
	.text
	.p2align 4
	.globl	memcpy
	.type	memcpy, @function
memcpy:
	movq	%rdi, %rcx
	testq	%rdx, %rdx
	je	.L2
	leaq	1(%rsi), %rdi
	movq	%rcx, %rax
	subq	%rdi, %rax
	cmpq	$6, %rax
	leaq	-1(%rdx), %rax
	seta	%dil
	cmpq	$6, %rax
	seta	%al
	testb	%al, %dil
	je	.L7
	movq	%rcx, %rax
	orq	%rsi, %rax
	testb	$7, %al
	jne	.L7
	movq	%rdx, %r8
	xorl	%eax, %eax
	andq	$-8, %r8
	.p2align 4,,10
	.p2align 3
.L4:
	movq	(%rsi,%rax), %rdi
	movq	%rdi, (%rcx,%rax)
	addq	$8, %rax
	cmpq	%rax, %r8
	jne	.L4
	movq	%rdx, %rax
	andq	$-8, %rax
	testb	$7, %dl
	je	.L2
	movzbl	(%rsi,%rax), %edi
	movb	%dil, (%rcx,%rax)
	leaq	1(%rax), %rdi
	cmpq	%rdx, %rdi
	jnb	.L2
	movzbl	1(%rsi,%rax), %edi
	movb	%dil, 1(%rcx,%rax)
	leaq	2(%rax), %rdi
	cmpq	%rdx, %rdi
	jnb	.L2
	movzbl	2(%rsi,%rax), %edi
	movb	%dil, 2(%rcx,%rax)
	leaq	3(%rax), %rdi
	cmpq	%rdx, %rdi
	jnb	.L2
	movzbl	3(%rsi,%rax), %edi
	movb	%dil, 3(%rcx,%rax)
	leaq	4(%rax), %rdi
	cmpq	%rdx, %rdi
	jnb	.L2
	movzbl	4(%rsi,%rax), %edi
	movb	%dil, 4(%rcx,%rax)
	leaq	5(%rax), %rdi
	cmpq	%rdx, %rdi
	jnb	.L2
	movzbl	5(%rsi,%rax), %edi
	movb	%dil, 5(%rcx,%rax)
	leaq	6(%rax), %rdi
	cmpq	%rdx, %rdi
	jnb	.L2
	movzbl	6(%rsi,%rax), %edx
	movb	%dl, 6(%rcx,%rax)
.L2:
	movq	%rcx, %rax
	ret
	.p2align 4,,10
	.p2align 3
.L7:
	xorl	%eax, %eax
	.p2align 4,,10
	.p2align 3
.L3:
	movzbl	(%rsi,%rax), %edi
	movb	%dil, (%rcx,%rax)
	addq	$1, %rax
	cmpq	%rax, %rdx
	jne	.L3
	movq	%rcx, %rax
	ret
	.size	memcpy, .-memcpy
	.ident	"GCC: (Ubuntu 12-20220319-1ubuntu1) 12.0.1 20220319 (experimental) [master r12-7719-g8ca61ad148f]"
	.section	.note.GNU-stack,"",@progbits
