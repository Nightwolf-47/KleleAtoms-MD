#ifndef TUTORIALSTATE_H_INCLUDED
#define TUTORIALSTATE_H_INCLUDED
#include <types.h>

void tutorialstate_init(void);

void tutorialstate_update(fix32 dt);

void tutorialstate_joyevent(u16 joy, u16 changed, u16 state);

void tutorialstate_stop(void);

#endif //TUTORIALSTATE_H_INCLUDED
