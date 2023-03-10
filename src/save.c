#include "save.h"
#include "data.h"
#include "states/game/gamelogic.h"
#include "states/game/gameai.h"
#include "states/game/gamestate.h"

const char ksfMagicNum[3] = {'K','S','F'};

struct SFHeader { //KSF header
    char magicNumber[3]; //Has to be 'KSF'
    u8 gridWidth;
    u8 gridHeight;
    u8 totalPlayers; //Amount of players after the game started
    u8 playerCount; //Current player count
    u8 aiDifficulty; //only used when player*AI == 1 (Auto)
    u8 currentPlayer;
    u8 player1status; //0 - eliminated, 1 - not moved, 2 - playing, 255 - no player
    u8 player2status;
    u8 player3status;
    u8 player4status;
    u8 player1AI; //0 - No AI, 1 - Auto (uses aiDifficulty), 2 - Easy AI, 3 - Normal AI, 4 - Hard AI
    u8 player2AI;
    u8 player3AI;
    u8 player4AI;
    u8 seconds; //Seconds from the in-game timer (0-59)
    u8 minutes;
    u8 hours; //Uses full u8 range (0-255)
};

struct TileData {
    u8 playerNum; //It's 0-3 for player 1-4 and 255 (0xFF) for no player 
    u8 atomCount;
};

struct SaveFile { //Full KSF data with static size
    struct SFHeader header;
    struct TileData tiles[MAXGRIDSIZE];
};

struct SaveFile saveData; //Structure used when loading and saving data to SRAM

bool saveValid = FALSE; //If TRUE, a valid savegame was found in SRAM and it's ready to be loaded

void invalidateSRAM(void)
{
    SRAM_enable();
    SRAM_writeByte(0,0x00); //SRAM is now invalid and won't be loaded until next save
    SRAM_writeByte(SAVEDATA_SRAM_POS,0x00); //Saved game is now invalid as well in case settings set byte 0 to 0x47 again
    SRAM_disable();
    saveValid = FALSE;
}

//Save settings to SRAM
void saveSettings(void)
{
    SRAM_enable();
    SRAM_writeByte(0,0x47); //If this byte is there, SRAM is valid
    u8* setMem = (u8*)(&settings);
    for(u32 i=0; i<sizeof(struct KASettings); i++)
    {
        SRAM_writeByte(i+SETTINGS_SRAM_POS,setMem[i]);
    }
    SRAM_disable();
}

//Convert player status to KSF player status format
u8 save_pst(s16 pnum)
{
    switch(playerTab[pnum])
    {
        case PTAB_PLAY:
            if(playerMoved[pnum])
                return 2;
            else
                return 1;
            break;
        case PTAB_LOST:
            return 0;
            break;
        case PTAB_NO:
            return 255;
            break;
        default:
            SYS_die("Wrong player status");
            break;
    }
    return 255;
}

//Convert AI type to KSF AI type format
u8 save_pai(s16 pnum)
{
    if(aiPlayerTab[pnum])
    {
        return aiDifficulty[pnum] + 1;
    }
    else
    {
        return 0;
    }
}

void saveGameData(void)
{
    memset(&saveData,0,sizeof(struct SaveFile));
    memcpy(saveData.header.magicNumber,ksfMagicNum,3); //Write 'K','S','F' to the magicNumber variable
    saveData.header.gridWidth = (u8)grid.width;
    saveData.header.gridHeight = (u8)grid.height;
    saveData.header.totalPlayers = (u8)startPlayers;
    saveData.header.playerCount = (u8)playerCount;
    saveData.header.aiDifficulty = 2;
    saveData.header.currentPlayer = (u8)curPlayer;
    saveData.header.player1status = save_pst(0);
    saveData.header.player2status = save_pst(1);
    saveData.header.player3status = save_pst(2);
    saveData.header.player4status = save_pst(3);
    saveData.header.player1AI = save_pai(0);
    saveData.header.player2AI = save_pai(1);
    saveData.header.player3AI = save_pai(2);
    saveData.header.player4AI = save_pai(3);
    s32 sectime = fix32ToInt(startTime) & 0x1FFFFF;
    s32 mintime = sectime / 60;
    saveData.header.seconds = (u8)(sectime % 60);
    saveData.header.minutes = (u8)(mintime & 0xFF);
    saveData.header.hours = (u8)(mintime >> 8);
    for(int i=0; i<(grid.width*grid.height); i++)
    {
        saveData.tiles[i].atomCount = grid.tiles[i].atomCount;
        saveData.tiles[i].playerNum = grid.tiles[i].playerNum;
    }
    SRAM_enable();
    //At this point settings are always saved, no need to write 0x47 to offset 0
    u8* saveMem = (u8*)(&saveData);
    for(u32 i=0; i<sizeof(struct SaveFile); i++)
    {
        SRAM_writeByte(i+SAVEDATA_SRAM_POS,saveMem[i]);
    }
    saveValid = TRUE;
    SRAM_disable();
}

//Convert KSF AI type to in-game version
void load_cai(s16 pnum, u8 val, u8 aidiff)
{
    if(val==0)
    {
        aiPlayerTab[pnum] = FALSE;
    }
    else
    {
        aiPlayerTab[pnum] = TRUE;
        if(val==1)
            aiDifficulty[pnum] = aidiff;
        else if(val<=4)
            aiDifficulty[pnum] = val-1;
    }
}

//Convert KSF player status to in-game version
void load_cpst(s16 pnum, u8 val)
{
    switch(val)
    {
        case 0:
            playerTab[pnum] = PTAB_LOST;
            playerMoved[pnum] = TRUE;
            playerAtoms[pnum] = 0;
            break;
        case 1:
            playerTab[pnum] = PTAB_PLAY;
            playerMoved[pnum] = FALSE;
            playerAtoms[pnum] = 0;
            break;
        case 2:
            playerTab[pnum] = PTAB_PLAY;
            playerMoved[pnum] = TRUE;
            playerAtoms[pnum] = 1;
            break;
        default:
            playerTab[pnum] = PTAB_NO;
            playerMoved[pnum] = FALSE;
            playerAtoms[pnum] = 0;
            break;
    }
}

fix32 loadGameData(void)
{
    fix32 ttime = -1;
    if(saveValid && saveData.header.gridWidth*saveData.header.gridHeight <= MAXGRIDSIZE)
    {
        grid.width = saveData.header.gridWidth;
        grid.height = saveData.header.gridHeight;
        startPlayers = saveData.header.totalPlayers;
        playerCount = saveData.header.playerCount;
        curPlayer = saveData.header.currentPlayer;
        load_cpst(0,saveData.header.player1status);
        load_cpst(1,saveData.header.player2status);
        load_cpst(2,saveData.header.player3status);
        load_cpst(3,saveData.header.player4status);
        load_cai(0,saveData.header.player1AI,2);
        load_cai(1,saveData.header.player2AI,2);
        load_cai(2,saveData.header.player3AI,2);
        load_cai(3,saveData.header.player4AI,2);
        s32 secTime = saveData.header.seconds+(saveData.header.minutes*60)+(saveData.header.hours*3600);
        ttime = intToFix32(secTime);
        for(int i=0; i<(grid.width*grid.height); i++)
        {
            grid.tiles[i].atomCount = saveData.tiles[i].atomCount;
            grid.tiles[i].playerNum = saveData.tiles[i].playerNum;
        }
        ai_init();
    }
    SRAM_enable();
    for(u32 i=0; i<sizeof(struct SaveFile); i++)
    {
        SRAM_writeByte(SAVEDATA_SRAM_POS+i,0);
    }
    SRAM_disable();
    memset(&saveData,0,sizeof(struct SaveFile));
    saveValid = FALSE;
    return ttime;
}

void loadSRAM(void)
{
    SRAM_enableRO();
    if(SRAM_readByte(0)==0x47)
    {
        memset(&saveData,0,sizeof(struct SaveFile));
        memset(&settings,0,sizeof(struct KASettings));
        u8* setMem = (u8*)(&settings);
        for(u32 i=0; i<sizeof(struct KASettings); i++)
        {
            setMem[i] = SRAM_readByte(i+SETTINGS_SRAM_POS);
        }
        u8* saveMem = (u8*)(&saveData);
        for(u32 i=0; i<sizeof(struct SaveFile); i++)
        {
            saveMem[i] = SRAM_readByte(i+SAVEDATA_SRAM_POS);
        }

        if(saveData.header.magicNumber[0]=='K' && saveData.header.magicNumber[1]=='S' && saveData.header.magicNumber[2]=='F')
        {
            saveValid = TRUE;
        }
        else
        {
            saveValid = FALSE;
        }
    }
    SRAM_disable();
}
