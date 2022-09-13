#include "gameai.h"
#include "gamelogic.h"
#include "../../data.h"

const fix32 aiDelay = FIX32(0.3); //Minimal delay between the start of a turn and AI movement

fix32 aiTime = 0; //AI timer, when aiTime >= aiDelay, make a move

bool aiPlayerTab[4] = {0,0,0,0};

int aiDifficulty[4] = {2,2,2,2};

s16 ccdCheckTab[84] = {-1}; //Corner check table - used for checking for atoms in diagonally neighboring tiles to prevent cheesing AI 3

#define NOTILE -1

//Returns TRUE if the tile belongs to another player
bool aiIsTileEnemy(u8 x, u8 y)
{
    s16 tplayer = grid.tiles[GXYIndex(x,y)].playerNum;
    return (bool)(tplayer!=NOPLAYER && tplayer!=curPlayer);
}

//Get the amount of corners where the AI has atoms
s16 aiGetCorners(u8 gridw, u8 gridh)
{
    s16 corcount = 0;
    if(grid.tiles[GXYIndex(0,0)].playerNum == curPlayer)
        corcount++;
    if(grid.tiles[GXYIndex(gridw-1,0)].playerNum == curPlayer)
        corcount++;
    if(grid.tiles[GXYIndex(0,gridh-1)].playerNum == curPlayer)
        corcount++;
    if(grid.tiles[GXYIndex(gridw-1,gridh-1)].playerNum == curPlayer)
        corcount++;
    return corcount;
}

//Returns TRUE if there are any neighboring atoms
bool aiAtomsNearby(u8 x, u8 y)
{
    if(y>0 && grid.tiles[GXYIndex(x,y-1)].atomCount > 0)
        return TRUE;
    if(y<grid.height-1 && grid.tiles[GXYIndex(x,y+1)].atomCount > 0)
        return TRUE;
    if(x>0 && grid.tiles[GXYIndex(x-1,y)].atomCount > 0)
        return TRUE;
    if(x<grid.width-1 && grid.tiles[GXYIndex(x+1,y)].atomCount > 0)
        return TRUE;
    return FALSE;
}

//Returns TRUE if the current player has an advantage over another player on a given tile
bool aiCheckAdvantage(u8 x, u8 y, s16 patoms)
{
    if(y>0 && aiIsTileEnemy(x,y-1) && patoms>=(s16)grid.tiles[GXYIndex(x,y-1)].atomCount-(s16)critGrid[GXYIndex(x,y-1)])
        return TRUE;
    if(y<grid.height-1 && aiIsTileEnemy(x,y+1) && patoms>=(s16)grid.tiles[GXYIndex(x,y+1)].atomCount-(s16)critGrid[GXYIndex(x,y+1)])
        return TRUE;
    if(x>0 && aiIsTileEnemy(x-1,y) && patoms>=(s16)grid.tiles[GXYIndex(x-1,y)].atomCount-(s16)critGrid[GXYIndex(x-1,y)])
        return TRUE;
    if(x<grid.width-1 && aiIsTileEnemy(x+1,y) && patoms>=(s16)grid.tiles[GXYIndex(x+1,y)].atomCount-(s16)critGrid[GXYIndex(x+1,y)])
        return TRUE;
    return FALSE;
}

//Returns TRUE if it's viable to put an atom in the given corner tile
bool aiCornerCheck(u8 x, u8 y)
{
    s16 curIndex = GXYIndex(x,y);
    if(critGrid[curIndex] > 2)
        return FALSE;

    u16 ccval = 0;
    s16 patoms = -2;
    if(y>0 && grid.tiles[GXYIndex(x,y-1)].playerNum >= 0 && patoms>=(s16)grid.tiles[GXYIndex(x,y-1)].atomCount-(s16)critGrid[GXYIndex(x,y-1)])
        ccval++;
    if(y<grid.height-1 && grid.tiles[GXYIndex(x,y+1)].playerNum >= 0 && patoms>=(s16)grid.tiles[GXYIndex(x,y+1)].atomCount-(s16)critGrid[GXYIndex(x,y+1)])
        ccval++;
    if(x>0 && grid.tiles[GXYIndex(x-1,y)].playerNum >= 0 && patoms>=(s16)grid.tiles[GXYIndex(x-1,y)].atomCount-(s16)critGrid[GXYIndex(x-1,y)])
        ccval++;
    if(x<grid.width-1 && grid.tiles[GXYIndex(x+1,y)].playerNum >= 0 && patoms>=(s16)grid.tiles[GXYIndex(x-1,y)].atomCount-(s16)critGrid[GXYIndex(x-1,y)])
        ccval++;
    u8 cposx = ccdCheckTab[curIndex] >> 8;
    u8 cposy = ccdCheckTab[curIndex] & 0xFF;
    if(aiIsTileEnemy(cposx,cposy) && grid.tiles[curIndex].atomCount == grid.tiles[GXYIndex(cposx,cposy)].atomCount-2)
        return FALSE;
    return (bool)(ccval >= 2);
}

//Returns TRUE if any neighboring tile that has other player's atoms only needs one atom to become critical
bool aiCheckPreCrit(u8 x, u8 y)
{
    if(y>0 && aiIsTileEnemy(x,y-1) && grid.tiles[GXYIndex(x,y-1)].atomCount >= critGrid[GXYIndex(x,y-1)]-1)
        return TRUE;
    if(y<grid.height-1 && aiIsTileEnemy(x,y+1) && grid.tiles[GXYIndex(x,y+1)].atomCount >= critGrid[GXYIndex(x,y+1)]-1)
        return TRUE;
    if(x>0 && aiIsTileEnemy(x-1,y) && grid.tiles[GXYIndex(x-1,y)].atomCount >= critGrid[GXYIndex(x-1,y)]-1)
        return TRUE;
    if(x<grid.width-1 && aiIsTileEnemy(x+1,y) && grid.tiles[GXYIndex(x+1,y)].atomCount >= critGrid[GXYIndex(x+1,y)]-1)
        return TRUE;
    return FALSE;
}

//Get a list of tiles and special tiles (higher priority)
void aiGetSpecialTiles(int difficulty, s16 (*psptiles)[84], s16 (*ptiles)[84], s16* pspcount, s16* ptcount)
{
    s16* sptiles = *psptiles; //Special tiles
    s16* tiles = *ptiles; //Normal tiles
    s16 advtiles[84] = {0}; //Tiles with advantage over another player
    s16 advcount = 0;
    s16 natiles[84] = {0}; //Not avoided tiles
    s16 nacount = 0;
    bool wasSpCorner = FALSE; //TRUE if there was a Special corner
    bool wasAdvCorner = FALSE; //TRUE if there was a corner with Advantage
    s16 corners[4] = {0}; //Array of corners, can replace special tile array
    s16 corcount = 0;
    s16 aicornercount = aiGetCorners(grid.width,grid.height); //Amount of corners the AI has right now
    for(int x=0; x<grid.width; x++)
    {
        for(int y=0; y<grid.height; y++)
        {
            u16 tileIndex = GXYIndex(x,y); //Current checked tile index
            struct Tile* curtile = &grid.tiles[tileIndex];
            if(curtile->playerNum == curPlayer || curtile->playerNum == NOPLAYER) //Atom can be placed on that tile
            {
                bool isSpTile = FALSE;
                bool tileAvoided = FALSE;
                tiles[*ptcount] = PACKCoords(x,y);
                *ptcount += 1;
                natiles[nacount++] = PACKCoords(x,y);
                if(difficulty==2)
                {
                    if(aiCheckPreCrit(x,y)) //Tile is special if you and another player have near-critical atom tiles near each other
                    {
                        if(curtile->atomCount == (critGrid[tileIndex]-1))
                        {
                            sptiles[*pspcount] = PACKCoords(x,y);
                            *pspcount += 1;
                            isSpTile = TRUE;
                        }
                        else //If only another player has a pre-critical atom tile, try to avoid neighboring tiles
                        {
                            nacount--;
                            tileAvoided = TRUE;
                        }
                    }
                }
                else if(difficulty==3)
                {
                    u8 atomcount = curtile->atomCount;
                    bool isCorner = (bool)((x==0 || x==grid.width-1) && (y==0 || y==grid.height-1));
                    if(isCorner && atomcount==1 && !aiAtomsNearby(x,y)) //Avoid corner tiles with your atom and atoms nearby
                    {
                        nacount--;
                        tileAvoided = TRUE;
                    }
                    else if(aiCheckPreCrit(x,y)) //The same as in difficulty 2
                    {
                        if(atomcount == (critGrid[tileIndex]-1))
                        {
                            sptiles[*pspcount] = PACKCoords(x,y);
                            *pspcount += 1;
                            isSpTile = TRUE;
                        }
                        else
                        {
                            nacount--;
                            tileAvoided = TRUE;
                        }
                    }
                    bool isAdvantage = FALSE;
                    if(!isSpTile) //If a tile isn't special, check if you have an atom advantage over another player
                        isAdvantage = aiCheckAdvantage(x,y,(s16)(atomcount)-(s16)(critGrid[tileIndex]));
                    
                    if(!tileAvoided && isCorner && (aicornercount == 0 || aiCornerCheck(x,y))) //Add the corner tile to the corner array
                    {
                        //Corner tile hierarchy: special > advantage > normal
                        if(wasSpCorner && isSpTile) //Only pick special corners if any other tile is special
                        {
                            corners[corcount++] = PACKCoords(x,y);
                        }
                        else if(wasAdvCorner) //Pick special and advantage corners if any other tile has an advantage
                        {
                            if(isSpTile)
                            {
                                corcount=0;
                                wasSpCorner = TRUE;
                            }
                            else if(isAdvantage)
                            {
                                corners[corcount++] = PACKCoords(x,y);
                            }
                        }
                        else
                        {
                            if(isSpTile)
                            {
                                corcount=0;
                                wasSpCorner = TRUE;
                            }
                            else if(isAdvantage)
                            {
                                corcount=0;
                                wasAdvCorner = TRUE;
                            }
                            if(atomcount==0 || isSpTile || isAdvantage) //Only blow up a corner when it's a special tile or it has an advantage
                            {
                                corners[corcount++] = PACKCoords(x,y);
                            }
                        }
                    }
                    else if(isAdvantage && !isCorner && !tileAvoided) //Add advantage tiles
                    {
                        advtiles[advcount++] = PACKCoords(x,y);
                    }
                }
            }
        }
    }
    if(corcount>0) //First priority: corners (only if none claimed or strategically important)
    {
        memcpy(sptiles,corners,corcount*sizeof(s16));
        *pspcount = corcount;
    }
    else if(*pspcount == 0 && advcount > 0) //Second priority: special tiles, third priority: advantage tiles (AI 3 only)
    {
        memcpy(sptiles,advtiles,advcount*sizeof(s16));
        *pspcount = advcount;
    }
    if(nacount > 0) //Otherwise, if there are any not avoided tiles, choose them
    {
        memcpy(tiles,natiles,nacount*sizeof(s16));
        *ptcount = nacount;
    }
}

void aiThinker(void)
{
    u8 tx = 0;
    u8 ty = 0;
    if(aiDifficulty[curPlayer] <= 3)
    {
        s16 sptiles[84] = {0};
        s16 tiles[84] = {0};
        s16 spcount = 0, tcount = 0;
        aiGetSpecialTiles(aiDifficulty[curPlayer],&sptiles,&tiles,&spcount,&tcount);
        if(spcount > 0) //Choose a random corner/special/advantage tile
        {
            u16 rindex = random() % spcount;
            tx = sptiles[rindex] >> 8;
            ty = sptiles[rindex] & 0xFF;
        }
        else //Choose any random tile
        {
            u16 rindex = random() % tcount;
            tx = tiles[rindex] >> 8;
            ty = tiles[rindex] & 0xFF;
        }
    }
    else
    {
        SYS_die("Incorrect AI difficulty");
    }
    logic_clickedTile(tx,ty,TRUE);
}

//Initialize AI corner check array and reset AI timer
void ai_init(void)
{
    aiTime = 0;
    ccdCheckTab[GXYIndex(0,0)] = PACKCoords(1,1);
    ccdCheckTab[GXYIndex(0,grid.height-1)] = PACKCoords(1,grid.height-2);
    ccdCheckTab[GXYIndex(grid.width-1,0)] = PACKCoords(grid.width-2,1);
    ccdCheckTab[GXYIndex(grid.width-1,grid.height-1)] = PACKCoords(grid.width-2,grid.height-2);
}

//Reset AI timer, used by nextPlayer()
void ai_resetTime(void)
{
    aiTime = 0;
}

//Increase AI timer and try to move AI
void ai_tryMove(fix32 dt)
{
    aiTime += dt;
    if(aiTime >= aiDelay)
    {
        aiThinker();
    }
}