#ifndef MENUATOMS_H_INCLUDED
#define MENUATOMS_H_INCLUDED
#include <genesis.h>

// Try spawning an atom on the screen, will fail if there are already 15 atoms present
void spawnMenuAtom(void);

// Moves the menu atoms according to their movement speed and automatically removes out of bounds atoms
void moveMenuAtoms(void);

// Initializes the object pool used to store menu atoms
void initMenuAtoms(void);

// Removes all atoms and clears the atom object pool
void cleanupMenuAtoms(void);

#endif //MENUATOMS_H_INCLUDED