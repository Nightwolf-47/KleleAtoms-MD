#ifndef SAVE_H_DEFINED
#define SAVE_H_DEFINED
#include <types.h>

#define SETTINGS_SRAM_POS 1
#define SAVEDATA_SRAM_POS 11

// save.c + save.h - SRAM read/write support for settings and savegames

void saveSettings(void);

void resetSRAM(void);

void loadSRAM(void);

void invalidateSRAM(void);

//Load game data from SRAM
fix32 loadGameData(void);

//Save game data to SRAM
void saveGameData(void);

extern bool saveValid;

#endif //SAVE_H_DEFINED
