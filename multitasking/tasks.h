/**
 * @file tasks.h
 * @author paul.qureshi
 * @author giuliof (giulio@glgprograms.it)
 * @brief
 * @version 1.0
 * @date 2021-02-19
 * 
 * @copyright GNU General Public License v3.0
 * 
 */

#ifndef TASKS_H_
#define TASKS_H_

#include <stdbool.h>

extern void TASK_init(void);
void TASK_create(void (* task_pointer)(), const uint8_t task_number);

// tasks_asm.S
extern void TASK_yield(void);
extern void TASK_load(void (* task_pointer)(), const uint8_t task_number);

void TASK_enable(const uint8_t task_number);
void TASK_disable(const uint8_t task_number);
uint8_t TASK_wait(const uint8_t event_mask, const uint16_t timeout);
void TASK_signal(const uint8_t task_number, const uint8_t event_mask);

#define ticks_to_s(s) ((s) * 1000 / TICKS_MS)
#define ticks_to_ms(ms) ((ms) / TICKS_MS)

#endif /* TASKS_H_ */
