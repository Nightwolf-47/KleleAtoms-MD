#include "menustate.h"
#include "../../data.h"
#include "../../save.h"
#include "../../../res/resources.h"
#include "../../mouse.h"
#include "menuatoms.h"

static const fix32 initButtonTimer = FIX32(-0.15);
static const fix32 actionButtonTimer = FIX32(0.2);

static fix32 idleTimer;

static const fix32 maxIdleTime = FIX32(5*60); //If idle for that time (5 minutes), return to title screen

typedef struct MenuButton {
    s16 x;
    s16 y;
    bool small;
    VidImagePtr icon;
    const char* description;
} MenuButton;

typedef struct ButtonPressed {
    bool pressed;
    bool continuous;
    s8 direction;
    fix32 timer;
} ButtonPressed;

enum MenuButtonNames {
    MEB_TUTORIAL = 0,
    MEB_START,
    MEB_PLAYER1,
    MEB_PLAYER2,
    MEB_GRIDWIDTH,
    MEB_GRIDHEIGHT,
    MEB_PLAYER3,
    MEB_PLAYER4,
    MEB_MULTICONTROLLER,
    MEB_COLORMODE,
    MEB_ABOUT,

    MENU_BUTTON_COUNT
};

static bool isInit;

static int selectedButton = MEB_START;
static ButtonPressed selectedPressed;

static VidImagePtr buttonImg;
static VidImagePtr buttonSmImg;

static VidImagePtr multiIconImg;
static VidImagePtr noMultiIconImg;

static VidImagePtr menuTextImg;

static MenuButton menuButtons[MENU_BUTTON_COUNT] = {
    {3,7,FALSE,NULL,"Open the tutorial."},              // Tutorial
    {15,7,FALSE,NULL,"Start the game."},                // Play/Load
    {27,7,TRUE,NULL,"Player 1 type"},                   // Player 1
    {33,7,TRUE,NULL,"Player 2 type"},                   // Player 2
    {3,13,FALSE,NULL,"Set grid width. (5-12)"},         // Grid Width
    {15,13,FALSE,NULL,"Set grid height. (4-7)"},        // Grid Height
    {27,13,TRUE,NULL,"Player 3 type"},                  // Player 3
    {33,13,TRUE,NULL,"Player 4 type"},                  // Player 4
    {3,19,FALSE,NULL,"Use multiple controllers."},      // Multi-Controller
    {15,19,FALSE,NULL,"Use default in-game colors."},   // Color mode
    {28,19,FALSE,NULL,"Open the about menu."},          // About menu
};

const char* playerTypeNames[5] = {
    "Nothing",
    "Human",
    "AI 1",
    "AI 2",
    "AI 3"
};

//PAL_setColor equivalent that works both during initialization and after it
static void setPaletteColor(u16 index, u16 value)
{
    if(isInit)
        newPalette[index] = value;
    else
        PAL_setColor(index,value);
}

//PAL_getColor equivalent that works both during initialization and after it
static u16 getPaletteColor(u16 index)
{
    if(isInit)
        return newPalette[index];
    else
        return PAL_getColor(index);
}

//Resets the player icon palette to show every color (like on AI 3)
static void resetPlayerPalette(u8 index, u8 palStartIndex)
{
    setPaletteColor(palStartIndex,RGB24_TO_VDPCOLOR(0x000000));
    setPaletteColor(palStartIndex+1,RGB24_TO_VDPCOLOR(0x00EE00));
    setPaletteColor(palStartIndex+2,RGB24_TO_VDPCOLOR(0xCCCC00));
    setPaletteColor(palStartIndex+3,RGB24_TO_VDPCOLOR(0xEE0000));
    setPaletteColor(palStartIndex+4,getPaletteColor(index*16+3));
}

//Updates the player palette to show the current player type
static void updatePlayerPalette(u8 index)
{
    u8 playerVal;
    switch(index)
    {
        case 0:
            playerVal = settings.player1;
            break;
        case 1:
            playerVal = settings.player2;
            break;
        case 2:
            playerVal = settings.player3;
            break;
        case 3:
            playerVal = settings.player4;
            break;
        default:
            return;
    }
    u8 palStartIndex = index*16+10;
    resetPlayerPalette(index,palStartIndex);
    
    if(playerVal == 0)
        setPaletteColor(palStartIndex+4,RGB24_TO_VDPCOLOR(0x666666));
    
    u16 bgColor = (playerVal < 2) ? getPaletteColor(palStartIndex+4) : RGB24_TO_VDPCOLOR(0x000000);

    if(playerVal < 2)
    {
        setPaletteColor(palStartIndex,bgColor);
        setPaletteColor(palStartIndex+1,bgColor);
    }
    if(playerVal < 3)
        setPaletteColor(palStartIndex+2,bgColor);
    if(playerVal < 4)
        setPaletteColor(palStartIndex+3,bgColor);
}

//Initalizes the menu palette (only use during menu initialization)
static void setupMenuPalette(bool oldColors)
{
    memcpy(newPalette,texButton.palette->data,sizeof(u16)*texButton.palette->length);
    memcpy(&newPalette[16],texPlayerIcon.palette->data,sizeof(u16)*texPlayerIcon.palette->length);
    memcpy(&newPalette[32],texPlayerIcon.palette->data,sizeof(u16)*texPlayerIcon.palette->length);
    memcpy(&newPalette[48],sprCursor.palette->data,sizeof(u16)*sprCursor.palette->length);
    newPalette[0] = RGB24_TO_VDPCOLOR(0x002266);
    //PAL0 (Red)
    if(oldColors)
    {
        newPalette[1] = RGB24_TO_VDPCOLOR(0xF80048);
        newPalette[3] = newPalette[14] = RGB24_TO_VDPCOLOR(0xC82448);
    }
    else
    {
        newPalette[1] = RGB24_TO_VDPCOLOR(0xEE0000);
        newPalette[3] = newPalette[14] = RGB24_TO_VDPCOLOR(0xCC2200);
    }
    newPalette[2] = RGB24_TO_VDPCOLOR(0xEEEEEE);
    newPalette[10] = RGB24_TO_VDPCOLOR(0x000000);
    
    //PAL1 (Blue)
    if(oldColors)
    {
        newPalette[17] = RGB24_TO_VDPCOLOR(0x00B4F8);
        newPalette[19] = newPalette[30] = RGB24_TO_VDPCOLOR(0x2090F8);
    }
    else
    {
        newPalette[17] = RGB24_TO_VDPCOLOR(0x0022EE);
        newPalette[19] = newPalette[30] = RGB24_TO_VDPCOLOR(0x0000EE);
    }
    newPalette[18] = RGB24_TO_VDPCOLOR(0xEEEEEE);
    newPalette[26] = RGB24_TO_VDPCOLOR(0x000000);

    //PAL2 (Green)
    if(oldColors)
    {
        newPalette[33] = RGB24_TO_VDPCOLOR(0x48FC00);
        newPalette[35] = newPalette[46] = RGB24_TO_VDPCOLOR(0x66CC22);
    }
    else
    {
        newPalette[33] = RGB24_TO_VDPCOLOR(0x00EE00);
        newPalette[35] = newPalette[46] = RGB24_TO_VDPCOLOR(0x00CC00);
    }
    newPalette[34] = RGB24_TO_VDPCOLOR(0xEEEEEE);
    newPalette[42] = RGB24_TO_VDPCOLOR(0x000000);

    //PAL3 (Yellow)
    if(oldColors)
    {
        newPalette[49] = RGB24_TO_VDPCOLOR(0xF8FC48);
        newPalette[51] = newPalette[62] = RGB24_TO_VDPCOLOR(0xEECC44);
    }
    else
    {
        newPalette[49] = RGB24_TO_VDPCOLOR(0xEECC00);
        newPalette[51] = newPalette[62] = RGB24_TO_VDPCOLOR(0xEEAA00);
    }
    newPalette[50] = RGB24_TO_VDPCOLOR(0xEEEEEE);
    newPalette[58] = RGB24_TO_VDPCOLOR(0x000000);
    newPalette[15] = RGB24_TO_VDPCOLOR(0x000000);
    newPalette[31] = RGB24_TO_VDPCOLOR(0xFFFFFF);
    newPalette[47] = RGB24_TO_VDPCOLOR(0xFFA500);

    for(u16 i=0; i<4; i++)
    {
        updatePlayerPalette(i);
    }
}

//Updates menu colors when the colors option is selected
static void updateMenuColors(bool oldColors)
{
    //PAL0 (Red)
    if(oldColors)
    {
        PAL_setColor(1,RGB24_TO_VDPCOLOR(0xF80048));
        PAL_setColor(3,RGB24_TO_VDPCOLOR(0xC82448));
        PAL_setColor(14,RGB24_TO_VDPCOLOR(0xC82448));
    }
    else
    {
        PAL_setColor(1,RGB24_TO_VDPCOLOR(0xEE0000));
        PAL_setColor(3,RGB24_TO_VDPCOLOR(0xCC2200));
        PAL_setColor(14,RGB24_TO_VDPCOLOR(0xCC2200));
    }

    //PAL1 (Blue)
    if(oldColors)
    {
        PAL_setColor(17,RGB24_TO_VDPCOLOR(0x00B4F8));
        PAL_setColor(19,RGB24_TO_VDPCOLOR(0x2090F8));
        PAL_setColor(30,RGB24_TO_VDPCOLOR(0x2090F8));
    }
    else
    {
        PAL_setColor(17,RGB24_TO_VDPCOLOR(0x0022EE));
        PAL_setColor(19,RGB24_TO_VDPCOLOR(0x0000EE));
        PAL_setColor(30,RGB24_TO_VDPCOLOR(0x0000EE));
    }

    //PAL2 (Green)
    if(oldColors)
    {
        PAL_setColor(33,RGB24_TO_VDPCOLOR(0x48FC00));
        PAL_setColor(35,RGB24_TO_VDPCOLOR(0x66CC22));
        PAL_setColor(46,RGB24_TO_VDPCOLOR(0x66CC22));
    }
    else
    {
        PAL_setColor(33,RGB24_TO_VDPCOLOR(0x00EE00));
        PAL_setColor(35,RGB24_TO_VDPCOLOR(0x00CC00));
        PAL_setColor(46,RGB24_TO_VDPCOLOR(0x00CC00));
    }

    //PAL3 (Yellow)
    if(oldColors)
    {
        PAL_setColor(49,RGB24_TO_VDPCOLOR(0xF8FC48));
        PAL_setColor(51,RGB24_TO_VDPCOLOR(0xEECC44));
        PAL_setColor(62,RGB24_TO_VDPCOLOR(0xEECC44));
    }
    else
    {
        PAL_setColor(49,RGB24_TO_VDPCOLOR(0xEECC00));
        PAL_setColor(51,RGB24_TO_VDPCOLOR(0xEEAA00));
        PAL_setColor(62,RGB24_TO_VDPCOLOR(0xEEAA00));
    }

    for(u16 i=0; i<4; i++)
    {
        updatePlayerPalette(i);
    }
}

//Draws the icon for the specific button
static void drawMenuIcon(u16 index)
{
    MenuButton* curButton = &menuButtons[index]; 
    u8 palIndex = 1;
    u8 iconWidth = 9;

    switch(index)
    {
        case MEB_PLAYER1:
            palIndex = 0;
            iconWidth = 5;
            break;
        case MEB_PLAYER2:
            palIndex = 1;
            iconWidth = 5;
            break;
        case MEB_PLAYER3:
            palIndex = 2;
            iconWidth = 5;
            break;
        case MEB_PLAYER4:
            palIndex = 3;
            iconWidth = 5;
            break;
        case MEB_GRIDHEIGHT:
        case MEB_GRIDWIDTH:
            iconWidth = 5;
            break;
        case MEB_COLORMODE:
            VDP_setTileMapEx(BG_A,curButton->icon->img->tilemap,TILE_ATTR_FULL(PAL1,1,0,0,curButton->icon->vPos),curButton->x,curButton->y,0,0,3,5,CPU);
            VDP_setTileMapEx(BG_A,curButton->icon->img->tilemap,TILE_ATTR_FULL(PAL0,1,0,0,curButton->icon->vPos),curButton->x+3,curButton->y,3,0,1,5,CPU);
            VDP_setTileMapEx(BG_A,curButton->icon->img->tilemap,TILE_ATTR_FULL(PAL2,1,0,0,curButton->icon->vPos),curButton->x+4,curButton->y,4,0,1,5,CPU);
            VDP_setTileMapEx(BG_A,curButton->icon->img->tilemap,TILE_ATTR_FULL(PAL3,1,0,0,curButton->icon->vPos),curButton->x+5,curButton->y,5,0,1,5,CPU);
            VDP_setTileMapEx(BG_A,curButton->icon->img->tilemap,TILE_ATTR_FULL(PAL1,1,0,0,curButton->icon->vPos),curButton->x+6,curButton->y,6,0,3,5,CPU);
            return;
        default:
            break;
    }
    VDP_setTileMapEx(BG_A,curButton->icon->img->tilemap,TILE_ATTR_FULL(palIndex,1,0,0,curButton->icon->vPos),curButton->x,curButton->y,0,0,iconWidth,5,CPU);
}

// Draws the button with specified parameters and optionally its icon
static void drawMenuButton(u16 index, bool drawIcon, bool selected, bool pressed)
{
    MenuButton* curButton = &menuButtons[index];

    u16 button_ry = 0;
    if(pressed)
        button_ry = 10;
    else if(selected)
        button_ry = 5;
    
    if(curButton->small)
        VDP_setTileMapEx(BG_B,buttonSmImg->img->tilemap,TILE_ATTR_FULL(PAL0,1,0,0,buttonSmImg->vPos),curButton->x,curButton->y,0,button_ry,5,5,CPU);
    else
        VDP_setTileMapEx(BG_B,buttonImg->img->tilemap,TILE_ATTR_FULL(PAL0,1,0,0,buttonImg->vPos),curButton->x,curButton->y,0,button_ry,9,5,CPU);
    
    if(drawIcon)
        drawMenuIcon(index);
}

// Draws the description for a given button on the bottom of the screen
static void drawButtonDescription(u16 index)
{
    char buf[41];
    const char* description = menuButtons[index].description;

    switch(index)
    {
        case MEB_PLAYER1:
            sprintf(buf,"%s (%s)",description,playerTypeNames[settings.player1]);
            break;
        case MEB_PLAYER2:
            sprintf(buf,"%s (%s)",description,playerTypeNames[settings.player2]);
            break;
        case MEB_PLAYER3:
            sprintf(buf,"%s (%s)",description,playerTypeNames[settings.player3]);
            break;
        case MEB_PLAYER4:
            sprintf(buf,"%s (%s)",description,playerTypeNames[settings.player4]);
            break;
        case MEB_COLORMODE:
            if(settings.useOldColors)
                description = "Use original in-game colors.";
            memcpy(buf,description,41);
            break;
        case MEB_MULTICONTROLLER:
            if(settings.isHotSeat)
                description = "Use one controller. (Hot Seat)";
            memcpy(buf,description,41);
            break;
        default:
            memcpy(buf,description,41);
            break;
    }

    VDP_clearTextLine(25);
    VDP_setTextPalette(PAL2);
    VDP_drawText(buf,GETCENTERX(buf),25);
    VDP_setTextPalette(PAL0);
}

// Updates the value text of a button with a given index, works only if the button has a value text
static void updateButtonValue(u16 index)
{
    MenuButton* curButton = &menuButtons[index];
    char buf[4];
    switch(index)
    {
        case MEB_GRIDWIDTH:
            sprintf(buf,"%d",settings.gridWidth);
            break;
        case MEB_GRIDHEIGHT:
            sprintf(buf,"%d",settings.gridHeight);
            break;
        default:
            return;
    }
    s16 xpos = curButton->x+6;
    s16 ypos = curButton->y+2;
    VDP_clearText(xpos,ypos,2);
    VDP_drawText(buf,xpos,ypos);
}

// Draws everything in the menu
static void drawMenu(void)
{
    VDP_setTileMapEx(BG_B,menuTextImg->img->tilemap,TILE_ATTR_FULL(PAL0,1,0,0,menuTextImg->vPos),7,1,0,0,25,4,CPU);
    for(u16 i=0; i<MENU_BUTTON_COUNT; i++)
    {
        drawMenuButton(i,TRUE,(i == selectedButton),FALSE);
        updateButtonValue(i);
    }
    drawButtonDescription(selectedButton);
    VDP_setTextPalette(PAL1);
    VDP_drawText(versionStr,0,27);
    VDP_setTextPalette(PAL0);
}

// Sets up button icons, start button description and initalizes button textures
static void setupButtons(void)
{
    buttonImg = reserveVImage(&texButton,TRUE);
    buttonSmImg = reserveVImage(&texSmButton,TRUE);
    multiIconImg = reserveVImage(&texMultiIcon,TRUE);
    noMultiIconImg = reserveVImage(&texNoMultiIcon,TRUE);
    VidImagePtr playerImage = reserveVImage(&texPlayerIcon,TRUE);

    menuButtons[MEB_TUTORIAL].icon = reserveVImage(&texTutorialIcon,TRUE);
    if(saveValid)
    {
        menuButtons[MEB_START].icon = reserveVImage(&texSaveIcon,TRUE);
        menuButtons[MEB_START].description = "Resume a saved game.";
    }
    else
    {
        menuButtons[MEB_START].icon = reserveVImage(&texStartIcon,TRUE);
        menuButtons[MEB_START].description = "Start the game.";
    }
    menuButtons[MEB_PLAYER1].icon = playerImage;
    menuButtons[MEB_PLAYER2].icon = playerImage;
    menuButtons[MEB_GRIDWIDTH].icon = reserveVImage(&texGridWidthIcon,TRUE);
    menuButtons[MEB_GRIDHEIGHT].icon = reserveVImage(&texGridHeightIcon,TRUE);
    menuButtons[MEB_PLAYER3].icon = playerImage;
    menuButtons[MEB_PLAYER4].icon = playerImage;
    menuButtons[MEB_MULTICONTROLLER].icon = (settings.isHotSeat) ? noMultiIconImg : multiIconImg;
    menuButtons[MEB_COLORMODE].icon = reserveVImage(&texColorIcon,TRUE);
    menuButtons[MEB_ABOUT].icon = reserveVImage(&texAboutIcon,TRUE);
}

// Resets button pressed values
static void resetButtonPress(void)
{
    selectedPressed.pressed = FALSE;
    selectedPressed.continuous = FALSE;
    selectedPressed.timer = 0;
    selectedPressed.direction = 0;
}

// Changes the selected button by moving in a given direction and adjusts the button textures
static void moveSelection(bool yAxis, bool leftup)
{
    resetButtonPress();
    drawMenuButton(selectedButton,FALSE,FALSE,FALSE);
    s16 x = selectedButton & 3;
    s16 y = selectedButton >> 2;
    if(yAxis) //Move vertically
    {
        if(leftup) //Move up
        {
            y--;
            if(y < 0)
                y = 2;
        }
        else  //Move down
        {
            y++;
            if(y > 2)
                y = 0;
        }
        if(y == 2 && x == 3)
            x = 2;
    }
    else //Move horizontally
    {
        u16 maxX = (y == 2) ? 2 : 3;
        if(leftup) //Move left
        {
            x--;
            if(x < 0)
                x = maxX;
        }
        else  //Move right
        {
            x++;
            if(x > maxX)
                x = 0;
        }
    }
    selectedButton = (y << 2) + x;
    drawMenuButton(selectedButton,FALSE,TRUE,selectedPressed.pressed);
    drawButtonDescription(selectedButton);
}

// Changes player type value and returns the new one, goes backwards if moveBack is TRUE, otherwise forwards
static u8 changePlayerValue(u8 value, bool moveBack)
{
    s16 playerCount = (settings.player1 > 0) + (settings.player2 > 0) + (settings.player3 > 0) + (settings.player4 > 0);
    u8 minVal = (playerCount > 2) ? 0 : 1;
    if(moveBack)
    {
        if(value <= minVal)
            value = 4;
        else
            value--;
    }
    else
    {
        if(value >= 4)
            value = minVal;
        else
            value++;
    }
    return value;
}

// Perform a button action, done on button press or continuously if the button is held
static void buttonAction(void)
{
    if(!selectedPressed.pressed)
        return;

    XGM_stopPlayPCM(SOUND_PCM_CH2);
    XGM_startPlayPCM(SFX_CLICK,0,SOUND_PCM_CH2);

    switch(selectedButton)
    {
        case MEB_TUTORIAL:
            changeState(ST_TUTORIALSTATE);
            break;
        case MEB_START:
            changeState(ST_GAMESTATE);
            break;
        case MEB_GRIDWIDTH:
            settings.gridWidth += selectedPressed.direction;
            if(settings.gridWidth < 5)
                settings.gridWidth = 12;
            else if(settings.gridWidth > 12)
                settings.gridWidth = 5;
            updateButtonValue(MEB_GRIDWIDTH);
            selectedPressed.continuous = TRUE;
            break;
        case MEB_GRIDHEIGHT:
            settings.gridHeight += selectedPressed.direction;
            if(settings.gridHeight < 4)
                settings.gridHeight = 7;
            else if(settings.gridHeight > 7)
                settings.gridHeight = 4;
            updateButtonValue(MEB_GRIDHEIGHT);
            selectedPressed.continuous = TRUE;
            break;
        case MEB_MULTICONTROLLER:
            if(mouse_isEnabled())
                return;
            settings.isHotSeat = !settings.isHotSeat;
            menuButtons[MEB_MULTICONTROLLER].icon = (settings.isHotSeat) ? noMultiIconImg : multiIconImg;
            drawMenuIcon(selectedButton);
            drawButtonDescription(selectedButton);
            return;
        case MEB_COLORMODE:
            settings.useOldColors = !settings.useOldColors;
            updateMenuColors(settings.useOldColors);
            drawButtonDescription(selectedButton);
            break;
        case MEB_ABOUT:
            changeState(ST_ABOUTSTATE);
            break;
        case MEB_PLAYER1:
            settings.player1 = changePlayerValue(settings.player1,(selectedPressed.direction < 0));
            selectedPressed.continuous = TRUE;
            updatePlayerPalette(0);
            drawButtonDescription(selectedButton);
            break;
        case MEB_PLAYER2:
            settings.player2 = changePlayerValue(settings.player2,(selectedPressed.direction < 0));
            selectedPressed.continuous = TRUE;
            updatePlayerPalette(1);
            drawButtonDescription(selectedButton);
            break;
        case MEB_PLAYER3:
            settings.player3 = changePlayerValue(settings.player3,(selectedPressed.direction < 0));
            selectedPressed.continuous = TRUE;
            updatePlayerPalette(2);
            drawButtonDescription(selectedButton);
            break;
        case MEB_PLAYER4:
            settings.player4 = changePlayerValue(settings.player4,(selectedPressed.direction < 0));
            selectedPressed.continuous = TRUE;
            updatePlayerPalette(3);
            drawButtonDescription(selectedButton);
            break;
        default:
            break;
    }
}

// Function used when button is pressed - sets up press data and draws a pressed button image
static void pressButton(s8 direction)
{
    selectedPressed.direction = direction;
    selectedPressed.pressed = TRUE;
    selectedPressed.timer = initButtonTimer;
    selectedPressed.continuous = FALSE;
    drawMenuButton(selectedButton,FALSE,TRUE,TRUE);
    buttonAction();
}

static void mouseButtonSelect(void)
{
    MousePosition mpos = mouse_getPosition(TRUE);
    for(u16 i=0; i<MENU_BUTTON_COUNT; i++)
    {
        if(i == selectedButton)
            continue;
        u16 width = (menuButtons[i].small) ? 5 : 9;
        if(mpos.y >= menuButtons[i].y && mpos.y < menuButtons[i].y+5 && mpos.x >= menuButtons[i].x && mpos.x < menuButtons[i].x+width)
        {
            resetButtonPress();
            drawMenuButton(selectedButton,FALSE,FALSE,FALSE);
            selectedButton = i;
            drawMenuButton(selectedButton,FALSE,TRUE,selectedPressed.pressed);
            drawButtonDescription(selectedButton);
            return;
        }
    }
}

void menustate_init(void)
{
    isInit = TRUE;
    idleTimer = 0;
    VDP_setTextPriority(1);
    resetButtonPress();
    initMenuAtoms();
    menuTextImg = reserveVImage(&texMenuText,TRUE);
    setupMenuPalette(settings.useOldColors);
    if(mouse_isEnabled())
        settings.isHotSeat = TRUE;
    settings.useOldColors &= 1; //Make sure the value of this boolean is either 0 or 1
    setupButtons();
    drawMenu();
    isInit = FALSE;
}

void menustate_update(fix32 dt)
{
    spawnMenuAtom();
    moveMenuAtoms();
    if(selectedPressed.pressed && selectedPressed.continuous)
    {
        selectedPressed.timer += dt;
        if(selectedPressed.timer >= actionButtonTimer)
        {
            selectedPressed.timer = 0;
            buttonAction();
        }
    }

    if(!selectedPressed.pressed)
    {
        idleTimer += dt;
        if(idleTimer >= maxIdleTime)
            changeState(ST_TITLESTATE);
    }

    if(mouse_isEnabled())
    {
        mouseButtonSelect();
    }
}

void menustate_joyevent(u16 joy, u16 changed, u16 state)
{
    if(joy==JOY_1)
    {
        if(state & changed)
        {
            switch(changed)
            {
                case BUTTON_UP:
                    moveSelection(TRUE,TRUE);
                    break;
                case BUTTON_DOWN:
                    moveSelection(TRUE,FALSE);
                    break;
                case BUTTON_LEFT:
                    moveSelection(FALSE,TRUE);
                    break;
                case BUTTON_RIGHT:
                    moveSelection(FALSE,FALSE);
                    break;
                case BUTTON_A:
                case BUTTON_B:
                case BUTTON_START:
                    pressButton(1);
                    break;
                case BUTTON_C:
                    pressButton(-1);
                    break;
                default:
                    break;
            }
        }
        else if(selectedPressed.pressed && changed)
        {
            resetButtonPress();
            drawMenuButton(selectedButton,FALSE,TRUE,FALSE);
        }
    }
    idleTimer = 0;
}

void menustate_stop(void)
{
    cleanupMenuAtoms();
    VDP_setTextPriority(0);
    saveSettings();
}
