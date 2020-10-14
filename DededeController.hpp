#ifndef DEDEDECONTROLLER_HPP_INCLUDED
#define DEDEDECONTROLLER_HPP_INCLUDED

#include "GameController.hpp"

class DededeController : public GameController {
private:
    SDL_Window* myWindow;
    SDL_Surface* windowIcon;
    int windowW, windowH;
    SDL_Renderer* myRenderer;
    SDL_Texture* background;
    SDL_Texture* logo;
    SDL_Texture* surfingDedede;
    SDL_Texture* play_spritesheet;
    SDL_Texture* quit_spritesheet;
    SDL_Texture* wave[12];
    SDL_Texture* sparkle[6];
    SDL_Texture* lensFlare;
    SDL_Texture* credits;
    SDL_Texture* opening;
    SDL_Texture* justKidding;
    SDL_Texture* repeat;
    int introFrames;
    int waveFrame;
    float dededeHeight;
    bool dededeHover;
    SDL_Rect dededePlacement;
    SDL_Rect playPlacement;
    SDL_Rect quitPlacement;
    SDL_Rect repeatPlacement;
    int playButtonMode;
    int quitButtonMode;
    int exitFrames;
    GameType exitCode;
    // This is hard coded so badly its annoying, but luckily it isnt needed that bad

    Mix_Music* dededeTheme;
    Mix_Chunk* waveSound;
    Mix_Chunk* clash;
    Mix_Chunk* menuSelect;
    Mix_Chunk* menuConfirm;
    Mix_Chunk* dededeLaugh;

public:
    DededeController(SDL_Window* inheritedWindow, SDL_Renderer* inheritedRenderer);
    ~DededeController();
    GameType play();
    void display();
    SDL_Texture* loadTextureFromFile(std::string filePath);
    void renderIntro();
    void renderMain();
};

#endif // DEDEDECONTROLLER_HPP_INCLUDED
