	.file	"semaphore_up.c"
	.text
	.p2align 4
	.globl	semaphore_up
	.type	semaphore_up, @function
semaphore_up:
#APP
# 5 "semaphore_up.c" 1
	cli
# 0 "" 2
# 10 "semaphore_up.c" 1
	movq  $1, %rax
.Ltsl_lock6:
	xchgq %rax, (%rdi)
	testq %rax, %rax
	jnz   .Ltsl_lock6
# 0 "" 2
#NO_APP
	movq	16(%rdi), %rax
	leaq	16(%rdi), %rdx
	cmpq	%rax, %rdx
	je	.L7
	movq	(%rax), %rcx
	movq	%rdx, 8(%rcx)
	movq	%rcx, 16(%rdi)
#APP
# 26 "semaphore_up.c" 1
	movq  $0, (%rdi)
# 0 "" 2
#NO_APP
	leaq	-65616(%rax), %rcx
	leaq	-80(%rax), %rdx
#APP
# 29 "semaphore_up.c" 1
	movq  $1, %rax
.Ltsl_lock18:
	xchgq %rax, sched_threads_mutex(%rip)
	testq %rax, %rax
	jnz   .Ltsl_lock18
# 0 "" 2
#NO_APP
	movq	index_sched_threads(%rip), %rax
	testq	%rax, %rax
	je	.L8
	movq	(%rax), %rsi
	movq	%rdx, 8(%rsi)
	movq	%rsi, 65536(%rcx)
	movq	%rax, 65544(%rcx)
	movq	%rdx, (%rax)
.L5:
#APP
# 39 "semaphore_up.c" 1
	movq  $0, sched_threads_mutex(%rip)
# 0 "" 2
# 40 "semaphore_up.c" 1
	sti
# 0 "" 2
#NO_APP
	ret
	.p2align 4,,10
	.p2align 3
.L8:
	movq	%rdx, 65544(%rcx)
	movq	%rdx, 65536(%rcx)
	movq	%rdx, index_sched_threads(%rip)
	jmp	.L5
	.p2align 4,,10
	.p2align 3
.L7:
	addq	$1, 8(%rdi)
#APP
# 14 "semaphore_up.c" 1
	movq  $0, (%rdi)
# 0 "" 2
# 15 "semaphore_up.c" 1
	sti
# 0 "" 2
#NO_APP
	ret
	.size	semaphore_up, .-semaphore_up
	.ident	"GCC: (Ubuntu 12-20220319-1ubuntu1) 12.0.1 20220319 (experimental) [master r12-7719-g8ca61ad148f]"
	.section	.note.GNU-stack,"",@progbits
