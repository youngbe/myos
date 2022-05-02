	.file	"timer_isr.c"
	.text
	.p2align 4
	.globl	timer_isr
	.type	timer_isr, @function
timer_isr:
	pushq	%rdi
	pushq	%rsi
	pushq	%rbx
	pushq	%rcx
	pushq	%rdx
	movq	running_threads(%rip), %rdx
	pushq	%rax
	movq	(%rdx), %rsi
#APP
# 18 "timer_isr.c" 1
	movq  $1, %rax
.Ltsl_lock13:
	xchgq %rax, sched_threads_mutex(%rip)
	testq %rax, %rax
	jnz   .Ltsl_lock13
# 0 "" 2
#NO_APP
	movq	index_sched_threads(%rip), %rax
	testq	%rax, %rax
	jne	.L2
#APP
# 22 "timer_isr.c" 1
	movq  $0, sched_threads_mutex(%rip)
# 0 "" 2
#NO_APP
	popq	%rax
	popq	%rdx
	popq	%rcx
	popq	%rbx
	popq	%rsi
	popq	%rdi
	iretq
.L2:
	movq	(%rax), %rdi
	testq	%rsi, %rsi
	je	.L12
#APP
# 41 "timer_isr.c" 1
	pushq  %rbp
	pushq  %r12
	pushq  %r13
	pushq  %r14
	pushq  %r15
	pushq  %r8
	pushq  %r9
	pushq  %r10
	pushq  %r11
	movq   %rsp, 65552(%rsi)
# 0 "" 2
#NO_APP
	movl	$0, 65560(%rsi)
	leaq	65536(%rsi), %rcx
	cmpq	%rdi, %rax
	je	.L13
	movq	%rdi, 65536(%rsi)
	movq	%rcx, 8(%rdi)
	movq	8(%rax), %rbx
	movq	%rbx, 65544(%rsi)
	movq	%rcx, (%rbx)
	movq	%rdi, index_sched_threads(%rip)
.L5:
#APP
# 73 "timer_isr.c" 1
	movq  $0, sched_threads_mutex(%rip)
# 0 "" 2
#NO_APP
	leaq	-65536(%rax), %rsi
	cmpl	$0, 65560(%rsi)
	movq	%rsi, (%rdx)
	movq	tsss(%rip), %rdx
	movq	%rax, 4(%rdx)
	jne	.L7
#APP
# 13 "../sched/switch.h" 1
	movq   65552(%rsi), %rsp
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
#NO_APP
.L7:
#APP
# 38 "../sched/switch.h" 1
	movq   65552(%rsi), %rsp
	sti
	popq   %r15
	popq   %r14
	popq   %r13
	popq   %r12
	popq   %rbp
	popq   %rbx
	retq
# 0 "" 2
#NO_APP
.L13:
	movq	%rcx, index_sched_threads(%rip)
	movq	%rcx, 65544(%rsi)
	movq	%rcx, 65536(%rsi)
	jmp	.L5
.L12:
	cmpq	%rdi, %rax
	je	.L14
	movq	8(%rax), %rsi
	movq	%rsi, 8(%rdi)
	movq	%rdi, (%rsi)
	movq	%rdi, index_sched_threads(%rip)
	jmp	.L5
.L14:
	movq	$0, index_sched_threads(%rip)
	jmp	.L5
	.size	timer_isr, .-timer_isr
	.ident	"GCC: (Ubuntu 12-20220319-1ubuntu1) 12.0.1 20220319 (experimental) [master r12-7719-g8ca61ad148f]"
	.section	.note.GNU-stack,"",@progbits
