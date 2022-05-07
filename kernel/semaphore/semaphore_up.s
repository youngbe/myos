	.file	"semaphore_up.c"
	.text
	.p2align 4
	.globl	semaphore_up
	.type	semaphore_up, @function
semaphore_up:
	movl	$1, %ecx
#APP
# 6 "kernel/semaphore/semaphore_up.c" 1
	cli
.Ltsl_lock6:
	xorq  %rax, %rax
	cmpxchgq %rcx, (%rdi)
	jne   .Ltsl_lock6
# 0 "" 2
#NO_APP
	movq	16(%rdi), %rax
	leaq	16(%rdi), %rdx
	cmpq	%rax, %rdx
	je	.L7
	movq	(%rax), %rsi
	movq	%rdx, 8(%rsi)
	movq	%rsi, 16(%rdi)
#APP
# 17 "kernel/semaphore/semaphore_up.c" 1
	movq  $0, (%rdi)
# 0 "" 2
#NO_APP
	leaq	-65616(%rax), %rsi
	leaq	-80(%rax), %rdx
#APP
# 20 "kernel/semaphore/semaphore_up.c" 1
	.Ltsl_lock18:
	xorq  %rax, %rax
	cmpxchgq %rcx, sched_threads_mutex(%rip)
	jne   .Ltsl_lock18
# 0 "" 2
#NO_APP
	movq	index_sched_threads(%rip), %rax
	testq	%rax, %rax
	je	.L8
	movq	(%rax), %rcx
	movq	%rdx, 8(%rcx)
	movq	%rcx, 65536(%rsi)
	movq	%rax, 65544(%rsi)
	movq	%rdx, (%rax)
.L5:
#APP
# 30 "kernel/semaphore/semaphore_up.c" 1
	movq  $0, sched_threads_mutex(%rip)
	sti
# 0 "" 2
#NO_APP
	ret
	.p2align 4,,10
	.p2align 3
.L8:
	movq	%rdx, 65544(%rsi)
	movq	%rdx, 65536(%rsi)
	movq	%rdx, index_sched_threads(%rip)
	jmp	.L5
	.p2align 4,,10
	.p2align 3
.L7:
	addq	$1, 8(%rdi)
#APP
# 10 "kernel/semaphore/semaphore_up.c" 1
	movq  $0, (%rdi)
	sti
# 0 "" 2
#NO_APP
	ret
	.size	semaphore_up, .-semaphore_up
	.ident	"GCC: (Ubuntu 12-20220319-1ubuntu1) 12.0.1 20220319 (experimental) [master r12-7719-g8ca61ad148f]"
	.section	.note.GNU-stack,"",@progbits
