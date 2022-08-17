#ifndef GAMESTATE_H_INCLUDED
#define GAMESTATE_H_INCLUDED

void gamestate_init(void);

void gamestate_update(long dt);

void gamestate_joyevent(unsigned short joy, unsigned short changed, unsigned short state);

void gamestate_stop(void);

extern long startTime; //In-game timer

#endif //GAMESTATE_H_INCLUDED
