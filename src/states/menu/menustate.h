#ifndef MENUSTATE_H_INCLUDED
#define MENUSTATE_H_INCLUDED

void menustate_init(void);

void menustate_update(long dt);

void menustate_joyevent(unsigned short joy, unsigned short changed, unsigned short state);

void menustate_stop(void);

#endif //MENUSTATE_H_INCLUDED
