#ifndef TITLESTATE_H_INCLUDED
#define TITLESTATE_H_INCLUDED
#include <types.h>

void titlestate_init(void);

void titlestate_joyevent(u16 joy, u16 changed, u16 state);

void titlestate_update(fix32 dt);

void titlestate_stop(void);

#endif //TITLESTATE_H_INCLUDED
