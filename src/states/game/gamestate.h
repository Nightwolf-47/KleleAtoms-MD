#ifndef GAMESTATE_H_INCLUDED
#define GAMESTATE_H_INCLUDED
#include <types.h>

void gamestate_init(void);

void gamestate_update(fix32 dt);

void gamestate_joyevent(u16 joy, u16 changed, u16 state);

void gamestate_stop(void);

extern fix32 startTime; //In-game timer

#endif //GAMESTATE_H_INCLUDED
