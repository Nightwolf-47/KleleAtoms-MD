# KłełeAtoms MD

Mega Drive port of KłełeAtoms, a game where you have to take atoms from other players by blowing up yours, made in SGDK.  

![kleleatoms-md3](https://user-images.githubusercontent.com/72660447/185236242-6d2cb012-f4cf-4e31-9306-1399067d41bd.png)

## How to play

The game starts with an empty grid. Every player can place 1 atom per turn on tiles without enemy atoms.  
When a tile has too many* atoms, it becomes critical and explodes. Each surrounding tile gains an atom and all existing atoms on those tiles go to the player who started the explosion.  
If an explosion causes other atoms to become critical, they will explode as well, causing a chain reaction.  
If a player loses all their atoms, they lose. The last standing player wins the game.  
  
\*corner - 2 atoms, side - 3 atoms, otherwise 4 atoms (or more)  

## Options
- Grid width - Set ingame grid width (7-12)  
- Grid height - Set ingame grid height (4-7)  
- Player 1-4 type - Set player type (None, Human, AI 1, AI 2, AI 3) - only 2 players can be None at the same time.  
- Player colors - Choose between default colors and the ones used in early development versions (original).  
- Multiple controllers - When enabled, every non-AI player uses a separate controller.  
- Reset settings - Completely reset all data and remove the saved game if it exists.  

## Credits  
**Stephane Dallongeville** - [SGDK](https://github.com/Stephane-D/sgdk), a development kit used to compile this game.  
**DrPetter** - SFXR, a tool used to make sounds for this game.  
**[GreffMASTER](https://github.com/GreffMASTER)** - Title screen/README.MD image.

## License
This game is licensed under the [MIT License](https://github.com/Nightwolf-47/KleleAtoms-MD/blob/master/LICENSE).  
