#include "tutorialstate.h"
#include <genesis.h>
#include "../../data.h"
#include "../../../res/resources.h"
#include "../../mouse.h"
#include <string.h>

#define TUTORIAL_TILE_COUNT 6
#define PAGE_COUNT 6

// Pack tile palette and index into one byte for storage in a const u8 tile animation array
#define PANIM(i,pal) (((i)<<4) | ((pal) & 3))

static VidImagePtr tutorialBGImg;
static VidImagePtr tuttiles[TUTORIAL_TILE_COUNT];
static VidImagePtr kamdtext;

static const fix32 animSpeed = FIX32(0.5);

static u16 pageNumber;

const char* pageTexts[PAGE_COUNT] = {
    "Welcome to KleleAtoms MD tutorial.\n\
It will teach you the game's basics.\n\n\
Press A/B/C button to continue or\n\
press START to quit the tutorial.\n\n\
Alternatively, you can press RIGHT\n\
or LEFT to navigate the pages.",

    "Each player can place one atom per\n\
turn on empty or their own tiles.",

    "The atoms will explode if too many\n\
are present on one tile.\n\n\
In the corners - 2 or more.\n\
On the sides - 3 or more.\n\
Anywhere else - 4 or more.",

    "Atom explosions can make nearby\n\
atoms explode, causing chain\n\
reactions.",

    "Exploding atoms turn nearby enemy\n\
atoms into current player atoms.",

    "If a player loses their atoms,\n\
they're out.\n\n\
The last standing player wins.\n\n\
That's the end of the tutorial!"
};

typedef struct TileAnimationData {
    bool enabled;
    u8 width;
    u8 height;
    u8 x;
    u8 y;
    u8 currentFrame;
    u8 frameSize;
    u8 frameCount;
    const u8* animationsArray;
    fix32 animTimer;
} TileAnimationData;

static TileAnimationData animationData;

const u8 tileAnimsPage2[] = {
    4,1,5,0xFA, //HEADER: WIDTH, HEIGHT, ANIMATION COUNT, MAGIC NUMBER
    PANIM(0,PAL0),PANIM(0,PAL1),PANIM(1,PAL2),PANIM(2,PAL3),
    PANIM(1,PAL0),PANIM(0,PAL1),PANIM(1,PAL2),PANIM(2,PAL3),
    PANIM(1,PAL0),PANIM(1,PAL1),PANIM(1,PAL2),PANIM(2,PAL3),
    PANIM(1,PAL0),PANIM(1,PAL1),PANIM(2,PAL2),PANIM(2,PAL3),
    PANIM(1,PAL0),PANIM(1,PAL1),PANIM(2,PAL2),PANIM(3,PAL3),
};

const u8 tileAnimsPage3[] = {
    7,3,4,0xFA, //HEADER: WIDTH, HEIGHT, ANIMATION COUNT, MAGIC NUMBER
    PANIM(1,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0)/**/,PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(3,PAL2),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(2,PAL1)/**/,PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),
    PANIM(2,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0)/**/,PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(4,PAL2),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(3,PAL1)/**/,PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),
    PANIM(5,PAL1),PANIM(1,PAL0),PANIM(0,PAL0),PANIM(1,PAL2),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(1,PAL1)/**/,PANIM(1,PAL0),PANIM(0,PAL0),PANIM(1,PAL2),PANIM(5,PAL1),PANIM(1,PAL2),PANIM(1,PAL1),PANIM(5,PAL1)/**/,PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(1,PAL2),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(1,PAL1),
    PANIM(0,PAL0),PANIM(1,PAL0),PANIM(0,PAL0),PANIM(1,PAL2),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(1,PAL1)/**/,PANIM(1,PAL0),PANIM(0,PAL0),PANIM(1,PAL2),PANIM(0,PAL2),PANIM(1,PAL2),PANIM(1,PAL1),PANIM(0,PAL1)/**/,PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(1,PAL2),PANIM(0,PAL0),PANIM(0,PAL0),PANIM(1,PAL1)
};

const u8 tileAnimsPage4[] = {
    3,4,7,0xFA, //HEADER: WIDTH, HEIGHT, ANIMATION COUNT, MAGIC NUMBER
    PANIM(1,PAL0),PANIM(2,PAL0),PANIM(0,PAL0)/**/,PANIM(2,PAL0),PANIM(3,PAL0),PANIM(0,PAL0)/**/,PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0)/**/,PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),
    PANIM(1,PAL0),PANIM(2,PAL0),PANIM(0,PAL0)/**/,PANIM(2,PAL0),PANIM(4,PAL0),PANIM(0,PAL0)/**/,PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0)/**/,PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),
    PANIM(1,PAL0),PANIM(3,PAL0),PANIM(0,PAL0)/**/,PANIM(3,PAL0),PANIM(5,PAL1),PANIM(1,PAL0)/**/,PANIM(0,PAL0),PANIM(1,PAL0),PANIM(0,PAL0)/**/,PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),
    PANIM(2,PAL0),PANIM(5,PAL1),PANIM(1,PAL0)/**/,PANIM(3,PAL0),PANIM(1,PAL0),PANIM(1,PAL0)/**/,PANIM(0,PAL0),PANIM(1,PAL0),PANIM(0,PAL0)/**/,PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),
    PANIM(5,PAL1),PANIM(1,PAL0),PANIM(1,PAL0)/**/,PANIM(4,PAL0),PANIM(1,PAL0),PANIM(1,PAL0)/**/,PANIM(0,PAL0),PANIM(1,PAL0),PANIM(0,PAL0)/**/,PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),
    PANIM(1,PAL0),PANIM(1,PAL0),PANIM(1,PAL0)/**/,PANIM(5,PAL1),PANIM(2,PAL0),PANIM(1,PAL0)/**/,PANIM(2,PAL0),PANIM(1,PAL0),PANIM(0,PAL0)/**/,PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),
    PANIM(1,PAL0),PANIM(1,PAL0),PANIM(1,PAL0)/**/,PANIM(0,PAL0),PANIM(2,PAL0),PANIM(1,PAL0)/**/,PANIM(2,PAL0),PANIM(1,PAL0),PANIM(0,PAL0)/**/,PANIM(0,PAL0),PANIM(0,PAL0),PANIM(0,PAL0),
};

const u8 tileAnimsPage5[] = {
    3,3,4,0xFA, //HEADER: WIDTH, HEIGHT, ANIMATION COUNT, MAGIC NUMBER
    PANIM(0,PAL3),PANIM(1,PAL2),PANIM(0,PAL1)/**/,PANIM(1,PAL1),PANIM(3,PAL0),PANIM(1,PAL3)/**/,PANIM(0,PAL1),PANIM(1,PAL1),PANIM(1,PAL2),
    PANIM(0,PAL3),PANIM(1,PAL2),PANIM(0,PAL1)/**/,PANIM(1,PAL1),PANIM(4,PAL0),PANIM(1,PAL3)/**/,PANIM(0,PAL1),PANIM(1,PAL1),PANIM(1,PAL2),
    PANIM(0,PAL3),PANIM(2,PAL0),PANIM(0,PAL1)/**/,PANIM(2,PAL0),PANIM(5,PAL1),PANIM(2,PAL0)/**/,PANIM(0,PAL1),PANIM(2,PAL0),PANIM(1,PAL2),
    PANIM(0,PAL3),PANIM(2,PAL0),PANIM(0,PAL1)/**/,PANIM(2,PAL0),PANIM(0,PAL0),PANIM(2,PAL0)/**/,PANIM(0,PAL1),PANIM(2,PAL0),PANIM(1,PAL2),
};

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

static u16 printMultilineText(const char* text, u16 x, u16 y, u16 verticalSpacing)
{
    u16 cury = y;
    u16 curx = x;
    u8 palette = VDP_getTextPalette();
    u8 priority = VDP_getTextPriority();
    VDPPlane plane = VDP_getTextPlane();
    s16 baseFontIndex = TILE_FONT_INDEX-32;
    char c;
    while(*text)
    {
        c = *text;
        switch(c)
        {
            case '\n':
                cury += verticalSpacing;
                curx = x;
                break;
            default:
                VDP_setTileMapXY(plane,TILE_ATTR_FULL(palette,priority,0,0,baseFontIndex+c),curx++,cury);
                break;
        }
        text++;
    }
    return cury;
}

static void drawTileAnimation(void)
{
    if(!animationData.enabled || !animationData.animationsArray)
        return;

    if(animationData.currentFrame >= animationData.frameCount)
        SYS_die("Invalid animation number selected!",NULL);

    u16 x = animationData.x;
    u16 y = animationData.y;
    u16 startIndex = animationData.currentFrame*animationData.frameSize;
    for(s16 dy=0; dy<animationData.height; dy++)
    {
        for(s16 dx=0; dx<animationData.width; dx++)
        {
            u8 animDataFull = animationData.animationsArray[startIndex+dx+dy*animationData.width];
            VidImagePtr curTile=tuttiles[animDataFull >> 4];
            u8 palette = animDataFull & 3;
            VDP_setTileMapEx(BG_A,curTile->img->tilemap,TILE_ATTR_FULL(palette,0,0,0,curTile->vPos),x+(dx*3),y+(dy*3),0,0,3,3,CPU);
        }
    }
}

static void initTileAnimation(const u8* tileAnimPtr, u16 x, u16 y)
{
    if(tileAnimPtr[3] != 0xFA)
        SYS_die("Loaded invalid tile animation!",NULL);

    animationData.enabled = TRUE;
    animationData.width = tileAnimPtr[0];
    animationData.height = tileAnimPtr[1];
    animationData.frameCount = tileAnimPtr[2];
    animationData.frameSize = animationData.width*animationData.height;
    animationData.animationsArray = &tileAnimPtr[4];
    animationData.currentFrame = 0;
    animationData.x = x;
    animationData.y = y;
    animationData.animTimer = 0;
    drawTileAnimation();
}

static void showPage(void)
{
    animationData.enabled = FALSE;
    VDP_clearPlane(BG_A,TRUE);
    u16 y;
    if(pageNumber > 0)
        y = printMultilineText(pageTexts[pageNumber],2,3,2);
    switch(pageNumber)
    {
        case 0:
            animationData.enabled = 0;
            VDP_setTileMapEx(BG_A,texKAMDText.tilemap,TILE_ATTR_FULL(PAL0,0,0,0,kamdtext->vPos),2,2,0,0,35,4,CPU);
            printMultilineText(pageTexts[0],2,8,2);
            break;
        case 1:
            initTileAnimation(&tileAnimsPage2[0],14,y+8);
            break;
        case 2:
            initTileAnimation(&tileAnimsPage3[0],10,y+2);
            break;
        case 3:
            initTileAnimation(&tileAnimsPage4[0],16,y+4);
            break;
        case 4:
            initTileAnimation(&tileAnimsPage5[0],16,y+6);
            break;
        default:
            break;
    }
    char buf[16];
    sprintf(buf,"Page %d/%d",pageNumber+1,PAGE_COUNT);
    VDP_drawText(buf,RIGHTALIGNX(buf,37),25);
}

static void setupTutorial(void)
{
    tutorialBGImg = reserveVImage(&texTutorialBG,TRUE);
    tuttiles[0] = reserveVImage(&texTile,TRUE);
    tuttiles[1] = reserveVImage(&texTile1,TRUE);
    tuttiles[2] = reserveVImage(&texTile2,TRUE);
    tuttiles[3] = reserveVImage(&texTile3,TRUE);
    tuttiles[4] = reserveVImage(&texTile4,TRUE);
    tuttiles[5] = reserveVImage(&texTileExp,TRUE);
    kamdtext = reserveVImage(&texKAMDText,TRUE);
    memset(&animationData,0,sizeof(animationData));
    setupTutorialColors(settings.useOldColors);
    showPage();
}

void tutorialstate_init(void)
{
    pageNumber = 0;
    setupTutorial();
    if(mouse_isEnabled())
    {
        mouse_setGameCursorColors(8,PAL3);
        mouse_setCursorData(FALSE,PAL3);
    }
    VDP_drawImageEx(BG_B,tutorialBGImg->img,TILE_ATTR_FULL(PAL0,0,0,0,tutorialBGImg->vPos),0,0,FALSE,FALSE);
}

void tutorialstate_update(fix32 dt)
{
    if(animationData.enabled)
    {
        animationData.animTimer += dt;
        if(animationData.animTimer >= animSpeed)
        {
            animationData.animTimer = 0;
            animationData.currentFrame++;
            if(animationData.currentFrame >= animationData.frameCount)
                animationData.currentFrame = 0;
            drawTileAnimation();
        }
    }
}

void tutorialstate_joyevent(u16 joy, u16 changed, u16 state)
{
    if(state & changed)
    {
        switch(changed)
        {
            case BUTTON_A:
            case BUTTON_B:
            case BUTTON_C:
            case BUTTON_RIGHT:
                pageNumber++;
                if(pageNumber >= PAGE_COUNT)
                {
                    changeState(ST_MENUSTATE);
                    return;
                }
                showPage();
                break;
            case BUTTON_LEFT:
                if(pageNumber > 0)
                {
                    pageNumber--;
                    showPage();
                }
                break;
            case BUTTON_START:
                changeState(ST_MENUSTATE);
                break;
        }
    }
}

void tutorialstate_stop(void)
{
    animationData.enabled = 0;
    if(mouse_isEnabled())
        mouse_setCursorData(TRUE,PAL3);
}