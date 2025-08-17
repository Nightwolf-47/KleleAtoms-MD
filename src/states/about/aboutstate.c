#include "aboutstate.h"
#include "../../data.h"
#include "../../../res/resources.h"

static VidImagePtr aboutBGImg;

static void initializeAboutColors()
{
    memcpy(newPalette,texTutorialBG.palette->data,sizeof(u16)*texTutorialBG.palette->length);
    newPalette[15] = RGB24_TO_VDPCOLOR(0x000000);
}

void aboutstate_init(void)
{
    aboutBGImg = reserveVImage(&texTutorialBG,TRUE);
    initializeAboutColors();
    VDP_drawImageEx(BG_B,aboutBGImg->img,TILE_ATTR_FULL(PAL0,0,0,0,aboutBGImg->vPos),0,0,FALSE,FALSE);
}

void aboutstate_update(fix32 dt)
{
    ;
}

void aboutstate_joyevent(u16 joy, u16 changed, u16 state)
{
    if(state & changed)
    {
        changeState(ST_MENUSTATE);
    }
}

void aboutstate_stop(void)
{
    ;
}
