#include "titlestate.h"
#include "../../data.h"
#include "../../../res/resources.h"

Image* titleScreen[2]; //Uncompressed title screen image part data

VidImagePtr vidImgTTS[2]; //Title screen image parts ready for drawing

fix32 demoTimer = 0; //If it reaches demoStartTime, a demo plays

const fix32 demoStartTime = FIX32(30); //Time after which a demo plays

void titlestate_init(void)
{
    demoTimer = 0;
    isDemoPlaying = FALSE;
    titleScreen[0] = unpackImage(&texTitleScr0,NULL);
    titleScreen[1] = unpackImage(&texTitleScr1,NULL);
    if(!titleScreen[0] || !titleScreen[1])
    {
        SYS_die("Could not load title screen!");
    }
    vidImgTTS[0] = reserveVImage(titleScreen[0]);
    vidImgTTS[1] = reserveVImage(titleScreen[1]);
    VDP_setTextPalette(PAL2);
    memcpy(newPalette,titleScreen[0]->palette->data,sizeof(u16)*titleScreen[0]->palette->length);
    memcpy(&newPalette[16],titleScreen[1]->palette->data,sizeof(u16)*titleScreen[1]->palette->length);
    newPalette[47] = RGB24_TO_VDPCOLOR(0xEEEEEE);
    VDP_drawImageEx(BG_B,vidImgTTS[0]->img,TILE_ATTR_FULL(PAL0,0,FALSE,FALSE,vidImgTTS[0]->vPos),0,0,FALSE,TRUE);
    VDP_drawImageEx(BG_B,vidImgTTS[1]->img,TILE_ATTR_FULL(PAL1,0,FALSE,FALSE,vidImgTTS[1]->vPos),26,0,FALSE,TRUE);
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
    for(int i=0; i<2; i++)
    {
        if(titleScreen[i])
        {
            MEM_free(titleScreen[i]);
            titleScreen[i] = NULL;
        }
    }
    //Change text color palette back to PAL0
    VDP_setTextPalette(PAL0);
}
