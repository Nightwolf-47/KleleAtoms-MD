#ifndef MENUSTATE_H_INCLUDED
#define MENUSTATE_H_INCLUDED
#include <types.h>

void menustate_init(void);

void menustate_update(fix32 dt);

void menustate_joyevent(u16 joy, u16 changed, u16 state);

void menustate_stop(void);

#endif //MENUSTATE_H_INCLUDED
