#ifndef TITLESTATE_H_INCLUDED
#define TITLESTATE_H_INCLUDED

void titlestate_init(void);

void titlestate_joyevent(unsigned short joy, unsigned short changed, unsigned short state);

void titlestate_update(long dt);

void titlestate_stop(void);

#endif //TITLESTATE_H_INCLUDED
