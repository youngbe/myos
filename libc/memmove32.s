    .code32
	.file	"memmove.c"
	.text
	.p2align 4
	.globl	memmove
	.type	memmove, @function
memmove:
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	movl	20(%esp), %edi
	movl	24(%esp), %edx
	movl	28(%esp), %ebx
	cmpl	%edx, %edi
	jb	.L5
	leal	(%edx,%ebx), %eax
	cmpl	%eax, %edi
	jb	.L31
.L5:
	testl	%ebx, %ebx
	je	.L4
	movl	%edx, %eax
	leal	1(%edx), %esi
	orl	%edi, %eax
	testb	$3, %al
	movl	%edi, %eax
	sete	%cl
	subl	%esi, %eax
	cmpl	$2, %eax
	seta	%al
	testb	%al, %cl
	je	.L7
	leal	-1(%ebx), %eax
	cmpl	$3, %eax
	jbe	.L7
	movl	%ebx, %ebp
	movl	%edx, %eax
	movl	%edi, %esi
	andl	$-4, %ebp
	addl	%edx, %ebp
	.p2align 4,,10
	.p2align 3
.L8:
	movl	(%eax), %ecx
	addl	$4, %eax
	addl	$4, %esi
	movl	%ecx, -4(%esi)
	cmpl	%ebp, %eax
	jne	.L8
	movl	%ebx, %ecx
	andl	$-4, %ecx
	testb	$3, %bl
	je	.L4
	movzbl	(%edx,%ecx), %eax
	movb	%al, (%edi,%ecx)
	leal	1(%ecx), %eax
	cmpl	%ebx, %eax
	jnb	.L4
	movzbl	1(%edx,%ecx), %eax
	movb	%al, 1(%edi,%ecx)
	leal	2(%ecx), %eax
	cmpl	%ebx, %eax
	jnb	.L4
	movzbl	2(%edx,%ecx), %eax
	movb	%al, 2(%edi,%ecx)
.L4:
	popl	%ebx
	movl	%edi, %eax
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.p2align 4,,10
	.p2align 3
.L31:
	cmpl	%edi, %edx
	jnb	.L4
	testl	%ebx, %ebx
	je	.L4
	.p2align 4,,10
	.p2align 3
.L12:
	subl	$1, %ebx
	movzbl	(%edx,%ebx), %eax
	movb	%al, (%edi,%ebx)
	jne	.L12
	popl	%ebx
	movl	%edi, %eax
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.p2align 4,,10
	.p2align 3
.L7:
	movl	%edi, %eax
	addl	%edx, %ebx
	.p2align 4,,10
	.p2align 3
.L10:
	movzbl	(%edx), %ecx
	addl	$1, %edx
	addl	$1, %eax
	movb	%cl, -1(%eax)
	cmpl	%ebx, %edx
	jne	.L10
	popl	%ebx
	movl	%edi, %eax
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.size	memmove, .-memmove
	.ident	"GCC: (Ubuntu 12-20220319-1ubuntu1) 12.0.1 20220319 (experimental) [master r12-7719-g8ca61ad148f]"
	.section	.note.GNU-stack,"",@progbits
