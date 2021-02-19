/**
 * @file tasks_asm.S
 * @author paul.qureshi
 * @author giuliof (giulio@glgprograms.it)
 * @brief
 * @version 1.0
 * @date 2021-02-19
 * 
 * @copyright GNU General Public License v3.0
 * 
 */ 

#include <avr/io.h>

#include "tasks_params.h"

.extern	task_enable_mask_AT
.extern	task_stack_ptr
.extern	task_index
.extern	task_index_mask
.extern	TASK_STACK_BASE
 
.section .text

.global TASK_yield
.global TASK_load

.weak TASK_stack_smash

/******************************************************************************
** Task switching. Here, interrupts must be enabled or no task will be woke up
*/
TASK_yield:
	// old task idx
	push	r19
	// task_enable_mask_AT
	push	r18
	// new task_idx
	push	r17
	// task_index_mask (new)
	push	r16
	push	zl
	push	zh
	
	// deep power saving sleep while no tasks enabled (must be woken by by interrupts)
idle_loop:
	lds		r18, task_enable_mask_AT
	tst		r18
	breq	idle_loop

	// find next task
	lds		r17, task_index
	mov		r19, r17		// save last task_index for later
find_next_task:
	lds		r16, task_index_mask
	lsl		r16
	inc		r17
	cpi		r17, NUM_TASKS
	brlt	no_wrap
	ldi		r17, 0			// task_index
	ldi		r16, 1			// task_index_mask
no_wrap:
	sts		task_index, r17
	sts		task_index_mask, r16
	
	and		r16, r18		// & task_enable_mask_AT
	breq	find_next_task	// not enabled
	
	// Found a task

	// 1) check if same task, avoiding some context switch overhead
	cp		r17, r19
	brne	not_same_task
	jmp		same_task

not_same_task:
	// 2) save current context
	in		r16, CPU_EIND
	push	r16
	in		r16, CPU_RAMPZ
	push	r16
	in		r16, CPU_RAMPY
	push	r16
	in		r16, CPU_RAMPX
	push	r16
	in		r16, CPU_RAMPD
	push	r16

	//push	r31		// zh
	//push	r30		// zl
	push	r29
	push	r28
	push	r27
	push	r26
	push	r25
	push	r24
	push	r23
	push	r22
	push	r21
	push	r20
	//push	r19
	//push	r18
	//push	r17
	//push	r16
	push	r15
	push	r14
	push	r13
	push	r12
	push	r11
	push	r10
	push	r9
	push	r8
	push	r7
	push	r6
	push	r5
	push	r4
	push	r3
	push	r2
	push	r1
	push	r0
	
	// 3) check stack smash
	ldi		zl, lo8(TASK_STACK_BASE)
	ldi		zh, hi8(TASK_STACK_BASE)
	ldi		r16, MAGIC_NUMBER
	mov		r15, r19	; copy task id
	inc		r15			; get near task id
	lsl		r15			; task index * 2
	add		zl, r15
	adc		zh, r1		; GGC keeps r1 always 0
	ld		xl, z
	ldd		xh, z+1
	ld		r15, X
	cpse	r15, r16
	call	TASK_stack_smash
	
	// save current task's stack pointer
	ldi		zl, lo8(task_stack_ptr)
	ldi		zh, hi8(task_stack_ptr)
	lsl		r19				// old task index * 2
	add		zl, r19
	adc		zh, r1			// GGC keeps r1 always 0
	cli
	in		r16, CPU_SPL
	st		Z+, r16
	in		r16, CPU_SPH
	sei
	st		Z, r16

	// load next tasks stack point into r16
	ldi		zl, lo8(task_stack_ptr)
	ldi		zh, hi8(task_stack_ptr)
	lsl		r17				// new task in index * 2
	add		zl, r17
	adc		zh, r1			// GGC keeps r1 always 0
	ld		r16, Z+
	cli
	out		CPU_SPL, r16
	ld		r16, Z
	out		CPU_SPH, r16
	sei

	pop		r0
	pop		r1
	pop		r2
	pop		r3
	pop		r4
	pop		r5
	pop		r6
	pop		r7
	pop		r8
	pop		r9
	pop		r10
	pop		r11
	pop		r12
	pop		r13
	pop		r14
	pop		r15
	//pop		r16
	//pop		r17
	//pop		r18
	//pop		r19
	pop		r20
	pop		r21
	pop		r22
	pop		r23
	pop		r24
	pop		r25
	pop		r26
	pop		r27
	pop		r28
	pop		r29
	//pop		r30	// zh
	//pop		r31	// zl
	
	pop		r16
	out		CPU_RAMPD, r16
	pop		r16
	out		CPU_RAMPX, r16
	pop		r16
	out		CPU_RAMPY, r16
	pop		r16
	out		CPU_RAMPZ, r16
	pop		r16
	out		CPU_EIND, r16
	
same_task:
	pop		zh
	pop		zl
	pop		r16
	pop		r17
	pop		r18
	pop		r19
	ret


/******************************************************************************
** Task creation
** task pointer in r24:r25
** task index in r22
*/
TASK_load:
	; push	r22
	push	r17
	push	r16
	push	zh
	push	zl
	push	yh
	push	yl
	push	xh
	push	xl

	//// load task stack base vector in X
	ldi		yl, lo8(TASK_STACK_BASE)
	ldi		yh, hi8(TASK_STACK_BASE)

	// load task stack pointer vector in Z
	ldi		zl, lo8(task_stack_ptr)
	ldi		zh, hi8(task_stack_ptr)
	
	// move vectors to current task (+2*r22)
	lsl		r22				// new task in index * 2
	add		zl, r22
	adc		zh, r1			// GGC keeps r1 always 0
	add		yl, r22
	adc		yh, r1			// GGC keeps r1 always 0
	
	// load current tasks stack pointer in X
	ld		xl, y
	ldd		xh, y+1
	
	//// skip one byte for MAGIC_NUMBER
	sbiw	xl, 1
	//st		z, xl
	//std		z+1, xh
	
	// temporarily load new task's stack pointer
	// Avoid turning on interrupts if not requested
	in		r22, SREG		; save current status register (w/ I flag)
	cli
	in		r16, CPU_SPL
	in		r17, CPU_SPH
	out		CPU_SPL, xl
	out		CPU_SPH, xh
	out		SREG, r22		; restore status register
	
	// Some devices have more than 128 KiB of program memory.
	// This also means that the program counter (PC) is 3 bytes wide. 
	push	r24
	push	r25
	#ifdef __AVR_3_BYTE_PC__
	// I'm assuming that task routine is relocated in internal flash,
	// so I'm pushing 0 as MSB
	push	r1
	#endif
	
	// clear all registers that will be reloaded from stack
	push	r1 ; r19
	push	r1 ; r18
	push	r1 ; r17
	push	r1 ; r16
	push	r1 ; zl
	push	r1 ; zh
	
	push	r1 ; CPU_EIND
	push	r1 ; CPU_RAMPZ
	push	r1 ; CPU_RAMPY
	push	r1 ; CPU_RAMPX
	push	r1 ; CPU_RAMPD

	push	r1 ; r29
	push	r1 ; r28
	push	r1 ; r27
	push	r1 ; r26
	push	r1 ; r25
	push	r1 ; r24
	push	r1 ; r23
	push	r1 ; r22
	push	r1 ; r21
	push	r1 ; r20
	push	r1 ; r15
	push	r1 ; r14
	push	r1 ; r13
	push	r1 ; r12
	push	r1 ; r11
	push	r1 ; r10
	push	r1 ; r9
	push	r1 ; r8
	push	r1 ; r7
	push	r1 ; r6
	push	r1 ; r5
	push	r1 ; r4
	push	r1 ; r3
	push	r1 ; r2
	push	r1 ; r1
	push	r1 ; r0

	// save new task stack pointer
	// Avoid turning on interrupts if not requested
	in		r22, SREG		; save current status register (w/ I flag)
	cli
	in		xl, CPU_SPL
	in		xh, CPU_SPH
	st		z, xl
	std		z+1, xh

	// restore current task's stack pointer
	out		CPU_SPL, r16
	out		CPU_SPH, r17
	out		SREG, r22		; restore status register
	
	// store MAGIC_NUMBER in next task's stack
	ldi		r16, MAGIC_NUMBER
	adiw	yl, 2
	ld		xl, y
	ldd		xh, y+1
	st		x, r16
	
	pop		xl
	pop		xh
	pop		yl
	pop		yh
	pop		zl
	pop		zh
	pop		r16
	pop		r17
	; pop		r22
	ret

/******************************************************************************
** Stack smash routine. Weak symbol, may me overwritten.
*/
TASK_stack_smash:
	; Trace informations
	; r19 - old task idx
	; r18 - task_enable_mask_AT
	; r17 - new task idx
	rjmp TASK_stack_smash
