#include "gamelogic.h"
#include "gameai.h"
#include "../../data.h"
#include "../../../res/resources.h"

#define ATOMSTACKSIZE 4000

#define SPRITEPURGATORYX -16

#define PMOVETABNONE 0x8080

#define ANIMTABNONE 0x8080

#define EXPLOSIONINDEX 0xFF

struct KAGrid grid; //In-game grid

u8 critGrid[84]; //Array of critical atom amounts for each tile

bool animPlaying; //If TRUE, an animation is playing

s16 curPlayer; //Current player (0-3)

int playerTab[4]; //Player statuses (PTAB_NO - not playing, PTAB_LOSE - lost, PTAB_PLAY - playing)

int playerAtoms[4]; //Player atom count, updated every frame

bool playerMoved[4];

int playerCount; //Amount of players still playing

int startPlayers; //Total amount of players

u8* atomStack; //Stack of up to 4000 previous exploded tile positions, stored in ((x<<4) | y) format

int asPos; //AtomStack position

struct KAExplodePos explodePos; //Structure: {x,y,willExplode} used for queued exploding tile

s16 playerWon; //If >= 0, the winning player number

int explosionCount;

struct AtomPosition atompos[16]; //Final atom sprite positions

u16 animatedTiles[4]; //Animated tile positions in ((x<<8) | y) format

s16 animTilePos; //Amount of animated tile positions stored

Sprite* atomSprites[16]; //All atom sprites

s16 atomposIndex; //Amount of atom sprites and atompos structs used

s16 gridStartX = 1; //Start X position of the grid, in 8x8 pixel tiles
s16 gridStartY = 6; //Start Y position of the grid, in 8x8 pixel tiles

VidImagePtr ttimages[6]; //Tile images (0 - none, 1-4 - atoms, 5 - exploded)

const char* playerNames[4] = {"Red","Blue","Green","Yellow"};

bool logicEnd = FALSE; //If TRUE the game will end and it will wait for a button press

const fix32 baseExplodeTime = FIX32(0.3); //Longest duration of explosion sprite being shown after atom explosion

void logic_endMessage(const char* msg)
{
    logicEnd = TRUE;
    VDP_clearText(5+8*curPlayer,3,1);
    VDP_clearText(8+8*curPlayer,3,1);
    VDP_clearText(0,5,40);
    VDP_clearTileMapRect(BG_B,1,0,40,6);
    VDP_clearText(31,0,9);
    VDP_clearText(1,0,10);
    VDP_drawText(msg,GETCENTERX(msg),0);
    VDP_drawText("Press any button to go to menu",5,2);
    curPlayer = 0;
}

//drawTile and tileToPixels are the only functions in this file without "logic_" prefix that can be accessed outside of it

void drawTile(u8 x, u8 y, s16 player, u8 atomCount)
{ 
    atomCount = (atomCount==EXPLOSIONINDEX) ? 5 : min(atomCount,4);
    VDP_drawImageEx(BG_B,ttimages[atomCount]->img,TILE_ATTR_FULL(player,0,FALSE,FALSE,ttimages[atomCount]->vPos),(x*3)+gridStartX,(y*3)+gridStartY,FALSE,TRUE);
}

void tileToPixels(u8 x, u8 y, s16* pixelx, s16* pixely)
{
    *pixelx = (x*24)+(gridStartX<<3);
    *pixely = (y*24)+(gridStartY<<3);
}

void moveAtomSprite(u8 index, s16 dx, s16 dy)
{
    s16 sprx = SPR_getPositionX(atomSprites[index]);
    s16 spry = SPR_getPositionY(atomSprites[index]);
    SPR_setPosition(atomSprites[index],sprx+dx,spry+dy);
}

//Change the player to the next possible one and reset the AI timer
void nextPlayer(void)
{
    ai_resetTime();
    if(playerCount < 2)
        return;
    
    VDP_clearText(5+8*curPlayer,3,1);
    VDP_clearText(8+8*curPlayer,3,1);

    do
    {
        curPlayer++;
        if(curPlayer>=4)
        {
            curPlayer = 0;
        }
    } 
    while(playerTab[curPlayer]!=PTAB_PLAY);
    VDP_drawText(">",5+8*curPlayer,3);
    VDP_drawText("<",8+8*curPlayer,3);
}

//Set atoms and player number on a tile (currently unused)
void setAtoms(u8 x, u8 y, u8 atomCount, s16 player)
{
    u16 index = GXYIndex(x,y);
    grid.tiles[index].atomCount = atomCount;
    grid.tiles[index].playerNum = player;
    drawTile(x,y,player,atomCount);
}

//Add atoms on a tile and change tile player number if (player != NOPLAYER)
void addAtoms(u8 x, u8 y, u8 atomCount, s16 player)
{
    u16 index = GXYIndex(x,y);
    grid.tiles[index].atomCount += atomCount;
    grid.tiles[index].playerNum = (player==NOPLAYER) ? grid.tiles[index].playerNum : player;
    drawTile(x,y,grid.tiles[index].playerNum,grid.tiles[index].atomCount);
}

//Put an atom, return TRUE if the tile becomes or was already critical
bool putAtom(u8 x, u8 y, s16 newPlayer)
{
    if(x>=grid.width || y>=grid.height)
        return FALSE;
    const u8 begpos = 2; //smallest X atom position offset (or beginning Y - 1)
    const u8 midpos = 8; //middle X and Y atom position offset
    const u8 endpos = 14; //biggest X and Y atom position offset
    struct Tile* atomTile = &grid.tiles[GXYIndex(x,y)];
    if(newPlayer==NOPLAYER)
        atomTile->playerNum = curPlayer;
    else
        atomTile->playerNum = newPlayer;
    s16 atplayer = atomTile->playerNum;
    if(atplayer<0 || atplayer>3)
    {
        SYS_die("Wrong player on tile!"); 
    }
    drawTile(x,y,atplayer,0);
    s16 px;
    s16 py;
    tileToPixels(x,y,&px,&py); //Get position of the tile in pixels
    atomTile->atomCount++;
    if(atomposIndex > 12)
    {
        SYS_die("Too many sprites (more than 16) requested!");
    }
    if(animTilePos > 3)
    {
        SYS_die("Too many tiles with animations (more than 4) requested!");
    }
    s16 atpi = atomposIndex;
    for(int i=0; i<atomTile->atomCount; i++)
    {
        if(atomTile->atomCount==1) //Just draw the atom immediately, no sprites involved
        {
            drawTile(x,y,atplayer,atomTile->atomCount);
            return FALSE;
        }

        animPlaying = TRUE;

        switch(i) //Draw sprites and set their final positions
        {
            case 0:
                atompos[atpi].destx = px+midpos;
                atompos[atpi].desty = py+midpos;
                SPR_setPalette(atomSprites[atpi],atplayer);
                SPR_setPosition(atomSprites[atpi],px+midpos,py+midpos);
                SPR_setVisibility(atomSprites[atpi],AUTO_FAST);
                atomposIndex++;
                break;
            case 1:
                atompos[atpi].destx = px+endpos;
                atompos[atpi+1].destx = px+begpos;
                atompos[atpi+1].desty = py+midpos;
                SPR_setPalette(atomSprites[atpi+1],atplayer);
                SPR_setPosition(atomSprites[atpi+1],px+midpos,py+midpos);
                SPR_setVisibility(atomSprites[atpi+1],AUTO_FAST);
                atomposIndex++;
                break;
            case 2:
                atompos[atpi].desty = py+begpos+1;
                atompos[atpi+1].desty = py+begpos+1;
                atompos[atpi+2].destx = px+midpos;
                atompos[atpi+2].desty = py+endpos;
                SPR_setPosition(atomSprites[atpi],px+endpos,py+midpos);
                SPR_setPosition(atomSprites[atpi+1],px+begpos,py+midpos);
                SPR_setPalette(atomSprites[atpi+2],atplayer);
                SPR_setPosition(atomSprites[atpi+2],px+midpos,py+midpos);
                SPR_setVisibility(atomSprites[atpi+2],AUTO_FAST);
                atomposIndex++;
                break;
            case 3:
                atompos[atpi+2].destx = px+begpos;
                atompos[atpi+3].destx = px+endpos;
                atompos[atpi+3].desty = py+endpos;
                SPR_setPosition(atomSprites[atpi],px+endpos,py+begpos+1);
                SPR_setPosition(atomSprites[atpi+1],px+begpos,py+begpos+1);
                SPR_setPosition(atomSprites[atpi+2],px+midpos,py+endpos);
                SPR_setPalette(atomSprites[atpi+3],atplayer);
                SPR_setPosition(atomSprites[atpi+3],px+midpos,py+endpos);
                SPR_setVisibility(atomSprites[atpi+3],AUTO_FAST);
                atomposIndex++;
                break;
            default:
                break;
        }
    }
    if(animPlaying) //If atoms are animated (they should), put the current tile in animatedTiles array
    {
        animatedTiles[animTilePos] = PACKCoords(x,y);
        animTilePos++;
    }
    if(atplayer >= 0)
    {
        if(grid.tiles[GXYIndex(x,y)].atomCount >= critGrid[GXYIndex(x,y)]) //If the tile is critical, return TRUE
        {
            return TRUE;
        }
    }
    return FALSE;
}

//Check if a tile is critical
bool checkTile(u8 x, u8 y)
{
    u16 index = GXYIndex(x,y);
    struct Tile* checkedTile = &grid.tiles[index];
    if(checkedTile->playerNum >= 0 && checkedTile->atomCount >= critGrid[index])
    {
        return TRUE;
    }
    return FALSE;
}

//Check if any surrounding tiles are critical, return the X,Y offset packed into a u16 integer
u16 checkSurrounding(u8 x, u8 y)
{
    if(y+1 < grid.height)
    {
        if(checkTile(x,y+1))
        {
            return PACKCoords(0,1);
        }
    }
    if(y > 0)
    {
        if(checkTile(x,y-1))
        {
            return PACKCoords(0,-1);
        }
    }
    if(x+1 < grid.width)
    {
        if(checkTile(x+1,y))
        {
            return PACKCoords(1,0);
        }
    }
    if(x > 0)
    {
        if(checkTile(x-1,y))
        {
            return PACKCoords(-1,0);
        }
    }
    return PMOVETABNONE;
}

//Blow up atoms, spread them and play explosion sound
void explodeAtoms(u8 x, u8 y, s16 atplayer)
{
    explosionCount++;
    u16 index = GXYIndex(x,y);
    struct Tile* curTile = &grid.tiles[index];
    curTile->explodeTime = baseExplodeTime/max(min(explosionCount,1000)/10,1);
    u8 extra = (u8)max(curTile->atomCount-critGrid[index],0); //Amount of atoms above critical amount
    curTile->atomCount = 0;
    curTile->playerNum = NOPLAYER;
    if(y+1<grid.height)
    {
        addAtoms(x,y+1,extra,atplayer);
        extra = 0;
        putAtom(x,y+1,atplayer);
    }
    if(y>0)
    {
        addAtoms(x,y-1,extra,atplayer);
        extra = 0;
        putAtom(x,y-1,atplayer);
    }
    if(x+1<grid.width)
    {
        addAtoms(x+1,y,extra,atplayer);
        extra = 0;
        putAtom(x+1,y,atplayer);
    }
    if(x>0)
    {
        addAtoms(x-1,y,extra,atplayer);
        extra = 0;
        putAtom(x-1,y,atplayer);
    }
    if(explosionCount<=1000)
    {
        XGM_stopPlayPCM(SOUND_PCM_CH3);
        XGM_startPlayPCM(SFX_EXPLODE,2,SOUND_PCM_CH3);
    }
    drawTile(x,y,1,EXPLOSIONINDEX); //Draw exploding tile
}

void prepareNewAtoms(u8 tx, u8 ty)
{
    playerAtoms[curPlayer] = max(playerAtoms[curPlayer],1);
    if(putAtom(tx,ty,curPlayer)) //If tile becomes critical, blow it up and add the position to atomStack
    {
        explodePos.x = tx;
        explodePos.y = ty;
        explodePos.willExplode = TRUE;
        atomStack[asPos] = PACKCoords8(tx,ty);
        asPos++;
    }
    else //Otherwise, it's next player's turn
    {
        nextPlayer();
    }
}

const char* logic_getPlayerName(s16 playerNum)
{
    return playerNames[playerNum];
}

//Set all gamelogic variables before starting the game
void logic_loadAll(u8 gridWidth, u8 gridHeight, u8 (*ppttab)[4])
{
    u8* pttab = *ppttab;
    grid.height = gridHeight;
    grid.width = gridWidth;
    gridStartX = 2+((36-(grid.width*3))>>1);
    gridStartY = 6+((21-(grid.height*3))>>1);
    ttimages[0] = reserveVImage(&texTile); //Prepare tile images for drawing
    ttimages[1] = reserveVImage(&texTile1);
    ttimages[2] = reserveVImage(&texTile2);
    ttimages[3] = reserveVImage(&texTile3);
    ttimages[4] = reserveVImage(&texTile4);
    ttimages[5] = reserveVImage(&texTileExp);
    if(gridHeight > 7 || gridWidth > 12) //Prevent overflow from invalid grid size
    {
        SYS_die("Invalid grid size");
        return;
    }
    memset(grid.tiles,0,84*sizeof(struct Tile));
    for(int x=0; x<grid.width; x++) //Reset grid player number and precalculate critical atom amount for each grid tile
    {
        for(int y=0; y<grid.height; y++)
        {
            grid.tiles[GXYIndex(x,y)].playerNum = NOPLAYER;
            critGrid[GXYIndex(x,y)] = 4;
            if(x==0 || x==(grid.width-1))
            {
                if(y==0 || y==(grid.height-1))
                {
                    critGrid[GXYIndex(x,y)] = 2;
                }
                else
                {
                    critGrid[GXYIndex(x,y)] = 3;
                }
            }
            else if(y==0 || y==(grid.height-1))
            {
                critGrid[GXYIndex(x,y)] = 3;
            }
        }
    }
    animPlaying = FALSE;
    curPlayer = NOPLAYER;
    memset(playerTab,0,4*sizeof(int));
    for(int i=0; i<4; i++)
    {
        playerAtoms[i] = -1;
    }
    memset(playerMoved,0,4*sizeof(bool));
    playerCount = 0;
    ai_init();
    for(int i=0; i<4; i++)
    {
        aiDifficulty[i] = 2;
        aiPlayerTab[i] = FALSE;
        if(pttab[i] > 0) //Player exists
        {
            if(curPlayer == NOPLAYER)
                curPlayer = i;
            playerCount++;
            playerTab[i] = PTAB_PLAY;
            playerAtoms[i] = 0;
            playerMoved[i] = FALSE;
            if(pttab[i] > 1) //Player is AI
            {
                aiPlayerTab[i] = TRUE;
                aiDifficulty[i] = pttab[i] - 1;
            }
        }
        else //Player is not present, make their icon black (invisible)
        {
            newPalette[(i*16)+6] = RGB24_TO_VDPCOLOR(0x000000);
        }
    }
    if(curPlayer == NOPLAYER || playerCount < 2)
    {
        logic_endMessage("2 or more players required to play!");
        return;
    }
    startPlayers = playerCount;
    atomStack = MEM_alloc(ATOMSTACKSIZE); //Allocate 4000 bytes of memory to atomStack
    if(!atomStack)
    {
        SYS_die("Couldn't allocate space for the atom stack!");
        return;
    }
    memset(atomStack,0,ATOMSTACKSIZE);
    asPos = 0;
    memset(&explodePos,0,sizeof(struct KAExplodePos));
    playerWon = NOPLAYER;
    explosionCount = 0;
    memset(atompos,0,4*sizeof(struct AtomPosition));
    atomposIndex = 0;
    animTilePos = 0;
    logicEnd = FALSE;
    for(int i=0; i<16; i++)
    {
        atomSprites[i] = SPR_addSprite(&sprAtom,0,0,TILE_ATTR(PAL0,0,FALSE,FALSE));
        SPR_setVisibility(atomSprites[i],HIDDEN);
    }
}

//Fix grid start position, critical atom table values and player icon colors after loading a saved game
void logic_fixLoadedData()
{
    gridStartX = 2+((36-(grid.width*3))>>1);
    gridStartY = 6+((21-(grid.height*3))>>1);
    for(int x=0; x<grid.width; x++)
    {
        for(int y=0; y<grid.height; y++)
        {
            critGrid[GXYIndex(x,y)] = 4;
            if(x==0 || x==(grid.width-1))
            {
                if(y==0 || y==(grid.height-1))
                {
                    critGrid[GXYIndex(x,y)] = 2;
                }
                else
                {
                    critGrid[GXYIndex(x,y)] = 3;
                }
            }
            else if(y==0 || y==(grid.height-1))
            {
                critGrid[GXYIndex(x,y)] = 3;
            }
        }
    }
    if(curPlayer == NOPLAYER || playerCount < 2)
    {
        logic_endMessage("2 or more players required to play!");
        return;
    }
    for(int i=0; i<4; i++)
    {
        switch(playerTab[i])
        {
            case PTAB_NO:
                newPalette[(i*16)+6] = RGB24_TO_VDPCOLOR(0x000000);
                break;
            case PTAB_LOST:
                newPalette[(i*16)+6] = RGB24_TO_VDPCOLOR(0x808080);
                break;
            default:
                break;
        }
    }
    animPlaying = FALSE;
}

//Return TRUE if the atom sprite is at the final position
bool isAtomAtDestination(u8 index)
{
    s16 px = SPR_getPositionX(atomSprites[index]);
    s16 py = SPR_getPositionY(atomSprites[index]);
    return (bool)(atompos[index].destx==px && atompos[index].desty==py);
}

//Callback when a grid tile was pressed
void logic_clickedTile(u8 tx, u8 ty, bool isAI)
{
    u16 index = GXYIndex(tx,ty);
    s16 atplayer = grid.tiles[index].playerNum;
    if(atplayer >= 0 && atplayer!=curPlayer) //Can't place atoms on someone else's tile
        return;
    if(!isAI && aiPlayerTab[curPlayer]) //Only AI can place atoms on their turn
        return;
    if(playerCount<2 || animPlaying || asPos>0 || explodePos.willExplode) //Wait until the previous turn finishes to make a move
        return;
    explosionCount = 0;
    playerMoved[curPlayer] = TRUE;
    XGM_startPlayPCM(SFX_PUT,1,SOUND_PCM_CH2);
    prepareNewAtoms(tx,ty);
}


void logic_tick(fix32 dt)
{
    if(explosionCount >= ATOMSTACKSIZE) //Prevent atomStack overflow and a softlock when too many explosions happen
    {
        logic_endMessage("More than 4000 explosions! Stopping...");
        return;
    }
    for(int i=0; i<4; i++) //Disqualify players who lost all their atoms
    {
        if(playerMoved[i] && playerAtoms[i]==0 && playerTab[i]==PTAB_PLAY)
        {
            playerCount--;
            playerTab[i] = PTAB_LOST;
            PAL_setColor((i*16)+6,RGB24_TO_VDPCOLOR(0x808080)); //Gray out the player icon
        }
    }
    if(playerCount<2)
    {
        for(int i=0; i<4; i++) //Pick the winner
        {
            if(playerTab[i]==PTAB_PLAY)
            {
                playerWon = i;
                return;
            }
        }
    }
    if(playerTab[curPlayer]!=PTAB_PLAY) //If the player is disqualified/not present, pick next one
    {
        nextPlayer();
    }
    if(!animPlaying)
    {
        if(explodePos.willExplode) //Blow up atoms queued for explosion
        {
            s8 tx = explodePos.x;
            s8 ty = explodePos.y;
            explodeAtoms(tx,ty,curPlayer);
            explodePos.x = 0;
            explodePos.y = 0;
            explodePos.willExplode = FALSE;
        }
        else if(asPos > 0)
        {
            int curAsPos = asPos;
            int popCount = min(asPos,50);
            for(int i=0; i<popCount; i++) //Pop up to 50 positions from atomStack, stop if a critical tile is found or atomStack is empty
            {
                s8 ttposx = atomStack[curAsPos-1] >> 4;
                s8 ttposy = atomStack[curAsPos-1] & 0xF;
                u16 pmovetab = checkSurrounding(ttposx,ttposy);
                if(pmovetab!=PMOVETABNONE) //Critical tile is found near the checked tile
                {
                    ttposx += (s8)(pmovetab >> 8);
                    ttposy += (s8)(pmovetab & 0xFF);
                    explodePos.x = ttposx;
                    explodePos.y = ttposy;
                    explodePos.willExplode = TRUE;
                    atomStack[curAsPos] = PACKCoords8(ttposx,ttposy);
                    curAsPos++;
                    break;
                }
                else //Pop the current position from the stack
                {
                    atomStack[curAsPos-1] = 0;
                    curAsPos--;
                    if(curAsPos==0) //atomStack is empty, it's next player's turn now
                    {
                        nextPlayer();
                        break;
                    }
                }
            }
            asPos = curAsPos;
        }
        else if(aiPlayerTab[curPlayer]) //If the current player is AI, attempt to move (will succeed once per turn after 0.3 seconds)
        {
            ai_tryMove(dt);
        }
    }
}

void logic_draw(fix32 dt)
{
    for(int i=0; i<4; i++)
    {
        playerAtoms[i] = 0;
    }
    bool isAnyTileExploding = FALSE;
    for(int x=0; x<grid.width; x++) //Calculate atom count for each player, reset playerNum in tiles without atoms and handle exploding tiles
    {
        for(int y=0; y<grid.height; y++)
        {
            struct Tile* curTile = &grid.tiles[GXYIndex(x,y)];
            if(curTile->atomCount==0)
                curTile->playerNum = NOPLAYER;
            else
                playerAtoms[curTile->playerNum] += curTile->atomCount;
            if(curTile->explodeTime > 0)
            {
                curTile->explodeTime -= dt;
                if(curTile->explodeTime <= 0) //If a tile stopped exploding, remove the explosion sprite
                {
                    curTile->explodeTime = -1;
                    drawTile(x,y,curTile->playerNum & 3,curTile->atomCount);
                }
                else //Otherwise prevent all animations from playing until all explosions stop
                {
                    isAnyTileExploding = TRUE;
                }
            }
        }
    }
    if(animPlaying)
    {
        static fix32 leftMoveSpeed = 0;
        leftMoveSpeed += ATOMSPEED*dt*max(min(explosionCount,1000)/10,1);
        bool stillPlaying = FALSE; //TRUE if any atoms are moving
        int movespeed = fix32ToInt(leftMoveSpeed);
        leftMoveSpeed -= intToFix32(movespeed);
        for(int i=0; i<atomposIndex; i++)
        {
            if(!isAtomAtDestination(i)) //Move all atoms closer to the destination
            {
                stillPlaying = TRUE;
                s16 px = SPR_getPositionX(atomSprites[i]);
                s16 py = SPR_getPositionY(atomSprites[i]);
                s32 xdist = abs(atompos[i].destx-px);
                s32 ydist = abs(atompos[i].desty-py);
                px = (atompos[i].destx-px > 0) ? px+min(xdist,movespeed) : px-min(xdist,movespeed);
                py = (atompos[i].desty-py > 0) ? py+min(ydist,movespeed) : py-min(ydist,movespeed);
                SPR_setPosition(atomSprites[i],px,py);
            }
        }
        animPlaying = (stillPlaying || isAnyTileExploding);
        if(!animPlaying) //If no atoms are moving and no tiles are exploding anymore
        {
            for(int i=0; i<atomposIndex; i++) //Hide all the sprites for later use
            {
                SPR_setPosition(atomSprites[i],SPRITEPURGATORYX,0);
                SPR_setVisibility(atomSprites[i],HIDDEN);
            }
            atomposIndex = 0;
            struct Tile* curTile;
            for(int i=0; i<animTilePos; i++) //Draw atoms on all tiles that had atom sprites
            {
                u8 tx = animatedTiles[i] >> 8;
                u8 ty = animatedTiles[i] & 0xFF;
                curTile = &grid.tiles[GXYIndex(tx,ty)];
                drawTile(tx,ty,curTile->playerNum,curTile->atomCount);
            }
            animTilePos = 0;
        }
    }
}

void logic_stop(void)
{
    if(atomStack)
    {
        MEM_free(atomStack);
        atomStack = NULL;
    }
    if(atomSprites[0])
    {
        for(int i=0; i<16; i++)
        {
            SPR_releaseSprite(atomSprites[i]);
            atomSprites[i] = NULL;
        }
    }
}
