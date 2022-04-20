	.file	"memset.c"
	.text
	.p2align 4
	.globl	memset
	.type	memset, @function
memset:
	movq	%rdi, %rcx
	testq	%rdx, %rdx
	je	.L65
	leaq	-8(%rdi,%rdx), %r9
	movl	$11, %r10d
	leaq	-1(%rdx), %r8
	pushq	%rbx
	movq	%r9, %rax
	movl	%esi, %ebx
	movl	%esi, %r11d
	movq	%r8, %rsi
	andl	$7, %eax
	leaq	7(%rax), %rdi
	cmpq	%r10, %rdi
	cmovb	%r10, %rdi
	cmpq	%rdi, %r8
	jb	.L3
	testq	%rax, %rax
	je	.L10
	movb	%bl, (%rcx,%r8)
	testb	$6, %r9b
	je	.L4
	movb	%bl, -2(%rcx,%rdx)
	leaq	-2(%rdx), %rsi
	cmpq	$2, %rax
	jbe	.L4
	andl	$4, %r9d
	movb	%bl, -3(%rcx,%rdx)
	leaq	-3(%rdx), %rsi
	je	.L4
	movb	%bl, -4(%rcx,%rdx)
	leaq	-4(%rdx), %rsi
	cmpq	$4, %rax
	jbe	.L4
	movb	%bl, -5(%rcx,%rdx)
	leaq	-5(%rdx), %rsi
	cmpq	$5, %rax
	je	.L4
	movb	%bl, -6(%rcx,%rdx)
	leaq	-6(%rdx), %rsi
	cmpq	$7, %rax
	jne	.L4
	movb	%bl, -7(%rcx,%rdx)
	leaq	-7(%rdx), %rsi
.L4:
	movabsq	$-4278190081, %r9
	movq	%rdx, %rdi
	movzbl	%bl, %edx
	subq	%rax, %rdi
	movzbl	%r11b, %eax
	movq	%rdx, %r8
	movb	%bl, %ah
	salq	$16, %r8
	andq	$-16711681, %rax
	orq	%r8, %rax
	movq	%rdx, %r8
	salq	$24, %r8
	andq	%r9, %rax
	movabsq	$-1095216660481, %r9
	orq	%r8, %rax
	movq	%rdx, %r8
	salq	$32, %r8
	andq	%r9, %rax
	movabsq	$-280375465082881, %r9
	orq	%r8, %rax
	movq	%rdx, %r8
	salq	$40, %r8
	andq	%r9, %rax
	movabsq	$-71776119061217281, %r9
	orq	%r8, %rax
	movq	%rdx, %r8
	salq	$56, %rdx
	salq	$48, %r8
	andq	%r9, %rax
	movq	%rdi, %r9
	orq	%r8, %rax
	andq	$-8, %r9
	movabsq	$72057594037927935, %r8
	andq	%r8, %rax
	orq	%rdx, %rax
	leaq	-8(%rcx,%rdi), %rdx
	movq	%rdx, %r8
	subq	%r9, %r8
	.p2align 4,,10
	.p2align 3
.L6:
	movq	%rax, (%rdx)
	subq	$8, %rdx
	cmpq	%r8, %rdx
	jne	.L6
	movq	%rdi, %rax
	andq	$-8, %rax
	subq	%rax, %rsi
	andl	$7, %edi
	movq	%rsi, %rdx
	je	.L28
	leaq	-1(%rsi), %r8
.L3:
	movb	%bl, -1(%rcx,%rdx)
	testq	%r8, %r8
	je	.L28
	movb	%bl, -2(%rcx,%rdx)
	cmpq	$2, %rdx
	je	.L28
	movb	%bl, -3(%rcx,%rdx)
	cmpq	$3, %rdx
	je	.L28
	movb	%bl, -4(%rcx,%rdx)
	cmpq	$4, %rdx
	je	.L28
	movb	%bl, -5(%rcx,%rdx)
	cmpq	$5, %rdx
	je	.L28
	movb	%bl, -6(%rcx,%rdx)
	cmpq	$6, %rdx
	je	.L28
	movb	%bl, -7(%rcx,%rdx)
	cmpq	$7, %rdx
	je	.L28
	movb	%bl, -8(%rcx,%rdx)
	cmpq	$8, %rdx
	je	.L28
	movb	%bl, -9(%rcx,%rdx)
	cmpq	$9, %rdx
	je	.L28
	movb	%bl, -10(%rcx,%rdx)
	cmpq	$10, %rdx
	je	.L28
	movb	%bl, -11(%rcx,%rdx)
	cmpq	$11, %rdx
	je	.L28
	movb	%bl, -12(%rcx,%rdx)
	cmpq	$12, %rdx
	je	.L28
	movb	%bl, -13(%rcx,%rdx)
	cmpq	$13, %rdx
	je	.L28
	movb	%bl, -14(%rcx,%rdx)
.L28:
	movq	%rcx, %rax
	popq	%rbx
	ret
	.p2align 4,,10
	.p2align 3
.L65:
	movq	%rdi, %rax
	ret
	.p2align 4,,10
	.p2align 3
.L10:
	movq	%rdx, %rsi
	jmp	.L4
	.size	memset, .-memset
	.ident	"GCC: (Ubuntu 12-20220319-1ubuntu1) 12.0.1 20220319 (experimental) [master r12-7719-g8ca61ad148f]"
	.section	.note.GNU-stack,"",@progbits
