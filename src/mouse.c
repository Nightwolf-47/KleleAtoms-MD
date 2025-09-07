#include "mouse.h"
#include <genesis.h>
#include <resources.h>

static Sprite* mouseSprite;

static bool mouseIsEnabled = FALSE;

static s16 mousePosX = 0;
static s16 mousePosY = 0;

// Possible in-game cursor colors
const u16 gameCursorPalettes[36] = {
    RGB24_TO_VDPCOLOR(0x440000), RGB24_TO_VDPCOLOR(0x880000), RGB24_TO_VDPCOLOR(0xCC0000), RGB24_TO_VDPCOLOR(0xEE0000), //Red default
    RGB24_TO_VDPCOLOR(0x000044), RGB24_TO_VDPCOLOR(0x000088), RGB24_TO_VDPCOLOR(0x0000AA), RGB24_TO_VDPCOLOR(0x0000CC), //Blue default
    RGB24_TO_VDPCOLOR(0x004400), RGB24_TO_VDPCOLOR(0x008800), RGB24_TO_VDPCOLOR(0x00AA00), RGB24_TO_VDPCOLOR(0x00CC00), //Green default
    RGB24_TO_VDPCOLOR(0x884400), RGB24_TO_VDPCOLOR(0xAA6600), RGB24_TO_VDPCOLOR(0xCC8800), RGB24_TO_VDPCOLOR(0xEEAA00), //Yellow default
    RGB24_TO_VDPCOLOR(0x442222), RGB24_TO_VDPCOLOR(0x662222), RGB24_TO_VDPCOLOR(0xAA2244), RGB24_TO_VDPCOLOR(0xCC2244), //Red original
    RGB24_TO_VDPCOLOR(0x002288), RGB24_TO_VDPCOLOR(0x0044CC), RGB24_TO_VDPCOLOR(0x0066EE), RGB24_TO_VDPCOLOR(0x22AAEE), //Blue original
    RGB24_TO_VDPCOLOR(0x006622), RGB24_TO_VDPCOLOR(0x228822), RGB24_TO_VDPCOLOR(0x44AA22), RGB24_TO_VDPCOLOR(0x66CC22), //Green original
    RGB24_TO_VDPCOLOR(0x886622), RGB24_TO_VDPCOLOR(0xAA8822), RGB24_TO_VDPCOLOR(0xCCAA22), RGB24_TO_VDPCOLOR(0xEECC44), //Yellow original
    RGB24_TO_VDPCOLOR(0x444444), RGB24_TO_VDPCOLOR(0x888888), RGB24_TO_VDPCOLOR(0xCCCCCC), RGB24_TO_VDPCOLOR(0xEEEEEE), //Gray
};

static inline u16 mouseConvCoord(s16 coord)
{
    return (u16)coord >> 2;
}

void mouse_init(void)
{
    if(mouseIsEnabled)
        return;
    JOY_setSupport(PORT_1, JOY_SUPPORT_MOUSE);
    mouseSprite = SPR_addSpriteSafe(&sprCursor,0,0,TILE_ATTR(PAL3,1,FALSE,FALSE));
    SPR_setFrame(mouseSprite, 0);
    SPR_setDepth(mouseSprite,SPR_MIN_DEPTH);
    mouseIsEnabled = TRUE;
}

void mouse_update(void)
{
    mousePosX = JOY_readJoypadX(JOY_1);
    mousePosX = clamp(mousePosX, 0, 1275);
    JOY_writeJoypadX(JOY_1,mousePosX);
    mousePosY = JOY_readJoypadY(JOY_1);
    mousePosY = clamp(mousePosY, 0, 891);
    JOY_writeJoypadY(JOY_1,mousePosY);
    SPR_setPosition(mouseSprite,mouseConvCoord(mousePosX),mouseConvCoord(mousePosY));
}

void mouse_setGameCursorColors(u16 color, u16 palette)
{
    u16 trueIndex = (palette << 4) + 12;
    if(isChangingState)
        memcpy(&newPalette[trueIndex], &gameCursorPalettes[color << 2], 4*sizeof(u16));
    else
        PAL_setColors(trueIndex,&gameCursorPalettes[color << 2],4,DMA_QUEUE);
}

void mouse_setCursorData(bool inMenus, u16 palette)
{
    SPR_setFrame(mouseSprite, !inMenus);
    SPR_setPalette(mouseSprite, palette & 3);
}

MousePosition mouse_getPosition(bool inTiles)
{
    MousePosition mpos;
    mpos.x = inTiles ? (mouseConvCoord(mousePosX)>>3) : mouseConvCoord(mousePosX);
    mpos.y = inTiles ? (mouseConvCoord(mousePosY)>>3) : mouseConvCoord(mousePosY);
    return mpos;
}

bool mouse_isEnabled(void)
{
    return mouseIsEnabled;
}

void mouse_stop(void)
{
    if(mouseSprite)
    {
        SPR_releaseSprite(mouseSprite);
        mouseSprite = NULL;
    }
    mouseIsEnabled = FALSE;
}
