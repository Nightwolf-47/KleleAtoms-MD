#include "data.h"
#include "save.h"
#include "../res/resources.h"
#include "states/game/gamestate.h"
#include "states/title/titlestate.h"
#include "states/menu/menustate.h"

struct KASettings settings;

struct GameState states[STATE_COUNT];
u8 currentState = ST_GAMESTATE;

bool randomNoPattern = TRUE;

const char* versionStr = "1.1.3";

u16 newPalette[64] = {0};

struct VidReservedImage vimages[VIMAGE_MAXCOUNT]; //Array of image data with reserved VRAM tile positions

int vImageCount = 0;

bool isDemoPlaying = FALSE;

void data_init(void)
{
    settings.gridWidth = 10;
    settings.gridHeight = 6;
    settings.player1 = 1;
    settings.player2 = 3;
    settings.player3 = 0;
    settings.player4 = 0;
    settings.isHotSeat = TRUE;
    settings.useOldColors = FALSE;
    settings.unused1 = 0;
    settings.unused2 = 0;
}

//Initialize all sounds
void data_initsfx(void)
{
    XGM_setPCM(SFX_CLICK,sfx_click,sizeof(sfx_click));
    XGM_setPCM(SFX_PUT,sfx_put,sizeof(sfx_put));
    XGM_setPCM(SFX_EXPLODE,sfx_explode,sizeof(sfx_explode));
}

void data_reset(void)
{
    data_init();
    invalidateSRAM();
}

//Initialize game states
void data_stateInit(void)
{
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
}

//Returns a pointer to a VidReservedImage struct
VidImagePtr reserveVImage(const Image* img)
{
    if(vImageCount>=VIMAGE_MAXCOUNT)
    {
        SYS_die("Too many images reserved");
    }
    VidImagePtr vidimg = &vimages[vImageCount];
    vidimg->img = img;
    vidimg->vPos = curTileInd;
    curTileInd += img->tileset->numTile;
    vImageCount++;
    return vidimg;
}

//Initializes the game with a given state (should be called only once)
void initState(enum States newState)
{
    if(newState >= STATE_COUNT)
    {
        SYS_die("Invalid game state!");
    }
    PAL_setColors(0,palette_black,64,CPU);
    vImageCount = 0;
    curTileInd = TILE_USER_INDEX;
    currentState = newState;
    memset(newPalette,0,sizeof(newPalette));
    newPalette[15] = RGB24_TO_VDPCOLOR(0xEEEEEE);
    if(states[currentState].init)
        states[currentState].init();
    PAL_fadeIn(0,63,newPalette,15,TRUE);
}

void changeState(enum States newState)
{
    if(newState >= STATE_COUNT)
    {
        SYS_die("Invalid game state!");
    }
    PAL_fadeOut(0,63,10,FALSE);
    if(states[currentState].stop)
        states[currentState].stop();
    VDP_clearPlane(BG_A,TRUE);
    VDP_clearPlane(BG_B,TRUE);
    VDP_clearSprites();
    SPR_defragVRAM();
    SPR_update();
    vImageCount = 0;
    curTileInd = TILE_USER_INDEX;
    currentState = newState;
    memset(newPalette,0,sizeof(newPalette));
    newPalette[15] = RGB24_TO_VDPCOLOR(0xEEEEEE);
    if(states[currentState].init)
        states[currentState].init();
    PAL_fadeIn(0,63,newPalette,15,TRUE);
}
