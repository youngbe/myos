    .code32
    .file	"memcpy.c"
	.text
	.p2align 4
	.globl	memcpy
	.type	memcpy, @function
memcpy:
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	movl	28(%esp), %ebx
	movl	20(%esp), %esi
	movl	24(%esp), %ecx
	testl	%ebx, %ebx
	je	.L2
	leal	1(%ecx), %edx
	movl	%esi, %eax
	subl	%edx, %eax
	cmpl	$2, %eax
	leal	-1(%ebx), %eax
	seta	%dl
	cmpl	$3, %eax
	seta	%al
	testb	%al, %dl
	je	.L3
	movl	%esi, %eax
	orl	%ecx, %eax
	testb	$3, %al
	jne	.L3
	movl	%ebx, %edi
	movl	%ecx, %eax
	movl	%esi, %edx
	andl	$-4, %edi
	addl	%ecx, %edi
	.p2align 4,,10
	.p2align 3
.L4:
	movl	(%eax), %ebp
	addl	$4, %eax
	addl	$4, %edx
	movl	%ebp, -4(%edx)
	cmpl	%edi, %eax
	jne	.L4
	movl	%ebx, %edx
	andl	$-4, %edx
	testb	$3, %bl
	je	.L2
	movzbl	(%ecx,%edx), %eax
	movb	%al, (%esi,%edx)
	leal	1(%edx), %eax
	cmpl	%ebx, %eax
	jnb	.L2
	movzbl	1(%ecx,%edx), %eax
	movb	%al, 1(%esi,%edx)
	leal	2(%edx), %eax
	cmpl	%ebx, %eax
	jnb	.L2
	movzbl	2(%ecx,%edx), %eax
	movb	%al, 2(%esi,%edx)
.L2:
	popl	%ebx
	movl	%esi, %eax
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.p2align 4,,10
	.p2align 3
.L3:
	movl	%ecx, %eax
	movl	%esi, %ecx
	addl	%eax, %ebx
	.p2align 4,,10
	.p2align 3
.L6:
	movzbl	(%eax), %edx
	addl	$1, %eax
	addl	$1, %ecx
	movb	%dl, -1(%ecx)
	cmpl	%ebx, %eax
	jne	.L6
	popl	%ebx
	movl	%esi, %eax
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.size	memcpy, .-memcpy
	.ident	"GCC: (Ubuntu 12-20220319-1ubuntu1) 12.0.1 20220319 (experimental) [master r12-7719-g8ca61ad148f]"
	.section	.note.GNU-stack,"",@progbits
