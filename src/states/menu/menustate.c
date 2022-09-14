#include "menustate.h"
#include "../../data.h"
#include "../../save.h"
#include "../../../res/resources.h"

bool isAboutPage = FALSE;

s16 menuSel = 0;

#define RIGHTALIGNX(str,x) ((x)+1-strlen(str))

#define OPTIONCOUNT 11

#define OPTVALPOS 36 //Position where option values end (in 8x8 pixel tiles)

const char* menuTitle = "MAIN MENU";

Image* menuBackground;

Sprite* menuSelSprites[2]; //Menu selection sprites

fix32 menuBGScroll = 0;

enum ActionType {
    AT_LEFT,
    AT_RIGHT,
    AT_PRESS
};

const char* optNames[OPTIONCOUNT] = {
    "Start the game",
    "Grid width",
    "Grid height",
    "Player 1 type",
    "Player 2 type",
    "Player 3 type",
    "Player 4 type",
    "Player colors",
    "Multiple controllers",
    "Reset settings",
    "About"
};

const char* ptnames[5] = { //Player type names
    "None",
    "Human",
    "AI 1",
    "AI 2",
    "AI 3"
};

void setupMenuPalette(bool oldColors)
{
    newPalette[0] = RGB24_TO_VDPCOLOR(0x002266);
    newPalette[2] = RGB24_TO_VDPCOLOR(0xEEEEEE);
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
    memcpy(&newPalette[16],menuBackground->palette->data,sizeof(u16)*menuBackground->palette->length);
}

//Wrap around the menu selection if needed and play sound
void fixMenuSelPos(void)
{
    if(menuSel>=OPTIONCOUNT)
        menuSel=0;
    else if(menuSel<0)
        menuSel=OPTIONCOUNT-1;
    XGM_stopPlayPCM(SOUND_PCM_CH2);
    XGM_startPlayPCM(SFX_CLICK,0,SOUND_PCM_CH2);
}

//Draw option value in the proper position
void drawValStr(int index, char* valuestr)
{
    VDP_clearText(OPTVALPOS-12,4+(index<<1),15);
    VDP_drawText(valuestr,RIGHTALIGNX(valuestr,OPTVALPOS),4+(index<<1));
}

//Update option value with given index
void updateVal(int index)
{
    char tempstr[24];
    switch(index)
    {
        case 1:
            sprintf(tempstr,"%d",settings.gridWidth);
            break;
        case 2:
            sprintf(tempstr,"%d",settings.gridHeight);
            break;
        case 3:
            sprintf(tempstr,"%s",ptnames[settings.player1]);
            break;
        case 4:
            sprintf(tempstr,"%s",ptnames[settings.player2]);
            break;
        case 5:
            sprintf(tempstr,"%s",ptnames[settings.player3]);
            break;
        case 6:
            sprintf(tempstr,"%s",ptnames[settings.player4]);
            break;
        case 7:
            sprintf(tempstr,"%s",(settings.useOldColors) ? "Original" : "Default");
            break;
        case 8:
            sprintf(tempstr,"%s",(settings.isHotSeat) ? "No" : "Yes");
            break;
        default:
            return;
            break;
    }
    drawValStr(index,tempstr);
}

void drawMenu(void)
{
    optNames[0] = (saveValid) ? "Load saved game" : "Start the game";
    VDP_clearPlane(BG_A,TRUE);
    SPR_setVisibility(menuSelSprites[0],AUTO_FAST);
    SPR_setVisibility(menuSelSprites[1],AUTO_FAST);
    VDP_drawText(menuTitle,GETCENTERX(menuTitle),1);
    for(int i=0; i<OPTIONCOUNT; i++)
    {
        VDP_drawText(optNames[i],3,4+(i<<1));
        updateVal(i);
    }
}

//Draw About screen
void drawAbout(void)
{
    const char* strPtr;
    VDP_clearPlane(BG_A,TRUE);
    SPR_setVisibility(menuSelSprites[0],HIDDEN);
    SPR_setVisibility(menuSelSprites[1],HIDDEN);
    VDP_drawText("ABOUT",GETCENTERX("ABOUT"),1);
    char tempstr[32];
    sprintf(tempstr,"KleleAtoms MD %s",versionStr);
    VDP_drawText(tempstr,GETCENTERX(tempstr),4);
    strPtr = "MegaDrive/Genesis port of KleleAtoms";
    VDP_drawText(strPtr,GETCENTERX(strPtr),6);
    strPtr = "Made by Nightwolf-47";
    VDP_drawText(strPtr,GETCENTERX(strPtr),8);
    strPtr = "Title screen image made by GreffMASTER";
    VDP_drawText(strPtr,GETCENTERX(strPtr),10);
    VDP_drawText("Software used:",1,14);
    VDP_drawText("SGDK 1.70 - Compiler/Development",1,16);
    VDP_drawText("GIMP 2.10 - Graphics",1,18);
    VDP_drawText("SFXR      - Sounds",1,20);
    strPtr = "Press any button to go back";
    VDP_drawText(strPtr,GETCENTERX(strPtr),26);
}

//Move menu option value with min and max values and return it
u8 moveOption(u8 orval, s16 valmin, s16 valmax, bool moveback)
{
    s16 val=orval;
    if(moveback)
    {
        val--;
        if(val<valmin)
            val=valmax;
    }   
    else
    {
        val++;
        if(val>valmax)
            val=valmin;
    }
    return (u8)val;
}

//Move menu option (version made specifically for player type options, with player amount checks)
u8 moveOptionPlayer(u8 orval, bool moveBack)
{
    u8 newval = moveOption(orval,0,4,moveBack);
    int pcount = 0;
    if(settings.player1>0)
        pcount++;
    if(settings.player2>0)
        pcount++;
    if(settings.player3>0)
        pcount++;
    if(settings.player4>0)
        pcount++;

    if(orval!=0 && pcount<3 && newval==0)
        return moveOption(orval,1,4,moveBack);
    
    return newval;
}

//Handles menu actions except moving up and down
void menuOptionAction(enum ActionType at)
{
    bool isOptDecreasing = (bool)(at==AT_LEFT);
    switch(menuSel)
    {
        case 0:
            if(at==AT_PRESS)
            {
                changeState(ST_GAMESTATE);
            }
            return;
            break;
        case 1:
            settings.gridWidth = moveOption(settings.gridWidth,7,12,isOptDecreasing);
            break;
        case 2:
            settings.gridHeight = moveOption(settings.gridHeight,4,7,isOptDecreasing);
            break;
        case 3:
            settings.player1 = moveOptionPlayer(settings.player1,isOptDecreasing);
            break;
        case 4:
            settings.player2 = moveOptionPlayer(settings.player2,isOptDecreasing);
            break;
        case 5:
            settings.player3 = moveOptionPlayer(settings.player3,isOptDecreasing);
            break;
        case 6:
            settings.player4 = moveOptionPlayer(settings.player4,isOptDecreasing);
            break;
        case 7:
            settings.useOldColors = !settings.useOldColors;
            setupMenuPalette(settings.useOldColors);
            PAL_setColors(0,newPalette,64,CPU);
            break;
        case 8:
            settings.isHotSeat = !settings.isHotSeat;
            break;
        case 9:
            if(at==AT_PRESS)
            {
                data_reset();
                setupMenuPalette(settings.useOldColors);
                PAL_setColors(0,newPalette,64,CPU);
                drawMenu();
            }
            else
            {
                return;
            }
            break;
        case 10:
            if(at==AT_PRESS)
            {
                isAboutPage = TRUE;
                drawAbout();
            }
            else
            {
                return;
            }
            break;
        default:
            break;
    }
    XGM_stopPlayPCM(SOUND_PCM_CH2);
    XGM_startPlayPCM(SFX_CLICK,0,SOUND_PCM_CH2);
    updateVal(menuSel);
}

void menustate_init(void)
{
    menuSelSprites[0] = SPR_addSpriteSafe(&sprAtom,6,24,TILE_ATTR(PAL0,0,FALSE,FALSE));
    menuSelSprites[1] = SPR_addSpriteSafe(&sprAtom,304,24,TILE_ATTR(PAL0,0,FALSE,FALSE));
    if(!menuSelSprites[0] || !menuSelSprites[1])
    {
        SYS_die("Couldn't load selector sprites");
    }
    menuBackground = unpackImage(&texMenuBG,NULL);
    setupMenuPalette(settings.useOldColors);
    VDP_drawImageEx(BG_B,menuBackground,TILE_ATTR_FULL(PAL1,FALSE,FALSE,FALSE,TILE_USERINDEX),0,0,FALSE,TRUE);
    isAboutPage = FALSE;
    drawMenu();
}

void menustate_update(fix32 dt)
{
    menuBGScroll += dt*20;
    menuBGScroll &= 0x7FFFF;
    VDP_setHorizontalScroll(BG_B,-fix32ToInt(menuBGScroll));
    if(!isAboutPage) //Update selection sprite positions
    {
        SPR_setPosition(menuSelSprites[0],6,32+(menuSel<<4));
        SPR_setPosition(menuSelSprites[1],304,32+(menuSel<<4));
    }
}

void menustate_joyevent(u16 joy, u16 changed, u16 state)
{
    if(joy==JOY_1 && (state & changed))
    {
        if(!isAboutPage)
        {
            switch(changed)
            {
                case BUTTON_UP:
                    menuSel--;
                    fixMenuSelPos();
                    break;
                case BUTTON_DOWN:
                    menuSel++;
                    fixMenuSelPos();
                    break;
                case BUTTON_LEFT:
                    menuOptionAction(AT_LEFT);
                    break;
                case BUTTON_RIGHT:
                    menuOptionAction(AT_RIGHT);
                    break;
                case BUTTON_A:
                case BUTTON_B:
                case BUTTON_C:
                case BUTTON_START:
                    menuOptionAction(AT_PRESS);
                    break;
                default:
                    break;
            }
        }
        else
        {
            isAboutPage = FALSE;
            drawMenu();
            XGM_stopPlayPCM(SOUND_PCM_CH2);
            XGM_startPlayPCM(SFX_CLICK,0,SOUND_PCM_CH2);
        }
    }
}

void menustate_stop(void)
{
    VDP_setHorizontalScroll(BG_B,0);
    if(menuSelSprites[0])
    {
        SPR_releaseSprite(menuSelSprites[0]);
        SPR_releaseSprite(menuSelSprites[1]);
    }
    if(menuBackground)
    {
        MEM_free(menuBackground);
    }
    saveSettings();
}
