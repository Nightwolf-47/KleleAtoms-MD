#include "titlestate.h"
#include "../../data.h"
#include "../../../res/resources.h"

Image* titleScreen; //Uncompressed title screen image data

VidImagePtr vidImgTTS; //Title screen image ready for drawing

fix32 demoTimer = 0; //If it reaches demoStartTime, a demo plays

const fix32 demoStartTime = FIX32(30); //Time after which a demo plays

void titlestate_init(void)
{
    demoTimer = 0;
    isDemoPlaying = FALSE;
    titleScreen = unpackImage(&texTitleScr,NULL);
    if(!titleScreen)
    {
        SYS_die("Could not load title screen!",NULL);
    }
    vidImgTTS = reserveVImage(titleScreen,FALSE);
    VDP_setTextPalette(PAL2);
    memcpy(newPalette,titleScreen->palette->data,sizeof(u16)*titleScreen->palette->length);
    newPalette[47] = RGB24_TO_VDPCOLOR(0xEEEEEE);
    memcpy(&newPalette[48],sprCursor.palette->data,sizeof(u16)*sprCursor.palette->length);
    VDP_drawImageEx(BG_B,vidImgTTS->img,TILE_ATTR_FULL(PAL0,0,FALSE,FALSE,vidImgTTS->vPos),0,0,FALSE,TRUE);
    VDP_drawText("Press any button to continue",2,5);
}

void titlestate_update(fix32 dt)
{
    demoTimer += dt;
    if(demoTimer>=demoStartTime) //Play a demo if idle for 30 seconds
    {
        isDemoPlaying = TRUE;
        changeState(ST_GAMESTATE);
    }
}

void titlestate_joyevent(u16 joy, u16 changed, u16 state)
{
    if(state & changed)
    {
        changeState(ST_MENUSTATE);
    }
}

void titlestate_stop(void)
{
    if(titleScreen)
    {
        MEM_free(titleScreen);
        titleScreen = NULL;
    }
    //Change text color palette back to PAL0
    VDP_setTextPalette(PAL0);
}
