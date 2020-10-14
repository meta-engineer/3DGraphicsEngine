#ifndef SCREENCONTROLLER_HPP_INCLUDED
#define SCREENCONTROLLER_HPP_INCLUDED

#include <SDL.h>
#include <iostream>
#include <stdlib.h>

#include "GameController.hpp"
#include "DededeController.hpp"
#include "StarFoxController.hpp"

const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 540;

class ScreenController {
private:
    SDL_Window* globalWindow;
    SDL_Renderer* globalRenderer;
    GameController* currentController;
public:
    ScreenController();
    ~ScreenController();
    void Start();
    GameType Dedede();
    GameType StarFox();
};

#endif // SCREENCONTROLLER_HPP_INCLUDED

