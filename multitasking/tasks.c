/*
 * tasks.c
 *
 * Created: 26/05/2016 13:57:20
 *  Author: paul.qureshi
 *
 * Note: Task 0's stack should begin where GCC's stack does by default, i.e. RAMEND.
 * This allows main() to become task 0.
 */ 

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdbool.h>
#include "tasks.h"


#define STACK_SIZE	200
#define NUM_TASKS	8

uint8_t		task_enable_mask_AT = 0;		// written by interrupts!
uint8_t		task_sleep_mask_AT = 0;			// written by interrupts!
uint16_t	task_stack_ptr[NUM_TASKS];
uint8_t		task_index = 0;
uint8_t		task_index_mask = 1;


/******************************************************************************
** Set up task switching system. Interrupt safe.
*/
void TASK_init(void)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		for (uint8_t i = 0; i < NUM_TASKS; i++)
			task_stack_ptr[i] = RAMEND - (STACK_SIZE * i);

		task_enable_mask_AT |= 1;	// enable task 0
	}
}

/******************************************************************************
** Create a new task. Interrupt safe.
*/
void TASK_create(uint16_t task_pointer, uint8_t task_number)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		task_stack_ptr[task_number] = RAMEND - (STACK_SIZE * task_number);
		TASK_load(task_pointer, task_number);
		task_enable_mask_AT |= 1 << task_number;
	}
}

/******************************************************************************
** Enable a task. Interrupt safe. May be called inside ISR.
*/
void TASK_enable(uint8_t task_number)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		task_enable_mask_AT |= 1 << task_number;
	}
}

/******************************************************************************
** Disable a task. Interrupt safe. May be called inside ISR.
*/
void TASK_disable(uint8_t task_number)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		task_enable_mask_AT &= ~(1 << task_number);
	}
}

/******************************************************************************
** Sleep task. Interrupt safe. May be called inside ISR.
*/
void TASK_sleep(uint8_t task_number)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		task_sleep_mask_AT |= 1 << task_number;
	}
}

/******************************************************************************
** Wake task. Interrupt safe. May be called inside ISR.
*/
void TASK_wake(uint8_t task_number)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		task_sleep_mask_AT &= ~(1 << task_number);
	}
}
