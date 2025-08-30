#ifndef MOUSE_H_INCLUDED
#define MOUSE_H_INCLUDED
#include <types.h>
#include "data.h"

typedef struct MousePosition {
    u16 x;
    u16 y;
} MousePosition;

//Enables mouse and loads cursor sprite
void mouse_init(void);

//Callback function called every frame, updates mouse position
void mouse_update(void);

/**
 * \brief
 *      Get mouse position in either pixels or MegaDrive VDP tiles (8x8 pixels)
 * 
 * \param inTiles
 *      if TRUE, returns position in VDP tiles, if FALSE returns position in pixels
 * \returns
 *      MousePosition struct with X and Y mouse position
*/
MousePosition mouse_getPosition(bool inTiles);

//Initialize cursor color when not inMenus to given palette, should be called in for loop for multiple colors
void mouse_setGameCursorColors(u16 color, u16 palette);

//Sets the mouse palette (PAL0 - PAL3) and the cursor sprite frame if in menus or not
void mouse_setCursorData(bool inMenus, u16 palette);

//Returns TRUE if mouse is enabled, FALSE otherwise
bool mouse_isEnabled(void);

//Disables mouse and unloads cursor sprite
void mouse_stop(void);

#endif
