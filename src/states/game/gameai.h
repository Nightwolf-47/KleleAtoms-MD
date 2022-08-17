#ifndef GAMEAI_H_INCLUDED
#define GAMEAI_H_INCLUDED

extern unsigned short aiPlayerTab[4]; //If TRUE, player is AI-controlled

extern int aiDifficulty[4]; //Array of AI difficulty (1-3) per player

void ai_init(void);

void ai_resetTime(void);

void ai_tryMove(long dt);

#endif //GAMEAI_H_INCLUDED
