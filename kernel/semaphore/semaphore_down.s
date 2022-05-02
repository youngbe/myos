	.file	"semaphore_down.c"
	.text
	.p2align 4
	.globl	semaphore_down
	.type	semaphore_down, @function
semaphore_down:
	movq	%rdi, %rdx
#APP
# 5 "semaphore_down.c" 1
	cli
# 0 "" 2
# 10 "semaphore_down.c" 1
	movq  $1, %rax
.Ltsl_lock7:
	xchgq %rax, (%rdi)
	testq %rax, %rax
	jnz   .Ltsl_lock7
# 0 "" 2
#NO_APP
	movq	8(%rdi), %rax
	testq	%rax, %rax
	jne	.L9
	movq	running_threads(%rip), %rsi
	leaq	16(%rdi), %rcx
	movq	(%rsi), %rax
	movq	65616(%rax), %rdi
	movl	$1, 65560(%rax)
	movq	%rcx, 8(%rdi)
	movq	%rdi, 16(%rdx)
	leaq	65616(%rax), %rdi
	movq	%rdi, 24(%rdx)
	movq	%rcx, 65616(%rax)
#APP
# 27 "semaphore_down.c" 1
	pushq  %rbx
	pushq  %r12
	pushq  %r13
	pushq  %r14
	pushq  %r15
	pushq  %rbp
	movq   %rsp, 65552(%rax)
# 0 "" 2
# 48 "semaphore_down.c" 1
	movq  $1, %rax
.Ltsl_lock23:
	xchgq %rax, sched_threads_mutex(%rip)
	testq %rax, %rax
	jnz   .Ltsl_lock23
# 0 "" 2
#NO_APP
	movq	index_sched_threads(%rip), %rax
	testq	%rax, %rax
	je	.L10
	movq	(%rax), %rcx
	cmpq	%rax, %rcx
	je	.L11
	movq	8(%rax), %rdi
	movq	%rcx, index_sched_threads(%rip)
	movq	%rdi, 8(%rcx)
	movq	%rcx, (%rdi)
.L5:
#APP
# 74 "semaphore_down.c" 1
	movq  $0, sched_threads_mutex(%rip)
# 0 "" 2
#NO_APP
	subq	$65536, %rax
	movq	65568(%rax), %rcx
#APP
# 78 "semaphore_down.c" 1
	movq   %cr3, %rdi
	cmpq   %rdi, %rcx
	je     1f
	movq   %rcx, %cr3
1:
# 0 "" 2
# 88 "semaphore_down.c" 1
	movq  $0, (%rdx)
# 0 "" 2
#NO_APP
	movl	65560(%rax), %edx
	movq	%rax, (%rsi)
	testl	%edx, %edx
	jne	.L6
#APP
# 13 "../sched/switch.h" 1
	movq   65552(%rax), %rsp
	popq   %r11
	popq   %r10
	popq   %r9
	popq   %r8
	popq   %r15
	popq   %r14
	popq   %r13
	popq   %r12
	popq   %rbp
	popq   %rax
	popq   %rdx
	popq   %rcx
	popq   %rbx
	popq   %rsi
	popq   %rdi
	iretq
# 0 "" 2
	.p2align 4,,10
	.p2align 3
#NO_APP
.L9:
	subq	$1, %rax
	movq	%rax, 8(%rdi)
#APP
# 14 "semaphore_down.c" 1
	movq  $0, (%rdi)
# 0 "" 2
# 15 "semaphore_down.c" 1
	sti
# 0 "" 2
#NO_APP
	ret
	.p2align 4,,10
	.p2align 3
.L6:
#APP
# 38 "../sched/switch.h" 1
	movq   65552(%rax), %rsp
	sti
	popq   %r15
	popq   %r14
	popq   %r13
	popq   %r12
	popq   %rbp
	popq   %rbx
	retq
# 0 "" 2
	.p2align 4,,10
	.p2align 3
#NO_APP
.L11:
	movq	$0, index_sched_threads(%rip)
	jmp	.L5
	.p2align 4,,10
	.p2align 3
.L10:
#APP
# 53 "semaphore_down.c" 1
	movq  $0, sched_threads_mutex(%rip)
# 0 "" 2
#NO_APP
	leaq	halt_pt0(%rip), %rax
#APP
# 54 "semaphore_down.c" 1
	movq   %rax, %cr3
# 0 "" 2
# 59 "semaphore_down.c" 1
	movq  $0, (%rdx)
# 0 "" 2
#NO_APP
	movq	halt_stacks(%rip), %rax
	movq	$0, (%rsi)
	addq	$4096, %rax
#APP
# 59 "../sched/switch.h" 1
	movq   %rax, %rsp
	sti
1:
	hlt
	jmp    1b
# 0 "" 2
#NO_APP
	.size	semaphore_down, .-semaphore_down
	.ident	"GCC: (Ubuntu 12-20220319-1ubuntu1) 12.0.1 20220319 (experimental) [master r12-7719-g8ca61ad148f]"
	.section	.note.GNU-stack,"",@progbits
