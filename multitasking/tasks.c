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
#include "tasks_params.h"

static void systick_init();

#define is_task_enabled(i) (task_enable_mask_AT & _BV(i))

uint8_t		task_enable_mask_AT = 0;		// written by interrupts!
// TODO: init all to zero?
// keeps the mask of the event that can wake the task
static uint8_t   task_event_mask[NUM_TASKS];
// keeps the mask of the event that woke the task
static uint8_t   task_event_id[NUM_TASKS];
// systick-driven counter for timed events
static volatile uint16_t  task_timeout[NUM_TASKS];
uint8_t* 	task_stack_ptr[NUM_TASKS];
uint8_t		task_index = 0;
uint8_t		task_index_mask = 1;

uint8_t* const TASK_STACK_BASE[] = {
	// Task 0 (aka main). Size can be trimmed
	(uint8_t*)RAMEND - (STACK_SIZE * 0),
	// Task 1-7. Size is STACK_SIZE
	(uint8_t*)RAMEND - (STACK_SIZE * 1),
	(uint8_t*)RAMEND - (STACK_SIZE * 2),
	(uint8_t*)RAMEND - (STACK_SIZE * 3),
	(uint8_t*)RAMEND - (STACK_SIZE * 4),
	(uint8_t*)RAMEND - (STACK_SIZE * 5),
	(uint8_t*)RAMEND - (STACK_SIZE * 6),
	(uint8_t*)RAMEND - (STACK_SIZE * 7),
	// Task 8 does not exist. This is only a placeholder for last stacksmash
	// detector
	(uint8_t*)RAMEND - (STACK_SIZE * 8)};

/******************************************************************************
** Set up task switching system. Interrupt safe.
*/
void TASK_init(void)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		// for (uint8_t i = 0; i < NUM_TASKS; i++) {
		// 	task_stack_ptr[i] = TASK_STACK_BASE[i] - 1;
    //   *TASK_STACK_BASE[i] = MAGIC_NUMBER;
    // }
    // Set canary byte for task 0
    *TASK_STACK_BASE[1] = MAGIC_NUMBER;
    // enable task 0
		task_enable_mask_AT |= 1;
	}
  
  systick_init();
}

/******************************************************************************
** Create a new task. Interrupt safe.
*/
void TASK_create(void (* task_pointer)(), const uint8_t task_number)
{
	if (task_number > NUM_TASKS)
    return;
  
  // reserve 1byte for MAGIC_NUMBER
  task_stack_ptr[task_number] = TASK_STACK_BASE[task_number] - 1;
  // (re-)write magic number at next task
  *TASK_STACK_BASE[task_number + 1] = MAGIC_NUMBER;

  TASK_load(task_pointer, task_number);
  TASK_enable(task_number);
}

/******************************************************************************
** Enable a task. Interrupt safe. May be called inside ISR.
*/
void TASK_enable(const uint8_t task_number)
{
  if (task_number > NUM_TASKS)
    return;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		task_enable_mask_AT |= 1 << task_number;
	}
}

/******************************************************************************
** Disable a task. Interrupt safe. May be called inside ISR.
*/
void TASK_disable(const uint8_t task_number)
{
  if (task_number > NUM_TASKS)
    return;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		task_enable_mask_AT &= ~(1 << task_number);
	}
}

/******************************************************************************
** Disable the task waiting for an event mask. Interrupt safe. May be called
** inside ISR.
*/
uint8_t TASK_wait(const uint8_t event_mask, const uint16_t timeout) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    task_event_mask[task_index] = event_mask;
    task_timeout[task_index] = timeout;
    // Disable the task
		task_enable_mask_AT &= ~(1 << task_index);
	}
  TASK_yield();

  // return
  return task_event_id[task_index];
}

/******************************************************************************
** Raise an event addressed to a specified task. Interrupt safe. May be called
** inside ISR.
*/
void TASK_signal(const uint8_t task_number, const uint8_t event_mask) {
  if (task_number > NUM_TASKS)
    return;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		if (!is_task_enabled(task_number) && (task_event_mask[task_number] & event_mask)) {
      // Wake up the task and save the signalling event
  		task_enable_mask_AT |= 1 << task_number;
      task_event_id[task_number] = event_mask;
    }
	}
}

static void systick_init() {
  // Set up in byte mode (CNTH = 0)
  //~ TCC0.CTRLE = TC_BYTEM_BYTEMODE_gc;
  // normal mode (no pwm or other)
  TCC0.CTRLB = 0x00;

  // generate a HI-lvl interrupt when overflow
  TCC0.INTCTRLA = TC_OVFINTLVL_HI_gc;

  PMIC.CTRL |= PMIC_HILVLEN_bm;

  TCC0.CNT = 0;
  TCC0.PER = F_CPU * TICKS_MS / TPRESCALER / 1000;

  #if TPRESCALER == 1
    TCC0.CTRLA = TC_CLKSEL_DIV1_gc;
  #elif TPRESCALER == 2
    TCC0.CTRLA = TC_CLKSEL_DIV2_gc;
  #elif TPRESCALER == 4
    TCC0.CTRLA = TC_CLKSEL_DIV4_gc;
  #elif TPRESCALER == 8
    TCC0.CTRLA = TC_CLKSEL_DIV8_gc;
  #elif TPRESCALER == 64
    TCC0.CTRLA = TC_CLKSEL_DIV64_gc;
  #elif TPRESCALER == 256
    TCC0.CTRLA = TC_CLKSEL_DIV256_gc;
  #elif TPRESCALER == 1024
    TCC0.CTRLA = TC_CLKSEL_DIV1024_gc;
  #else
    #error "Wrong definition of TPRESCALER"
  #endif
}

// Systick routine
ISR(TCC0_OVF_vect) {
  for (uint8_t i = 0; i < NUM_TASKS; i++) {
    // Ignore tasks not waiting for timeout events
    if (!(task_event_mask[i] & 1))
      continue;
    // Ignore wake tasks
    if (is_task_enabled(i))
      continue;

    // Timeout, time to wake up task
    if (task_timeout[i] == 0) {
      //TASK_signal(i, 1)
      task_enable_mask_AT |= 1 << i;
      task_event_id[i] = 1;
    }
    else
      task_timeout[i]--;
  }
}