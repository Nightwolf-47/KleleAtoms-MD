#ifndef DATA_H_INCLUDED
#define DATA_H_INCLUDED
#include <genesis.h>

#define STATE_COUNT 3 //Amount of game states

#define VIMAGE_MAXCOUNT 16 //Max amount of VidImages

#define GETCENTERX(str) (20-(strlen(str)>>1)) //Get the X position to make string centered

#define SFX_CLICK 64 //Menu option change and value change sound
#define SFX_PUT 65 //Atom place sound
#define SFX_EXPLODE 66 //Atom explosion sound

enum States { //List of game states
    ST_GAMESTATE,
    ST_TITLESTATE,
    ST_MENUSTATE
};

struct KASettings {
    u8 gridWidth;
    u8 gridHeight;
    u8 player1;
    u8 player2;
    u8 player3;
    u8 player4;
    u8 unused1; //Used to be taken by isHotSeat in 1.1.1 and older
    bool isHotSeat;
    u8 unused2; //Used to be taken by useOldColors in 1.1.1 and older
    bool useOldColors;
};

struct VidReservedImage //Structure containing a pointer to an image and VRAM position it should be drawn at (with VDP_drawImageEx TILE_ATTR_FULL)
{
    u16 vPos;
    const Image* img;
};

typedef struct VidReservedImage* VidImagePtr; //Shorter type for pointer to VidReservedImage

struct GameState { //Game state structure (not to be confused with ST_GAMESTATE)
    void (*init)(void);
    void (*stop)(void);
    void (*update)(fix32); //argument - fix32 dt
    void (*joyevent)(u16,u16,u16); //arguments - u16 joy, u16 changed, u16 state
};

void data_init(void);

void data_reset(void);

void data_stateInit(void);

void data_initsfx(void);

void initState(enum States newState);

void changeState(enum States newState);

extern struct GameState states[STATE_COUNT];
extern u8 currentState; //Current game state

extern bool randomNoPattern; //If TRUE, random() is called every frame to prevent the RNG from returning the same values every time

extern struct KASettings settings; //Game settings, can be changed in main menu

extern const char* versionStr; //Version string, shows up in About menu

extern u16 newPalette[64]; //New palette, has to be set in init() function of a given state

extern struct VidReservedImage vimages[VIMAGE_MAXCOUNT];

extern bool isDemoPlaying;

VidImagePtr reserveVImage(const Image* img); //returns pointer to VidReservedImage struct

#endif //DATA_H_INCLUDED
