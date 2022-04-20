    .code32
	.file	"memcmp.c"
	.text
	.p2align 4
	.globl	memcmp
	.type	memcmp, @function
memcmp:
	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$48, %esp
	movl	72(%esp), %ecx
	cmpl	$15, 76(%esp)
	movl	68(%esp), %edx
	movl	%ecx, %eax
	jbe	.L2
	testb	$3, %cl
	jne	.L3
	.p2align 4,,10
	.p2align 3
.L4:
	movl	76(%esp), %ebp
	movl	(%ecx), %ebx
	movl	%edx, %ecx
	shrl	$2, %ebp
	movl	%ebp, %edi
	movl	%ebp, (%esp)
	movl	%ebp, %esi
	andl	$3, %edi
	andl	$3, %ecx
	jne	.L7
	movl	(%edx), %ecx
	cmpl	$1, %edi
	je	.L8
	cmpl	$3, %edi
	je	.L9
	testl	%edi, %edi
	je	.L39
	addl	$2, %esi
	leal	-8(%edx), %edi
	movl	%esi, (%esp)
	movl	%edi, 4(%esp)
	leal	-8(%eax), %edi
.L11:
	movl	4(%esp), %esi
	movl	12(%esi), %ebp
	movl	12(%edi), %esi
	movl	%esi, 8(%esp)
	cmpl	%ebx, %ecx
	je	.L17
	movl	%ebx, 40(%esp)
	leal	40(%esp), %edx
	movl	%ecx, 44(%esp)
	leal	44(%esp), %ecx
	.p2align 4,,10
	.p2align 3
.L18:
	movzbl	(%ecx), %ebx
	movzbl	(%edx), %esi
	addl	$1, %ecx
	addl	$1, %edx
	cmpl	%esi, %ebx
	je	.L18
.L75:
	addl	$48, %esp
	movl	%ebx, %eax
	subl	%esi, %eax
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.p2align 4,,10
	.p2align 3
.L5:
	testb	$3, %al
	je	.L78
.L3:
	movzbl	(%edx), %ebx
	movzbl	(%eax), %esi
	addl	$1, %edx
	addl	$1, %eax
	subl	%esi, %ebx
	je	.L5
	movl	%ebx, %eax
.L1:
	addl	$48, %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
.L36:
	movl	76(%esp), %ecx
	andl	$3, 76(%esp)
	andl	$-4, %ecx
	addl	%ecx, %edx
	addl	%ecx, %eax
.L2:
	movl	76(%esp), %ecx
	testl	%ecx, %ecx
	je	.L55
	movzbl	(%edx), %ecx
	movzbl	(%eax), %esi
	subl	%esi, %ecx
	jne	.L38
	cmpl	$1, 76(%esp)
	je	.L55
	movzbl	1(%edx), %ecx
	movzbl	1(%eax), %ebx
	subl	%ebx, %ecx
	jne	.L38
	cmpl	$2, 76(%esp)
	je	.L55
	movzbl	2(%edx), %ecx
	movzbl	2(%eax), %esi
	subl	%esi, %ecx
	jne	.L38
	cmpl	$3, 76(%esp)
	je	.L55
	movzbl	3(%edx), %ecx
	movzbl	3(%eax), %ebx
	subl	%ebx, %ecx
	jne	.L38
	cmpl	$4, 76(%esp)
	je	.L55
	movzbl	4(%edx), %ecx
	movzbl	4(%eax), %esi
	subl	%esi, %ecx
	jne	.L38
	cmpl	$5, 76(%esp)
	je	.L55
	movzbl	5(%edx), %ecx
	movzbl	5(%eax), %ebx
	subl	%ebx, %ecx
	jne	.L38
	cmpl	$6, 76(%esp)
	je	.L55
	movzbl	6(%edx), %ecx
	movzbl	6(%eax), %esi
	subl	%esi, %ecx
	jne	.L38
	cmpl	$7, 76(%esp)
	je	.L55
	movzbl	7(%edx), %ecx
	movzbl	7(%eax), %ebx
	subl	%ebx, %ecx
	jne	.L38
	cmpl	$8, 76(%esp)
	je	.L55
	movzbl	8(%edx), %ecx
	movzbl	8(%eax), %esi
	subl	%esi, %ecx
	jne	.L38
	cmpl	$9, 76(%esp)
	je	.L55
	movzbl	9(%edx), %ecx
	movzbl	9(%eax), %ebx
	subl	%ebx, %ecx
	jne	.L38
	cmpl	$10, 76(%esp)
	je	.L55
	movzbl	10(%edx), %ecx
	movzbl	10(%eax), %esi
	subl	%esi, %ecx
	jne	.L38
	cmpl	$11, 76(%esp)
	je	.L55
	movzbl	11(%edx), %ecx
	movzbl	11(%eax), %ebx
	subl	%ebx, %ecx
	jne	.L38
	cmpl	$12, 76(%esp)
	je	.L55
	movzbl	12(%edx), %ecx
	movzbl	12(%eax), %esi
	leal	13(%edx), %ebp
	leal	13(%eax), %ebx
	subl	%esi, %ecx
	jne	.L38
	cmpl	$13, 76(%esp)
	je	.L55
	movzbl	0(%ebp), %ecx
	movzbl	(%ebx), %ebx
	addl	$14, %edx
	addl	$14, %eax
	subl	%ebx, %ecx
	jne	.L38
	cmpl	$14, 76(%esp)
	je	.L55
	movzbl	(%edx), %ecx
	movzbl	(%eax), %edx
	xorl	%eax, %eax
	subl	%edx, %ecx
	je	.L1
.L38:
	addl	$48, %esp
	movl	%ecx, %eax
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.p2align 4,,10
	.p2align 3
.L55:
	addl	$48, %esp
	xorl	%eax, %eax
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret
	.p2align 4,,10
	.p2align 3
.L78:
	addl	76(%esp), %ecx
	subl	%eax, %ecx
	movl	%ecx, 76(%esp)
	movl	%eax, %ecx
	jmp	.L4
	.p2align 4,,10
	.p2align 3
.L7:
	leal	0(,%ecx,8), %esi
	movl	$32, %ecx
	subl	%esi, %ecx
	movl	%esi, 16(%esp)
	movl	%ecx, 20(%esp)
	movl	%edx, %ecx
	andl	$-4, %ecx
	movl	(%ecx), %ebp
	leal	4(%ecx), %esi
	movl	%esi, 4(%esp)
	movl	4(%ecx), %esi
	movl	%ebp, 8(%esp)
	cmpl	$1, %edi
	je	.L23
	cmpl	$3, %edi
	je	.L24
	testl	%edi, %edi
	je	.L40
	addl	$2, (%esp)
	leal	-4(%ecx), %edi
	movl	%ebp, 24(%esp)
	movl	%edi, 4(%esp)
	leal	-8(%eax), %edi
	movl	%edi, 12(%esp)
	movl	%esi, %edi
.L26:
	movl	4(%esp), %esi
	movzbl	16(%esp), %ecx
	movl	%edi, %ebp
	movl	12(%esi), %esi
	movl	%esi, 8(%esp)
	movl	12(%esp), %esi
	movl	12(%esi), %esi
	movl	%esi, 28(%esp)
	movl	24(%esp), %esi
	shrl	%cl, %esi
	movzbl	20(%esp), %ecx
	sall	%cl, %ebp
	movl	%ebp, %ecx
	orl	%esi, %ecx
	cmpl	%ebx, %ecx
	je	.L32
	movl	%ebx, 40(%esp)
	leal	40(%esp), %edx
	movl	%ecx, 44(%esp)
	leal	44(%esp), %ecx
	.p2align 4,,10
	.p2align 3
.L33:
	movzbl	(%ecx), %ebx
	movzbl	(%edx), %esi
	addl	$1, %ecx
	addl	$1, %edx
	cmpl	%esi, %ebx
	je	.L33
	jmp	.L75
.L39:
	movl	%edx, 4(%esp)
	movl	%eax, %edi
.L10:
	movl	4(%esp), %esi
	movl	4(%esi), %ebp
	movl	4(%edi), %esi
	movl	%esi, 8(%esp)
	cmpl	%ebx, %ecx
	je	.L12
	movl	%ebx, 40(%esp)
	leal	40(%esp), %edx
	movl	%ecx, 44(%esp)
	leal	44(%esp), %ecx
	.p2align 4,,10
	.p2align 3
.L15:
	movzbl	(%ecx), %ebx
	movzbl	(%edx), %esi
	addl	$1, %ecx
	addl	$1, %edx
	cmpl	%esi, %ebx
	je	.L15
	jmp	.L75
.L9:
	addl	$1, (%esp)
	leal	-4(%edx), %edi
	movl	%ecx, %ebp
	movl	%ebx, 8(%esp)
	movl	%edi, 4(%esp)
	leal	-4(%eax), %edi
.L12:
	movl	4(%esp), %esi
	movl	8(%edi), %ebx
	movl	8(%esi), %ecx
	movl	8(%esp), %esi
	cmpl	%esi, %ebp
	je	.L11
	movl	%ebp, 44(%esp)
	leal	44(%esp), %ecx
	leal	40(%esp), %edx
	movl	%esi, 40(%esp)
	.p2align 4,,10
	.p2align 3
.L16:
	movzbl	(%ecx), %ebx
	movzbl	(%edx), %esi
	addl	$1, %ecx
	addl	$1, %edx
	cmpl	%esi, %ebx
	je	.L16
	jmp	.L75
.L40:
	movl	%eax, 12(%esp)
.L25:
	movl	4(%esp), %edi
	movzbl	16(%esp), %ecx
	movl	%esi, %ebp
	movl	4(%edi), %edi
	movl	%edi, 24(%esp)
	movl	12(%esp), %edi
	movl	4(%edi), %edi
	movl	%edi, 28(%esp)
	movl	8(%esp), %edi
	shrl	%cl, %edi
	movzbl	20(%esp), %ecx
	sall	%cl, %ebp
	orl	%ebp, %edi
	cmpl	%ebx, %edi
	je	.L27
	movl	%edi, 44(%esp)
	leal	44(%esp), %ecx
	leal	40(%esp), %edx
	movl	%ebx, 40(%esp)
	.p2align 4,,10
	.p2align 3
.L30:
	movzbl	(%ecx), %ebx
	movzbl	(%edx), %esi
	addl	$1, %ecx
	addl	$1, %edx
	cmpl	%esi, %ebx
	je	.L30
	jmp	.L75
.L24:
	leal	-4(%eax), %edi
	addl	$1, (%esp)
	movl	%edi, 12(%esp)
	movl	%ecx, 4(%esp)
	movl	%ebx, 28(%esp)
	movl	%esi, 24(%esp)
	movl	8(%esp), %esi
.L27:
	movzbl	16(%esp), %ecx
	movl	24(%esp), %ebp
	movl	4(%esp), %edi
	movl	12(%esp), %ebx
	shrl	%cl, %esi
	movzbl	20(%esp), %ecx
	movl	8(%edi), %edi
	movl	8(%ebx), %ebx
	sall	%cl, %ebp
	movl	28(%esp), %ecx
	orl	%ebp, %esi
	cmpl	%ecx, %esi
	je	.L26
	movl	%esi, 44(%esp)
	leal	40(%esp), %edx
	movl	%ecx, 40(%esp)
	leal	44(%esp), %ecx
	.p2align 4,,10
	.p2align 3
.L31:
	movzbl	(%ecx), %ebx
	movzbl	(%edx), %esi
	addl	$1, %ecx
	addl	$1, %edx
	cmpl	%esi, %ebx
	je	.L31
	jmp	.L75
.L8:
	subl	$1, (%esp)
	leal	4(%edx), %edi
	movl	%ecx, %ebp
	movl	%ebx, 8(%esp)
	movl	%edi, 4(%esp)
	leal	4(%eax), %edi
.L20:
	movl	4(%esp), %esi
	movl	(%edi), %ebx
	movl	(%esi), %ecx
	movl	%ebp, %esi
	movl	8(%esp), %ebp
	cmpl	%ebp, %esi
	je	.L10
	movl	%esi, 44(%esp)
	leal	44(%esp), %ecx
	leal	40(%esp), %edx
	movl	%ebp, 40(%esp)
	.p2align 4,,10
	.p2align 3
.L13:
	movzbl	(%ecx), %ebx
	movzbl	(%edx), %esi
	addl	$1, %ecx
	addl	$1, %edx
	cmpl	%esi, %ebx
	je	.L13
	jmp	.L75
.L23:
	leal	8(%ecx), %edi
	subl	$1, (%esp)
	movl	%esi, %ebp
	movl	%edi, 4(%esp)
	leal	4(%eax), %edi
	movl	%edi, 12(%esp)
	movl	8(%esp), %edi
	movl	%ebx, 28(%esp)
	movl	%esi, 8(%esp)
.L35:
	movzbl	16(%esp), %ecx
	movl	4(%esp), %esi
	movl	12(%esp), %ebx
	shrl	%cl, %edi
	movzbl	20(%esp), %ecx
	movl	(%esi), %esi
	movl	(%ebx), %ebx
	sall	%cl, %ebp
	movl	28(%esp), %ecx
	orl	%ebp, %edi
	cmpl	%ecx, %edi
	je	.L25
	movl	%edi, 44(%esp)
	leal	40(%esp), %edx
	movl	%ecx, 40(%esp)
	leal	44(%esp), %ecx
	.p2align 4,,10
	.p2align 3
.L28:
	movzbl	(%ecx), %ebx
	movzbl	(%edx), %esi
	addl	$1, %ecx
	addl	$1, %edx
	cmpl	%esi, %ebx
	je	.L28
	jmp	.L75
.L17:
	subl	$4, (%esp)
	je	.L19
	addl	$16, 4(%esp)
	addl	$16, %edi
	jmp	.L20
.L32:
	subl	$4, (%esp)
	je	.L34
	addl	$16, 4(%esp)
	movl	8(%esp), %ebp
	addl	$16, 12(%esp)
	jmp	.L35
.L19:
	cmpl	%esi, %ebp
	je	.L36
	movl	8(%esp), %eax
	movl	%ebp, 44(%esp)
	leal	44(%esp), %ecx
	leal	40(%esp), %edx
	movl	%eax, 40(%esp)
.L22:
	movzbl	(%ecx), %eax
	movzbl	(%edx), %esi
	addl	$1, %ecx
	addl	$1, %edx
	cmpl	%esi, %eax
	je	.L22
	subl	%esi, %eax
	jmp	.L1
.L34:
	movzbl	16(%esp), %ecx
	movl	8(%esp), %esi
	shrl	%cl, %edi
	movzbl	20(%esp), %ecx
	sall	%cl, %esi
	orl	%esi, %edi
	movl	28(%esp), %esi
	cmpl	%edi, %esi
	je	.L36
	movl	%edi, 44(%esp)
	leal	44(%esp), %ecx
	leal	40(%esp), %edx
	movl	%esi, 40(%esp)
.L37:
	movzbl	(%ecx), %eax
	movzbl	(%edx), %esi
	addl	$1, %ecx
	addl	$1, %edx
	cmpl	%esi, %eax
	je	.L37
	subl	%esi, %eax
	jmp	.L1
	.size	memcmp, .-memcmp
	.ident	"GCC: (Ubuntu 12-20220319-1ubuntu1) 12.0.1 20220319 (experimental) [master r12-7719-g8ca61ad148f]"
	.section	.note.GNU-stack,"",@progbits
