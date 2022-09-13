#ifndef GAMELOGIC_H_INCLUDED
#define GAMELOGIC_H_INCLUDED
#include <types.h>

#define GRIDSIZE 24

#define ATOMSPEED 30

#define PTAB_NO 0
#define PTAB_LOST 1
#define PTAB_PLAY 2

#define NOPLAYER -1

#define GXYIndex(x,y) ((x)+grid.width*(y))

#define PACKCoords(x,y) (((u16)((u8)(x))<<8) | (u8)(y))

#define PACKCoords8(x,y) (((x)<<4) | ((y) & 0xF))

struct Tile
{
    s16 playerNum;
    u8 atomCount;
    fix32 explodeTime; //If > 0, explosion sprite will appear
};

struct AtomPosition
{
    s16 destx;
    s16 desty;
};

struct KAExplodePos
{
    u8 x;
    u8 y;
    bool willExplode;
};

struct KAGrid
{
    int width;
    int height;
    struct Tile tiles[84]; //In-game tiles, 12*7 -> 84 max
};

extern struct KAGrid grid;

extern u8 critGrid[84];

extern bool animPlaying;

extern s16 curPlayer;

extern int playerTab[4];

extern int playerAtoms[4];

extern bool playerMoved[4];

extern int playerCount;

extern int startPlayers;

extern u8* atomStack;

extern int asPos;

extern struct KAExplodePos explodePos;

extern s16 playerWon;

extern int explosionCount;

extern struct AtomPosition atompos[16];

extern s16 atomposIndex;

extern bool logicEnd;

extern s16 gridStartX;

extern s16 gridStartY;

//End the current game with a message
void logic_endMessage(const char* msg);

const char* logic_getPlayerName(s16 playerNum);

void logic_loadAll(u8 gridWidth, u8 gridHeight, u8 (*ppttab)[4]);

void logic_fixLoadedData(void);

void logic_clickedTile(u8 tx, u8 ty, bool isAI);

void logic_tick(fix32 dt);

void logic_draw(fix32 dt);

void logic_stop(void);

void drawTile(u8 x, u8 y, s16 playerNum, u8 atomCount);

//Convert grid tile position (x,y) to pixel position (pixelx,pixely)
void tileToPixels(u8 x, u8 y, s16* pixelx, s16* pixely);

#endif //GAMELOGIC_H_INCLUDED
