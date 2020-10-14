
#include "ScreenController.hpp"


ScreenController::ScreenController() {
    bool success = true;

    //Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 ) {
		std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
		success = false;
	}

	if (success != false) {
        if ( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
            std::cout << "Warning: Linear texture filtering not enabled." << std::endl;
        }
	}

	if (success != false) {
        globalWindow = SDL_CreateWindow( "Loading...", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (globalWindow == NULL) {
            std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
            success = false;
        }
	}

	//Create vsynced renderer for window
    globalRenderer = SDL_CreateRenderer( globalWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
    if( globalRenderer == NULL ) {
        std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
        success = false;
    }

	if (success != false) {
        if (!(IMG_Init(IMG_INIT_PNG) && IMG_INIT_PNG)) {
            std::cout << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
            success = false;
        }
        if (TTF_Init() == -1) {
            std::cout << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
            success = false;
        }
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
            std::cout << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
            success = false;
        }
	}

	currentController = NULL;

	if (success == false) delete this;
}

ScreenController::~ScreenController() {
    delete currentController;

    SDL_DestroyRenderer(globalRenderer);
    globalRenderer = NULL;
    SDL_DestroyWindow(globalWindow);
    globalWindow = NULL;

    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void ScreenController::Start() {

    // start starfox unless no save data or save data says D3_SWITCH
    GameType nextGame = STARFOX_TEMPURATURES;

    // LOAD SAVE DATA
    // only need 0 (starting game) and 1 (fullscreen)
    float saveData[2];
    SDL_RWops* file = SDL_RWFromFile("resources/saveData.bin", "r+b");
    if (file == NULL) {
        std::cout << "No save Data detected! SDL Error: " << SDL_GetError() << std::endl;
        // set values w/o save data
        nextGame = DEDEDE_MEMELAND;
        SDL_SetWindowFullscreen(globalWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
    } else {
        SDL_RWread(file, &saveData[0], sizeof(float), 2);
        SDL_RWclose(file);

        if (saveData[0] == 1) nextGame = DEDEDE_MEMELAND;

        SDL_RWclose(file);
        file = NULL;
    }
    // END LOAD

    if (saveData[1] == 1) SDL_SetWindowFullscreen(globalWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);

    while (nextGame != END_GAMES) {
        if (nextGame == STARFOX_TEMPURATURES) nextGame = this->StarFox();
        else if (nextGame == DEDEDE_MEMELAND) nextGame = this->Dedede();
        else {
            std::cout << "Requested GameType was not recognized." << std::endl;
            break;
        }
    }

}

GameType ScreenController::StarFox() {
    delete currentController;
    currentController = new StarFoxController(globalWindow, globalRenderer);
    return currentController->play();
}

GameType ScreenController::Dedede() {
    delete currentController;
    currentController = new DededeController(globalWindow, globalRenderer);
    return currentController->play();
}
