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
	movl	$2051, %ecx
	pushq	%rdx
	pushq	%rax
#APP
# 70 "../public.h" 1
	rdmsr
# 0 "" 2
#NO_APP
	movq	running_threads(%rip), %rsi
	movl	%eax, %edx
	movq	(%rsi,%rdx,8), %rdi
#APP
# 18 "timer_isr.c" 1
	movq  $1, %rax
.Ltsl_lock17:
	xchgq %rax, sched_threads_mutex(%rip)
	testq %rax, %rax
	jnz   .Ltsl_lock17
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
	popq	%rbp
	iretq
.L2:
	movq	(%rax), %rcx
	leaq	-65536(%rax), %rbx
	testq	%rdi, %rdi
	je	.L12
#APP
# 47 "timer_isr.c" 1
	pushq  %r12
	pushq  %r13
	pushq  %r14
	pushq  %r15
	pushq  %r8
	pushq  %r9
	pushq  %r10
	pushq  %r11
	movq   %rsp, 65552(%rdi)
# 0 "" 2
#NO_APP
	movq	return_handler_timer_int@GOTPCREL(%rip), %rbp
	movq	%rbp, 65560(%rdi)
	leaq	65536(%rdi), %rbp
	cmpq	%rcx, %rax
	je	.L13
	movq	%rcx, 65536(%rdi)
	movq	%rbp, 8(%rcx)
	movq	8(%rax), %rax
	movq	%rax, 65544(%rdi)
	movq	%rbp, (%rax)
	movq	%rcx, index_sched_threads(%rip)
.L7:
	movq	65568(%rbx), %rax
#APP
# 80 "timer_isr.c" 1
	movq   %cr3, %rdi
	cmpq   %rdi, %rax
	je     1f
	movq   %rax, %cr3
1:
# 0 "" 2
#NO_APP
.L5:
#APP
# 91 "timer_isr.c" 1
	movq  $0, sched_threads_mutex(%rip)
# 0 "" 2
#NO_APP
	movq	%rdx, %rax
	movq	%rbx, (%rsi,%rdx,8)
	leaq	65536(%rbx), %rdx
	salq	$7, %rax
	addq	tsss(%rip), %rax
	movq	%rdx, 4(%rax)
#APP
# 16 "../sched/switch.h" 1
	movq   65552(%rbx), %rsp
	jmpq   *65560(%rbx)
# 0 "" 2
#NO_APP
.L12:
	cmpq	%rcx, %rax
	je	.L4
	movq	8(%rax), %rax
	movq	%rcx, %rdi
	movq	%rax, 8(%rcx)
	movq	%rcx, (%rax)
.L4:
	movq	%rdi, index_sched_threads(%rip)
	movq	65568(%rbx), %rax
#APP
# 38 "timer_isr.c" 1
	movq   %rax, %cr3
# 0 "" 2
#NO_APP
	jmp	.L5
.L13:
	movq	%rbp, index_sched_threads(%rip)
	movq	%rbp, 65544(%rdi)
	movq	%rbp, 65536(%rdi)
	jmp	.L7
	.size	timer_isr, .-timer_isr
	.ident	"GCC: (Ubuntu 12-20220319-1ubuntu1) 12.0.1 20220319 (experimental) [master r12-7719-g8ca61ad148f]"
	.section	.note.GNU-stack,"",@progbits
