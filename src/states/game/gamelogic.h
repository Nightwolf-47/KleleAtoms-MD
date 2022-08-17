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
    short playerNum;
    unsigned char atomCount;
    long explodeTime; //If > 0, explosion sprite will appear
};

struct AtomPosition
{
    short destx;
    short desty;
};

struct KAExplodePos
{
    unsigned char x;
    unsigned char y;
    unsigned short willExplode;
};

struct KAGrid
{
    int width;
    int height;
    struct Tile tiles[84]; //In-game tiles, 12*7 -> 84 max
};

extern struct KAGrid grid;

extern unsigned char critGrid[84];

extern unsigned short animPlaying;

extern short curPlayer;

extern int playerTab[4];

extern int playerAtoms[4];

extern unsigned short playerMoved[4];

extern int playerCount;

extern int startPlayers;

extern unsigned char* atomStack;

extern int asPos;

extern struct KAExplodePos explodePos;

extern short playerWon;

extern int explosionCount;

extern struct AtomPosition atompos[16];

extern short atomposIndex;

extern unsigned short logicEnd;

extern short gridStartX;

extern short gridStartY;

//End the current game with a message
void logic_endMessage(char* msg);

const char* logic_getPlayerName(short playerNum);

void logic_loadAll(unsigned char gridWidth, unsigned char gridHeight, unsigned char (*ppttab)[4]);

void logic_fixLoadedData(void);

void logic_clickedTile(unsigned char tx, unsigned char ty, unsigned short isAI);

void logic_tick(long dt);

void logic_draw(long dt);

void logic_stop(void);

void drawTile(unsigned char x, unsigned char y, short playerNum, unsigned char atomCount);

//Convert grid tile position (x,y) to pixel position (pixelx,pixely)
void tileToPixels(unsigned char x, unsigned char y, short* pixelx, short* pixely);
