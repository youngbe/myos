	.file	"timer_isr.c"
	.text
	.p2align 4
	.globl	timer_isr
	.type	timer_isr, @function
timer_isr:
	pushq	%rbp
	pushq	%rdi
	pushq	%rsi
	pushq	%rbx
	pushq	%rcx
	movl	$2050, %ecx
	pushq	%rdx
	pushq	%rax
#APP
# 93 "kernel/public.h" 1
	rdmsr
# 0 "" 2
#NO_APP
	movq	running_threads(%rip), %rdi
	movl	%eax, %edx
	movl	$1, %esi
	movq	(%rdi,%rdx,8), %rcx
#APP
# 17 "kernel/sched/timer_isr.c" 1
	.Ltsl_lock18:
	xorq  %rax, %rax
	cmpxchgq %rsi, sched_threads_mutex(%rip)
	jne   .Ltsl_lock18
# 0 "" 2
#NO_APP
	movq	index_sched_threads(%rip), %rax
	testq	%rax, %rax
	jne	.L2
#APP
# 21 "kernel/sched/timer_isr.c" 1
	movq  $0, sched_threads_mutex(%rip)
# 0 "" 2
#NO_APP
	popq	%rax
	popq	%rdx
	popq	%rcx
	popq	%rbx
	popq	%rsi
	popq	%rdi
	popq	%rbp
	iretq
.L2:
	movq	(%rax), %rsi
	leaq	-65536(%rax), %rbx
	testq	%rcx, %rcx
	je	.L12
#APP
# 46 "kernel/sched/timer_isr.c" 1
	pushq  %r12
	pushq  %r13
	pushq  %r14
	pushq  %r15
	pushq  %r8
	pushq  %r9
	pushq  %r10
	pushq  %r11
	movq   %rsp, 65552(%rcx)
# 0 "" 2
#NO_APP
	movq	return_handler_timer_int@GOTPCREL(%rip), %rbp
	movq	%rbp, 65560(%rcx)
	leaq	65536(%rcx), %rbp
	cmpq	%rsi, %rax
	je	.L13
	movq	%rsi, 65536(%rcx)
	movq	%rbp, 8(%rsi)
	movq	8(%rax), %rax
	movq	%rax, 65544(%rcx)
	movq	%rbp, (%rax)
	movq	%rsi, index_sched_threads(%rip)
.L7:
	movq	65568(%rbx), %rax
#APP
# 79 "kernel/sched/timer_isr.c" 1
	movq   %cr3, %rsi
	cmpq   %rsi, %rax
	je     1f
	movq   %rax, %cr3
1:
# 0 "" 2
#NO_APP
.L5:
#APP
# 90 "kernel/sched/timer_isr.c" 1
	movq  $0, sched_threads_mutex(%rip)
# 0 "" 2
#NO_APP
	movq	%rdx, %rax
	movq	%rbx, (%rdi,%rdx,8)
	leaq	65536(%rbx), %rdx
	salq	$7, %rax
	addq	tsss(%rip), %rax
	movq	%rdx, 4(%rax)
#APP
# 15 "kernel/sched/switch.h" 1
	movq   65552(%rbx), %rsp
	jmpq   *65560(%rbx)
# 0 "" 2
#NO_APP
.L12:
	cmpq	%rsi, %rax
	je	.L4
	movq	8(%rax), %rax
	movq	%rsi, %rcx
	movq	%rax, 8(%rsi)
	movq	%rsi, (%rax)
.L4:
	movq	%rcx, index_sched_threads(%rip)
	movq	65568(%rbx), %rax
#APP
# 37 "kernel/sched/timer_isr.c" 1
	movq   %rax, %cr3
# 0 "" 2
#NO_APP
	jmp	.L5
.L13:
	movq	%rbp, index_sched_threads(%rip)
	movq	%rbp, 65544(%rcx)
	movq	%rbp, 65536(%rcx)
	jmp	.L7
	.size	timer_isr, .-timer_isr
	.ident	"GCC: (Ubuntu 12-20220319-1ubuntu1) 12.0.1 20220319 (experimental) [master r12-7719-g8ca61ad148f]"
	.section	.note.GNU-stack,"",@progbits
