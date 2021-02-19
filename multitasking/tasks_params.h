/**
 * @file tasks_params.h
 * @author paul.qureshi
 * @author giuliof (giulio@glgprograms.it)
 * @brief shared defines between tasks.c and tasks_asm.S
 * @version 1.0
 * @date 2021-02-19
 * 
 * @copyright GNU General Public License v3.0
 * 
 */ 

#define TASKS_PARAMS_H_

// Number of allocable tasks. At most 8
#define NUM_TASKS	8

// Size of each task stack
#define STACK_SIZE	200

// Canary byte for task smash detector
#define MAGIC_NUMBER 0xA5

#endif