/*
 * tasks.h
 *
 * Created: 26/05/2016 13:57:40
 *  Author: paul.qureshi
 */ 


#ifndef TASKS_H_
#define TASKS_H_


extern void TASK_init(void);
void TASK_create(uint16_t task_pointer, uint8_t task_number, bool start);

// tasks_asm.S
extern void TASK_yield(void);
extern void TASK_load(uint16_t task_ptr, uint8_t task_number);

void TASK_enable(uint8_t task_number);
void TASK_disable(uint8_t task_number);
void TASK_sleep(uint8_t task_number);
void TASK_wake(uint8_t task_number);

#endif /* TASKS_H_ */
