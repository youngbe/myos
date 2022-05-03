	.file	"semaphore_down.c"
	.text
	.p2align 4
	.globl	semaphore_down
	.type	semaphore_down, @function
semaphore_down:
	movq	%rdi, %rsi
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
	jne	.L8
	movl	$2051, %ecx
#APP
# 70 "../public.h" 1
	rdmsr
# 0 "" 2
#NO_APP
	movq	running_threads(%rip), %r8
	movl	%eax, %edx
	movq	return_handler_function@GOTPCREL(%rip), %rcx
	movq	(%r8,%rdx,8), %rax
	movq	%rcx, 65560(%rax)
	leaq	16(%rdi), %rcx
	movq	65616(%rax), %rdi
	movq	%rcx, 8(%rdi)
	movq	%rdi, 16(%rsi)
	leaq	65616(%rax), %rdi
	movq	%rdi, 24(%rsi)
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
.Ltsl_lock27:
	xchgq %rax, sched_threads_mutex(%rip)
	testq %rax, %rax
	jnz   .Ltsl_lock27
# 0 "" 2
#NO_APP
	movq	index_sched_threads(%rip), %rax
	testq	%rax, %rax
	je	.L9
	movq	(%rax), %rcx
	cmpq	%rax, %rcx
	je	.L10
	movq	8(%rax), %rdi
	movq	%rcx, index_sched_threads(%rip)
	movq	%rdi, 8(%rcx)
	movq	%rcx, (%rdi)
.L5:
#APP
# 73 "semaphore_down.c" 1
	movq  $0, sched_threads_mutex(%rip)
# 0 "" 2
#NO_APP
	leaq	-65536(%rax), %rcx
	movq	65568(%rcx), %rdi
#APP
# 77 "semaphore_down.c" 1
	movq   %cr3, %r9
	cmpq   %r9, %rdi
	je     1f
	movq   %rdi, %cr3
1:
# 0 "" 2
# 87 "semaphore_down.c" 1
	movq  $0, (%rsi)
# 0 "" 2
#NO_APP
	movq	%rcx, (%r8,%rdx,8)
	salq	$7, %rdx
	addq	tsss(%rip), %rdx
	movq	%rax, 4(%rdx)
#APP
# 16 "../sched/switch.h" 1
	movq   65552(%rcx), %rsp
	jmpq   *65560(%rcx)
# 0 "" 2
	.p2align 4,,10
	.p2align 3
#NO_APP
.L8:
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
.L10:
	movq	$0, index_sched_threads(%rip)
	jmp	.L5
	.p2align 4,,10
	.p2align 3
.L9:
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
	movq  $0, (%rsi)
# 0 "" 2
#NO_APP
	movq	$0, (%r8,%rdx,8)
	addq	$1, %rdx
	salq	$12, %rdx
	addq	halt_stacks(%rip), %rdx
#APP
# 30 "../sched/switch.h" 1
	movq   %rdx, %rsp
	sti
1:
	hlt
	jmp    1b
# 0 "" 2
#NO_APP
	.size	semaphore_down, .-semaphore_down
	.ident	"GCC: (Ubuntu 12-20220319-1ubuntu1) 12.0.1 20220319 (experimental) [master r12-7719-g8ca61ad148f]"
	.section	.note.GNU-stack,"",@progbits
