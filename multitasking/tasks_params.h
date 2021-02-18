#ifndef TASKS_PARAMS_H_
#define TASKS_PARAMS_H_

// Number of allocable tasks. At most 8
#define NUM_TASKS	8

// Size of each task stack
#define STACK_SIZE	200

// Canary byte for task smash detector
#define MAGIC_NUMBER 0xA5

#endif