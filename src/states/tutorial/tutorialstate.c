#include "tutorialstate.h"
#include <genesis.h>
#include "../../data.h"
#include "../../../res/resources.h"

static VidImagePtr tutorialBGImg;

// Sets up tutorial palette colors
static void setupTutorialColors(bool oldColors)
{
    memcpy(newPalette,texTutorialBG.palette->data,sizeof(u16)*texTutorialBG.palette->length);
    memcpy(&newPalette[16],texTutorialBG.palette->data,sizeof(u16)*texTutorialBG.palette->length);
    memcpy(&newPalette[32],texTutorialBG.palette->data,sizeof(u16)*texTutorialBG.palette->length);
    memcpy(&newPalette[48],texTutorialBG.palette->data,sizeof(u16)*texTutorialBG.palette->length);
    //PAL0 (Red)
    if(oldColors)
    {
        newPalette[5] = RGB24_TO_VDPCOLOR(0xF80048);
        newPalette[6] = RGB24_TO_VDPCOLOR(0xC82448);
    }
    else
    {
        newPalette[5] = RGB24_TO_VDPCOLOR(0xEE0000);
        newPalette[6] = RGB24_TO_VDPCOLOR(0xCC2200);
    }

    //PAL1 (Blue)
    if(oldColors)
    {
        newPalette[21] = RGB24_TO_VDPCOLOR(0x00B4F8);
        newPalette[22] = RGB24_TO_VDPCOLOR(0x2090F8);
    }
    else
    {
        newPalette[21] = RGB24_TO_VDPCOLOR(0x0022EE);
        newPalette[22] = RGB24_TO_VDPCOLOR(0x0000EE);
    }

    //PAL2 (Green)
    if(oldColors)
    {
        newPalette[37] = RGB24_TO_VDPCOLOR(0x48FC00);
        newPalette[38] = RGB24_TO_VDPCOLOR(0x66CC22);
    }
    else
    {
        newPalette[37] = RGB24_TO_VDPCOLOR(0x00EE00);
        newPalette[38] = RGB24_TO_VDPCOLOR(0x00CC00);
    }

    //PAL3 (Yellow)
    if(oldColors)
    {
        newPalette[53] = RGB24_TO_VDPCOLOR(0xF8FC48);
        newPalette[54] = RGB24_TO_VDPCOLOR(0xEECC44);
    }
    else
    {
        newPalette[53] = RGB24_TO_VDPCOLOR(0xEECC00);
        newPalette[54] = RGB24_TO_VDPCOLOR(0xEEAA00);
    }
    newPalette[15] = RGB24_TO_VDPCOLOR(0x000000);
}

void tutorialstate_init(void)
{
    tutorialBGImg = reserveVImage(&texTutorialBG,TRUE);
    setupTutorialColors(settings.useOldColors);
    VDP_drawImageEx(BG_B,tutorialBGImg->img,TILE_ATTR_FULL(PAL0,0,0,0,tutorialBGImg->vPos),0,0,FALSE,FALSE);
}

void tutorialstate_update(fix32 dt)
{
    
}

void tutorialstate_joyevent(u16 joy, u16 changed, u16 state)
{
    if(state & changed)
    {
        changeState(ST_MENUSTATE);
    }
}

void tutorialstate_stop(void)
{
    
}