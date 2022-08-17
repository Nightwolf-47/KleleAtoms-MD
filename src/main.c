#include "data.h"
#include "save.h"
#include "states/game/gamestate.h"
#include "states/title/titlestate.h"
#include "states/menu/menustate.h"
#include <timer.h>

void joyEventHandler(u16 joy, u16 changed, u16 state)
{
    if(states[currentState].joyevent)
        states[currentState].joyevent(joy,changed,state);
}

int main(u16 hard)
{
    JOY_init();
    SPR_init();
    states[ST_GAMESTATE].init = &gamestate_init;
    states[ST_GAMESTATE].update = &gamestate_update;
    states[ST_GAMESTATE].joyevent = &gamestate_joyevent;
    states[ST_GAMESTATE].stop = &gamestate_stop;

    states[ST_TITLESTATE].init = &titlestate_init;
    states[ST_TITLESTATE].update = &titlestate_update;
    states[ST_TITLESTATE].joyevent = &titlestate_joyevent;
    states[ST_TITLESTATE].stop = &titlestate_stop;

    states[ST_MENUSTATE].init = &menustate_init;
    states[ST_MENUSTATE].update = &menustate_update;
    states[ST_MENUSTATE].joyevent = &menustate_joyevent;
    states[ST_MENUSTATE].stop = &menustate_stop;

    data_initsfx(); //Initialize sounds

    JOY_setEventHandler(&joyEventHandler);
    data_init();
    changeState(ST_TITLESTATE);
    fix32 dt = 1;
    fix32 totalTime = getTimeAsFix32(0);
    if(hard)
    {
        dt = totalTime; //DeltaTime is only equal to the total time right after a hard reset
    }
    while(TRUE)
    {
        if(randomNoPattern && !isDemoPlaying)
            random(); //Removes the pattern from actual random() calls
        if(states[currentState].update)
            states[currentState].update(dt);
        SPR_update();
        SYS_doVBlankProcess();
        fix32 newTime = getTimeAsFix32(0);
        dt = newTime - totalTime;
        totalTime = newTime;
    }
    return (0);
}
