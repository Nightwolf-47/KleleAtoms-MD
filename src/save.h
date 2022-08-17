#ifndef SAVE_H_DEFINED
#define SAVE_H_DEFINED

#define SETTINGS_SRAM_POS 1
#define SAVEDATA_SRAM_POS 11

// save.c + save.h - SRAM read/write support for settings and savegames

void saveSettings(void);

void resetSRAM(void);

void loadSRAM(void);

void invalidateSRAM(void);

//Load game data from SRAM
long loadGameData(void);

//Save game data to SRAM as KSF (KleleAtoms 1.3 save format)
void saveGameData(void);

extern unsigned short saveValid;

#endif //SAVE_H_DEFINED
