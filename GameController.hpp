#ifndef GAMECONTROLLER_HPP_INCLUDED
#define GAMECONTROLLER_HPP_INCLUDED

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <math.h>
#include <cmath>

enum GameType {
END_GAMES,
DEDEDE_MEMELAND,
STARFOX_TEMPURATURES
};

class GameController {
private:

public:
    GameController() {};
    virtual ~GameController() {};
    virtual GameType play() = 0;
};

#endif // GAMECONTROLLER_HPP_INCLUDED
