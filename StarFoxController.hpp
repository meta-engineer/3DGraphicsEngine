#ifndef STARFOXCONTROLLER_HPP_INCLUDED
#define STARFOXCONTROLLER_HPP_INCLUDED

#include "GameController.hpp"
#include "View.hpp"
#include "StarFoxGame.hpp"

enum GameState {
END_GAME,
MAIN_MENU,
LEVEL_E,
LEVEL_1,
D3_SWITCH
};

class StarFoxController : public GameController {
private:
    SDL_Window* myWindow;
    SDL_Surface* windowIcon;
    int windowW, windowH;
    SDL_Renderer* myRenderer;
    View* gameView;

    GameState nextState;
    int musicVolume;
    int SFXVolume;
    const int totalData = 6;

public:
    StarFoxController(SDL_Window* inheritedWindow, SDL_Renderer* inheritedRenderer);
    ~StarFoxController();
    GameType play();
    GameState mainMenu();
    GameState levelE();
    GameState level1();
    void level1_intro();
    bool pause();

    SDL_Texture* loadTextureFromFile(std::string filePath);
    SDL_Texture* loadTextureFromText(std::string txt, TTF_Font* font, SDL_Color colour);
};

#endif // STARFOXCONTROLLER_HPP_INCLUDED
