#ifndef ABOUTSTATE_H_INCLUDED
#define ABOUTSTATE_H_INCLUDED
#include <types.h>

void aboutstate_init(void);

void aboutstate_update(fix32 dt);

void aboutstate_joyevent(u16 joy, u16 changed, u16 state);

void aboutstate_stop(void);

#endif //ABOUTSTATE_H_INCLUDED
