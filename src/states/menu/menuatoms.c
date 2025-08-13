#include "menuatoms.h"
#include "../../../res/resources.h"

static Pool* menuAtomPool;

void spawnMenuAtom(void)
{
    MenuAtom* curAtom = POOL_allocate(menuAtomPool);
    if(curAtom)
    {
        u8 color = random() & 3;
        curAtom->x = (random() & 1023) * 320;
        curAtom->y = FIX32(-32);
        curAtom->velx = ((random() & 1023) - 512) * 3;
        curAtom->vely = ((random() & 511) + 300) * 3;
        curAtom->sprite = SPR_addSpriteSafe(&sprMenuAtom,(s16)F32_toInt(curAtom->x),(s16)F32_toInt(curAtom->y),TILE_ATTR(color,0,0,0));
        SPR_setFrame(curAtom->sprite,random() % 3);
    }
}

void removeMenuAtom(MenuAtom* atom)
{
    if(atom->sprite)
    {
        SPR_releaseSprite(atom->sprite);
        atom->sprite = NULL;
    }
}

void moveMenuAtoms(void)
{
    s16 atomsAllocated = POOL_getNumAllocated(menuAtomPool);
    MenuAtom** atomArray = POOL_getFirst(menuAtomPool);
    const fix32 maxx = FIX32(328);
    const fix32 minx = FIX32(-32);
    const fix32 maxy = FIX32(232);
    while(atomsAllocated--)
    {
        MenuAtom* curAtom = *atomArray++;
        fix32 curVelX = curAtom->velx;
        fix32 curVelY = curAtom->vely;
        curAtom->x += curVelX;
        curAtom->y += curVelY;
        SPR_setPosition(curAtom->sprite,(s16)F32_toInt(curAtom->x),(s16)F32_toInt(curAtom->y));
        if(curAtom->x > maxx || curAtom->x < minx || curAtom->y > maxy)
        {
            removeMenuAtom(curAtom);
            POOL_release(menuAtomPool,curAtom,TRUE);
        }
    }
}

void initMenuAtoms(void)
{
    menuAtomPool = POOL_create(MAX_MENUATOMCOUNT,sizeof(MenuAtom));
    if(!menuAtomPool)
    {
        SYS_die("Couldn't allocate menu atom pool",NULL);
    }
}

void cleanupMenuAtoms(void)
{
    if(menuAtomPool)
    {
        u16 atomsAllocated = POOL_getNumAllocated(menuAtomPool);
        MenuAtom** atomArray = POOL_getFirst(menuAtomPool);
        while(atomsAllocated--)
        {
            MenuAtom* curAtom = *atomArray++;
            removeMenuAtom(curAtom);
        }
        POOL_destroy(menuAtomPool);
        menuAtomPool = NULL;
    }
}