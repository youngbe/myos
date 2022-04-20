    .code32
	.file	"memset.c"
	.text
	.p2align 4
	.globl	memset
	.type	memset, @function
memset:
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$12, %esp
	movl	40(%esp), %eax
	movl	32(%esp), %edx
	movl	36(%esp), %ebp
	testl	%eax, %eax
	je	.L18
	leal	-4(%edx,%eax), %edi
	movl	%ebp, %ebx
	leal	-1(%eax), %esi
	movb	%bl, 4(%esp)
	movl	%edi, %ebx
	andl	$3, %ebx
	movl	%edi, 8(%esp)
	movl	$5, %edi
	leal	3(%ebx), %ecx
	movl	%esi, (%esp)
	cmpl	%edi, %ecx
	cmovb	%edi, %ecx
	cmpl	%ecx, %esi
	jb	.L3
	testl	%ebx, %ebx
	je	.L10
	movl	%ebp, %ecx
	movb	%cl, (%edx,%esi)
	testb	$2, 8(%esp)
	je	.L4
	leal	-2(%eax), %esi
	movb	%cl, -2(%edx,%eax)
	movl	%esi, (%esp)
	cmpl	$3, %ebx
	jne	.L4
	leal	-3(%eax), %esi
	movb	%cl, -3(%edx,%eax)
	movl	%esi, (%esp)
.L4:
	subl	%ebx, %eax
	movzbl	4(%esp), %ecx
	movl	%ebp, 4(%esp)
	movl	%eax, %esi
	movl	%ebp, %eax
	movb	4(%esp), %ch
	movzbl	%al, %ebx
	leal	-4(%edx,%esi), %eax
	movl	%ebx, %edi
	movzwl	%cx, %ecx
	sall	$24, %ebx
	sall	$16, %edi
	orl	%edi, %ecx
	movl	%esi, %edi
	orl	%ebx, %ecx
	andl	$-4, %edi
	movl	%eax, %ebx
	subl	%edi, %ebx
	.p2align 4,,10
	.p2align 3
.L6:
	movl	%ecx, (%eax)
	subl	$4, %eax
	cmpl	%ebx, %eax
	jne	.L6
	testl	$3, %esi
	je	.L18
	movl	(%esp), %eax
	andl	$-4, %esi
	subl	%esi, %eax
	leal	-1(%eax), %esi
.L3:
	movl	%ebp, %ebx
	movb	%bl, -1(%edx,%eax)
	testl	%esi, %esi
	je	.L18
	movb	%bl, -2(%edx,%eax)
	cmpl	$2, %eax
	je	.L18
	movb	%bl, -3(%edx,%eax)
	cmpl	$3, %eax
	je	.L18
	movb	%bl, -4(%edx,%eax)
	cmpl	$4, %eax
	je	.L18
	movb	%bl, -5(%edx,%eax)
	cmpl	$5, %eax
	je	.L18
	movb	%bl, -6(%edx,%eax)
.L18:
	addl	$12, %esp
	movl	%edx, %eax
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.p2align 4,,10
	.p2align 3
.L10:
	movl	%eax, (%esp)
	jmp	.L4
	.size	memset, .-memset
	.ident	"GCC: (Ubuntu 12-20220319-1ubuntu1) 12.0.1 20220319 (experimental) [master r12-7719-g8ca61ad148f]"
	.section	.note.GNU-stack,"",@progbits
