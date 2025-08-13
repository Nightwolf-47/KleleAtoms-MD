#ifndef MENUATOMS_H_INCLUDED
#define MENUATOMS_H_INCLUDED
#include <genesis.h>

#define MAX_MENUATOMCOUNT 15

typedef struct MenuAtom
{
    Sprite* sprite;
    fix32 x;
    fix32 y;
    fix32 velx;
    fix32 vely;
} MenuAtom;

void spawnMenuAtom(void);

void removeMenuAtom(MenuAtom* atom);

void moveMenuAtoms(void);

void initMenuAtoms(void);

void cleanupMenuAtoms(void);

#endif //MENUATOMS_H_INCLUDED