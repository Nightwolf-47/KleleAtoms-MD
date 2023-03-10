#include "data.h"
#include "save.h"

void joyEventHandler(u16 joy, u16 changed, u16 state)
{
    if(states[currentState].joyevent && !PAL_isDoingFade())
        states[currentState].joyevent(joy,changed,state);
}

int main(bool hard)
{
    JOY_init();
    SPR_init();

    data_stateInit(); //Initialize game states
    data_initsfx(); //Initialize sounds

    JOY_setEventHandler(&joyEventHandler);
    if(hard)
    {
        data_init();
        loadSRAM(); //Load settings and savegame (if exists)
    }
    initState(ST_TITLESTATE);
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
