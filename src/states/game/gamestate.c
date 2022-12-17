#include "gamestate.h"
#include "../../data.h"
#include "../../../res/resources.h"
#include "gamelogic.h"
#include "../../save.h"
#include "gameai.h"

s16 selectx = 0; //Selected grid tile X position
s16 selecty = 0; //Selected grid tile Y position

bool pauseOptDown = FALSE; //If TRUE, selected pause option is on the bottom row
bool pauseOptRight = FALSE; //If TRUE, selected pause option is on the right column

#define GETPAUSEOPT(down,right) (((down)<<1) | (right))

Sprite* selector; //Grid selector sprite

fix32 startTime = 0; //Despite the name, it's actually the in-game timer in seconds

bool pausing = FALSE; //Is the game waiting to be paused?

bool isPaused = FALSE;

VidImagePtr vidImgPlayer; //Player icons
VidImagePtr vidImgBorderH; //Horizontal grid border
VidImagePtr vidImgBorderV; //Vertical grid border
VidImagePtr vidImgPauseAtom; //Pause menu selection image

u16 gamePlayerJoys[4] = {0,0,0,0}; //Controller ID for each player (ID is controller number - 1)

extern bool aiPlayerTab[4];

void setupGamePalettes(bool oldColors)
{
    //PAL0 (Red)
    newPalette[0] = RGB24_TO_VDPCOLOR(0x000000);
    newPalette[1] = RGB24_TO_VDPCOLOR(0x444444);
    newPalette[2] = RGB24_TO_VDPCOLOR(0xEEEEEE);
    newPalette[3] = RGB24_TO_VDPCOLOR(0xA8A0A8);
    if(oldColors)
    {
        newPalette[5] = RGB24_TO_VDPCOLOR(0xF80048);
        newPalette[6] = RGB24_TO_VDPCOLOR(0xC82448);
        newPalette[7] = RGB24_TO_VDPCOLOR(0xF80048);
        newPalette[8] = RGB24_TO_VDPCOLOR(0xC82448);
    }
    else
    {
        newPalette[5] = RGB24_TO_VDPCOLOR(0xEE0000);
        newPalette[6] = RGB24_TO_VDPCOLOR(0xCC2200);
        newPalette[7] = RGB24_TO_VDPCOLOR(0xEE0000);
        newPalette[8] = RGB24_TO_VDPCOLOR(0xCC2200);
    }
    

    //PAL1 (Blue 17-22 + Explosion 29-31)
    newPalette[17] = RGB24_TO_VDPCOLOR(0x444444);
    newPalette[18] = RGB24_TO_VDPCOLOR(0xEEEEEE);
    newPalette[19] = RGB24_TO_VDPCOLOR(0xA8A0A8);
    if(oldColors)
    {
        newPalette[21] = RGB24_TO_VDPCOLOR(0x00B4F8);
        newPalette[22] = RGB24_TO_VDPCOLOR(0x2090F8);
    }
    else
    {
        newPalette[21] = RGB24_TO_VDPCOLOR(0x0000EE);
        newPalette[22] = RGB24_TO_VDPCOLOR(0x0000CC);
    }
    newPalette[29] = RGB24_TO_VDPCOLOR(0xF8FC48);
    newPalette[30] = RGB24_TO_VDPCOLOR(0xF8D820);
    newPalette[31] = RGB24_TO_VDPCOLOR(0xD80020);

    //PAL2 (Green)
    newPalette[33] = RGB24_TO_VDPCOLOR(0x444444);
    newPalette[34] = RGB24_TO_VDPCOLOR(0xEEEEEE);
    newPalette[35] = RGB24_TO_VDPCOLOR(0xA8A0A8);
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
    newPalette[49] = RGB24_TO_VDPCOLOR(0x444444);
    newPalette[50] = RGB24_TO_VDPCOLOR(0xEEEEEE);
    newPalette[51] = RGB24_TO_VDPCOLOR(0xA8A0A8);
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
}

//Wrap selection cursor around if it goes out of bounds
void fixSelPos(void)
{
    if(selectx<0)
        selectx = grid.width-1;
    else if(selectx>=grid.width)
        selectx = 0;
    
    if(selecty<0)
        selecty = grid.height-1;
    else if(selecty>=grid.height)
        selecty = 0;
}

//Draw the atom image next to the selected option and clear other spots where it can be
void drawPauseSelPos()
{
    VDP_clearTileMapRect(BG_B,5,2,1,1);
    VDP_clearTileMapRect(BG_B,23,2,1,1);
    VDP_clearTileMapRect(BG_B,5,4,1,1);
    VDP_clearTileMapRect(BG_B,23,4,1,1);
    VDP_drawImageEx(BG_B,vidImgPauseAtom->img,TILE_ATTR_FULL(PAL0,1,FALSE,FALSE,vidImgPauseAtom->vPos),5+(18*pauseOptRight),2+(2*pauseOptDown),FALSE,TRUE);
}

//Draw the pause menu
void pauseDraw(void)
{
    VDP_clearPlane(BG_A,TRUE);
    VDP_clearTileMapRect(BG_B,0,0,40,6);
    SPR_setVisibility(selector,HIDDEN);
    const char* strPtr;
    strPtr = "PAUSED";
    VDP_setTextPriority(1);
    VDP_setHilightShadow(TRUE);
    VDP_drawText(strPtr,GETCENTERX(strPtr),0);
    VDP_drawText("Resume",7,2);
    VDP_drawText("Save & Quit",25,2);
    VDP_drawText("Go to menu",7,4);
    VDP_drawText("Restart",25,4);
    drawPauseSelPos();
}

//Draw the grid (if drawgrid==TRUE), player icons and AI difficulty information
void gamePreDraw(bool drawgrid)
{
    VDP_setHilightShadow(FALSE);
    VDP_setTextPriority(0);
    VDP_clearPlane(BG_A,TRUE);
    VDP_drawText(">",5+8*curPlayer,3);
    VDP_drawText("<",8+8*curPlayer,3);
    for(int i=0; i<4; i++)
    {
        if(aiPlayerTab[i]) //Draw AI difficulty information
        {
            char aistr[8];
            sprintf(aistr,"AI %d",aiDifficulty[i]);
            VDP_clearText(5+8*i,5,4);
            VDP_drawText(aistr,5+8*i,5);
        }
    }
    if(drawgrid)
    {
        for(int x=0; x<grid.width; x++) //Draw tiles
        {
            for(int y=0; y<grid.height; y++)
            {
                struct Tile* ttile = &grid.tiles[GXYIndex(x,y)];
                drawTile(x,y,ttile->playerNum & 3,ttile->atomCount);
            }
        }
        for(int x=0; x<grid.width; x++) //Draw grid borders on the left side
        {
            VDP_drawImageEx(BG_B,vidImgBorderH->img,TILE_ATTR_FULL(PAL0,0,FALSE,FALSE,vidImgBorderH->vPos),(x*3)+gridStartX,gridStartY-1,FALSE,TRUE);
        }
        for(int y=0; y<grid.height; y++) //Draw grid borders on the top side
        {
            VDP_drawImageEx(BG_B,vidImgBorderV->img,TILE_ATTR_FULL(PAL0,0,FALSE,FALSE,vidImgBorderV->vPos),gridStartX-1,(y*3)+gridStartY,FALSE,TRUE);
        }
    }

    if(!logicEnd) //Don't draw player icons when the game has ended before it started (less than 2 players or lack of controllers)
    {
        for(int i=0; i<4; i++)
        {
            VDP_drawImageEx(BG_B,vidImgPlayer->img,TILE_ATTR_FULL(i,0,FALSE,FALSE,vidImgPlayer->vPos),6+8*i,1,FALSE,TRUE);
        }
    }
}

void gamestate_init(void)
{
    memset(gamePlayerJoys,0,sizeof(gamePlayerJoys));
    vidImgPauseAtom = reserveVImage(&texSelAtom);
    vidImgPlayer = reserveVImage(&texPlayer);
    vidImgBorderH = reserveVImage(&texBorderH);
    vidImgBorderV = reserveVImage(&texBorderV);
    setupGamePalettes(settings.useOldColors);
    selectx = 0;
    selecty = 0;
    startTime = 0;
    pausing = FALSE;
    isPaused = FALSE;
    if(!isDemoPlaying) //Initialize all the data and try to load a savegame
    {
        u8 pttab[4] = {settings.player1,settings.player2,settings.player3,settings.player4};
        logic_loadAll(settings.gridWidth,settings.gridHeight,&pttab);
        fix32 ttime = loadGameData();
        if(ttime!=-1) //If game has loaded correctly
        {
            setupGamePalettes(settings.useOldColors);
            startTime = ttime;
            logic_fixLoadedData();
        }
    }
    else //Demo mode, generate "random" grid, set player count and AI difficulty
    {
        u8 pttab[4] = {0,0,0,0};
        s16 rpcount = (random() % 3) + 2; // 2-4
        for(int i=0; i<rpcount; i++)
        {
            pttab[i] = (random() % 3) + 2; // 2-4
        }
        s16 rgh = (random() & 3) + 4; // 4-7
        s16 rgw = (random() & 7) + 5; // 5-12
        logic_loadAll(rgw,rgh,&pttab);
    }
    gamePreDraw(TRUE);
    selector = SPR_addSpriteSafe(&sprSel,8,48,TILE_ATTR(curPlayer,TRUE,FALSE,FALSE));
    if(isDemoPlaying)
        SPR_setVisibility(selector,HIDDEN);
    
    if(!settings.isHotSeat) //Multiple controller mode
    {
        JOY_reset();
        u16 joyindex = 0;
        for(int i=0; i<4; i++)
        {
            if(playerTab[i]==PTAB_PLAY && !aiPlayerTab[i])
            {
                if(JOY_getPortType(joyindex)!=JOY_TYPE_UNKNOWN) //Give the player an appropriate controller
                {
                    gamePlayerJoys[i] = joyindex;
                    joyindex++;
                }
                else
                {
                    char nojoymsg[36];
                    sprintf(nojoymsg,"Player %s: No controller %d",logic_getPlayerName(i),joyindex+1);
                    logic_endMessage(nojoymsg);
                    return;
                }
            }
        }
    }
}

void gamestate_update(fix32 dt)
{
    if(isPaused) //No code in this function runs when the game is paused
    {
        return;
    }
    else if(pausing && asPos==0 && !animPlaying && !logicEnd) //Only pause between turns, otherwise wait
    {
        isPaused = TRUE;
        pausing = FALSE;
        pauseDraw();
        return;
    }
    SPR_setPalette(selector,curPlayer);
    if(animPlaying||asPos>0||aiPlayerTab[curPlayer])
        SPR_setVisibility(selector,HIDDEN);
    else
        SPR_setVisibility(selector,AUTO_FAST);
    s16 px,py;
    tileToPixels(selectx,selecty,&px,&py);
    SPR_setPosition(selector,px-5,py-5); //Update selection box position
    if(!logicEnd)
    {
        if(playerWon<0) //No player has won yet, proceed as normal
        {
            startTime += dt;
            s32 sectime = fix32ToInt(startTime) & 0x1FFFFF;
            if(isDemoPlaying)
            {
                const char* demoStr="DEMO - Press any button";
                if(sectime & 1)
                    VDP_drawText(demoStr,GETCENTERX(demoStr),0);
                else
                    VDP_clearText(GETCENTERX(demoStr),0,strlen(demoStr));
            }
            else
            {
                s32 dresult = divmods(sectime,60);
                s16 minutes = dresult & 0xFFFF;
                s16 seconds = dresult >> 16;
                char timebuf[9];
                sprintf(timebuf,"%02d:%02d",minutes,seconds);
                s16 tblen = strlen(timebuf);
                VDP_drawText(timebuf,39-tblen,0);
            }
            logic_tick(dt);
        }
        else //A player has won the game
        {
            if(isDemoPlaying)
            {
                isDemoPlaying = FALSE;
                changeState(ST_TITLESTATE);
                return;
            }
            char wonstr[24];
            sprintf(wonstr,"Player %s won!",logic_getPlayerName(playerWon));
            logic_endMessage(wonstr);
            s32 secTime = fix32ToInt(startTime) & 0x1FFFFF;
            s32 dresult = divmods(secTime,60);
            s16 minutes = dresult & 0xFFFF;
            s16 seconds = dresult >> 16;
            char timebuf[16];
            sprintf(timebuf,"Time: %02d:%02d",minutes,seconds);
            VDP_drawText(timebuf,20-(strlen(timebuf)>>1),4);
        }
    }
    logic_draw(dt);
}

void gamestate_joyevent(u16 joy, u16 changed, u16 state)
{
    if(state & changed)
    {
        if(logicEnd || isDemoPlaying) //If the game has ended or a demo is playing, wait for any button press
        {
            isDemoPlaying = FALSE;
            changeState(ST_MENUSTATE);
            return;
        }

        if(isPaused)
        {
            s16 pauseOpt=0;
            switch(changed)
            {
                case BUTTON_UP:
                case BUTTON_DOWN:
                    {
                        pauseOptDown = !pauseOptDown;
                        drawPauseSelPos();
                        XGM_stopPlayPCM(SOUND_PCM_CH2);
                        XGM_startPlayPCM(SFX_CLICK,0,SOUND_PCM_CH2);
                    }
                    break;
                case BUTTON_LEFT:
                case BUTTON_RIGHT:
                    {
                        pauseOptRight = !pauseOptRight;
                        drawPauseSelPos();
                        XGM_stopPlayPCM(SOUND_PCM_CH2);
                        XGM_startPlayPCM(SFX_CLICK,0,SOUND_PCM_CH2);
                    }
                    break;
                case BUTTON_A:
                case BUTTON_B:
                case BUTTON_C:
                case BUTTON_START:
                    {
                        pauseOpt = GETPAUSEOPT(pauseOptDown,pauseOptRight);
                        switch(pauseOpt)
                        {
                            case 0:
                                isPaused = FALSE;
                                VDP_clearTileMapRect(BG_B,5,2,1,1);
                                gamePreDraw(FALSE);
                                break;
                            case 1:
                                saveGameData();
                                changeState(ST_MENUSTATE);
                                break;
                            case 2:
                                changeState(ST_MENUSTATE);
                                break;
                            case 3:
                                changeState(ST_GAMESTATE);
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
        else if(!pausing && (joy==gamePlayerJoys[curPlayer] || aiPlayerTab[curPlayer]))
        {
            switch(changed)
            {
                case BUTTON_UP:
                    if(!animPlaying)
                        selecty--;
                    fixSelPos();
                    break;
                case BUTTON_DOWN:
                    if(!animPlaying)
                        selecty++;
                    fixSelPos();
                    break;
                case BUTTON_LEFT:
                    if(!animPlaying)
                        selectx--;
                    fixSelPos();
                    break;
                case BUTTON_RIGHT:
                    if(!animPlaying)
                        selectx++;
                    fixSelPos();
                    break;
                case BUTTON_A:
                case BUTTON_B:
                case BUTTON_C:
                    logic_clickedTile(selectx,selecty,FALSE);
                    break;
                case BUTTON_START:
                    VDP_drawText("Pausing...",1,0);
                    pausing = TRUE;
                    pauseOptDown = FALSE;
                    pauseOptRight = FALSE;
                    break;
                default:
                    break;
            }
        }
        else if(changed==BUTTON_START) //Abort pausing
        {
            VDP_clearText(1,0,10);
            pausing = FALSE;
        }
    }
}

void gamestate_stop(void)
{
    if(selector)
    {
        SPR_releaseSprite(selector);
        selector = NULL;
    }
    logic_stop();
    //Reset pause menu shadow
    VDP_setTextPriority(0);
    VDP_setHilightShadow(FALSE);
}
