	.file	"semaphore_ups.c"
	.text
	.p2align 4
	.globl	semaphore_ups
	.type	semaphore_ups, @function
semaphore_ups:
	movq	%rdi, %rdx
	testq	%rsi, %rsi
	je	.L1
#APP
# 9 "semaphore_ups.c" 1
	cli
	movq  $1, %rax
.Ltsl_lock9:
	xchgq %rax, (%rdi)
	testq %rax, %rax
	jnz   .Ltsl_lock9
# 0 "" 2
#NO_APP
	movq	16(%rdi), %rax
	leaq	16(%rdi), %r8
	cmpq	%rax, %r8
	je	.L14
	movq	(%rax), %rcx
	leaq	-65616(%rax), %r9
	movq	%r8, 8(%rcx)
	movq	%rcx, 16(%rdi)
	leaq	-80(%rax), %rdi
#APP
# 22 "semaphore_ups.c" 1
	movq  $1, %rax
.Ltsl_lock20:
	xchgq %rax, sched_threads_mutex(%rip)
	testq %rax, %rax
	jnz   .Ltsl_lock20
# 0 "" 2
#NO_APP
	movq	index_sched_threads(%rip), %rcx
	testq	%rcx, %rcx
	je	.L16
	movq	(%rcx), %rax
	movq	%rdi, 8(%rax)
	movq	%rax, 65536(%r9)
	movq	%rcx, 65544(%r9)
	movq	%rdi, (%rcx)
.L5:
	subq	$1, %rsi
	je	.L9
	movq	16(%rdx), %rax
	cmpq	%rax, %r8
	je	.L7
.L17:
	movq	(%rax), %rdi
	movq	(%rcx), %r10
	leaq	-65616(%rax), %r9
	subq	$80, %rax
	movq	%r8, 8(%rdi)
	movq	%rdi, 16(%rdx)
	movq	%rax, 8(%r10)
	movq	%r10, 65536(%r9)
	movq	%rcx, 65544(%r9)
	movq	%rax, (%rcx)
	subq	$1, %rsi
	je	.L9
	movq	%rdi, %rax
	cmpq	%rax, %r8
	jne	.L17
.L7:
#APP
# 41 "semaphore_ups.c" 1
	movq  $0, sched_threads_mutex(%rip)
# 0 "" 2
#NO_APP
.L14:
	addq	%rsi, 8(%rdx)
#APP
# 43 "semaphore_ups.c" 1
	movq  $0, (%rdx)
	sti
# 0 "" 2
#NO_APP
.L1:
	ret
	.p2align 4,,10
	.p2align 3
.L16:
	movq	%rdi, 65544(%r9)
	movq	%rdi, %rcx
	movq	%rdi, 65536(%r9)
	movq	%rdi, index_sched_threads(%rip)
	jmp	.L5
	.p2align 4,,10
	.p2align 3
.L9:
	xorl	%esi, %esi
	jmp	.L7
	.size	semaphore_ups, .-semaphore_ups
	.ident	"GCC: (Ubuntu 12-20220319-1ubuntu1) 12.0.1 20220319 (experimental) [master r12-7719-g8ca61ad148f]"
	.section	.note.GNU-stack,"",@progbits
