# xmultitasking
Cooperative multitasking for XMEGA micros, easily adaptable to others.

- Non-preemptive: tasks must yield control themselves;
- `wait`/`signal` mechanism for task syncronisation and event detection;

## Why
I needed a light and simple xmega multitasking system to extend an old project.

## How to

An example is provided in `main.c`.

    cd multitasking
    make
    make flash

Note: change `PRGFLAGS` variabile in makefile according to your AVR programmer.
See avrdude documentation for further informations.