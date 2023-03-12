# KłełeAtoms MD

Mega Drive port of [KłełeAtoms](https://github.com/Nightwolf-47/KleleAtoms), a game where you have to take atoms from other players by blowing up yours, made in SGDK.  

![kleleatoms-md3](https://user-images.githubusercontent.com/72660447/185236242-6d2cb012-f4cf-4e31-9306-1399067d41bd.png)
![kamd-menu](https://user-images.githubusercontent.com/72660447/224516185-9e3cfaa3-c4eb-41b6-ad8a-e13439d8c434.png)  
![kamd-game](https://user-images.githubusercontent.com/72660447/224516218-f0ffcb4b-ef9f-4c2c-a1f3-a815d4de1a11.png)
![kamd-pause2](https://user-images.githubusercontent.com/72660447/224516223-07529fce-3f36-4ac3-88a9-121a02911cb3.png)  

## How to play

The game starts with an empty grid. Every player can place 1 atom per turn on tiles without enemy atoms.  
When a tile has too many* atoms, it becomes critical and explodes. Each surrounding tile gains an atom and all existing atoms on those tiles go to the player who started the explosion.  
If an explosion causes other atoms to become critical, they will explode as well, causing a chain reaction.  
If a player loses all their atoms, they lose. The last standing player wins the game.  
  
\*corner - 2 atoms, side - 3 atoms, otherwise 4 atoms (or more)  

## Options
- Grid width - Set in-game grid width (5-12)  
- Grid height - Set in-game grid height (4-7)  
- Player 1-4 type - Set player type (None, Human, AI 1, AI 2, AI 3) - only 2 players can be None at the same time.  
- Player colors - Choose between default colors and the ones used in early development versions (original).  
- Multiple controllers - When enabled, every non-AI player uses a separate controller.  
- Reset settings - Completely reset all data and remove the saved game if it exists.  

## Credits  
**Stephane Dallongeville** - [SGDK](https://github.com/Stephane-D/sgdk), a development kit used to compile this game.  
**DrPetter** - SFXR, a tool used to make sounds for this game.  
**[GreffMASTER](https://github.com/GreffMASTER)** - Title screen/First README.MD image.

## SGDK information
Since version 1.1.2, SGDK 1.80 is required to compile this game.  
Older versions require SGDK 1.70.

## License
This game is licensed under the [MIT License](https://github.com/Nightwolf-47/KleleAtoms-MD/blob/master/LICENSE).  
