/*
 * multitasking.c
 *
 * Created: 26/05/2016 13:55:47
 * Author : paul.qureshi
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "tasks.h"


void task0(void) __attribute__ ((noreturn));
void task1(void) __attribute__ ((noreturn));
void task2(void) __attribute__ ((noreturn));


/******************************************************************************
** Task 0, replaces the main()'s stack.
*/
void task0(void)
{
	for(;;)
	{
		PORTA.OUTTGL = PIN0_bm;
		TASK_wait(1, ticks_to_ms(200));
	}
}


/******************************************************************************
** Task 1
*/
void task1(void)
{
	for(;;)
	{
		PORTA.OUTTGL = PIN1_bm;
		TASK_wait(1, ticks_to_ms(400));
	}
}


/******************************************************************************
** Task 2
*/
void task2(void)
{
	for(;;)
	{
		PORTA.OUTTGL = PIN2_bm;
		TASK_wait(1, ticks_to_s(1));
	}
}


/******************************************************************************
** Main entry point, also task 0
*/
int main(void)
{
	PORTA.DIRSET = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm;
	TASK_init();
	TASK_create(task1, 1);
	TASK_create(task2, 2);

	PORTA.OUTSET = PIN0_bm;
	_delay_ms(1000);
	PORTA.OUTCLR = PIN0_bm;
	_delay_ms(1000);

  sei();
	
	task0();
}

void TASK_stack_smash() {
	for(;;) {
			PORTA.OUTTGL = PIN3_bm;
			_delay_ms(200);
	}
}