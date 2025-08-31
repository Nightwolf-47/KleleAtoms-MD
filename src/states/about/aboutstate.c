#include "aboutstate.h"
#include "../../data.h"
#include "../../../res/resources.h"

static VidImagePtr aboutBGImg;

static void initializeAboutColors(void)
{
    memcpy(newPalette,texTutorialBG.palette->data,sizeof(u16)*texTutorialBG.palette->length);
    newPalette[15] = RGB24_TO_VDPCOLOR(0x000000);
    newPalette[31] = RGB24_TO_VDPCOLOR(0xEEEEEE);
    memcpy(&newPalette[48],sprCursor.palette->data,sizeof(u16)*sprCursor.palette->length);
}

static void drawAboutMenu(void)
{
    VDP_setTextPalette(PAL1);
    VDP_drawText("About the game",13,0);
    VDP_drawText("Press any button to leave.",7,27);
    VDP_setTextPalette(PAL0);
    char buf[40];
    sprintf(buf,"KleleAtoms MD %s",versionStr);
    VDP_drawText(buf,GETCENTERX(buf),2);
    VDP_drawText("MegaDrive port of KleleAtoms made",2,6);
    VDP_drawText("by Nightwolf-47 (Invictissimi).",2,8);
    VDP_drawText("Graphics made by greffmaster:",2,12);
    VDP_drawText("-Title screen",2,14);
    VDP_drawText("-MAIN MENU & KLELEATOMS MD big texts",2,16);
    VDP_drawText("-Button backgrounds",2,18);
    VDP_drawText("-Mouse cursor sprite",2,20);
    VDP_drawText("The game was built with SGDK "str(SGDK_VERSION)".",2,24);
}

void aboutstate_init(void)
{
    aboutBGImg = reserveVImage(&texTutorialBG,TRUE);
    initializeAboutColors();
    VDP_drawImageEx(BG_B,aboutBGImg->img,TILE_ATTR_FULL(PAL0,0,0,0,aboutBGImg->vPos),0,0,FALSE,FALSE);
    drawAboutMenu();
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
