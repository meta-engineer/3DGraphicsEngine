#include "StarFoxController.hpp"

StarFoxController::StarFoxController(SDL_Window* inheritedWindow, SDL_Renderer* inheritedRenderer) {
    myWindow = inheritedWindow;
    SDL_SetWindowTitle(myWindow, "StarFox--");
    windowIcon = IMG_Load("resources/Starfox_Icon.png");
	if (windowIcon == NULL) {
        std::cout << "Unable to load image 'resources/Starfox_Icon.png' SDL_image Error: " << IMG_GetError() << std::endl;
    }
	SDL_SetWindowIcon(myWindow, windowIcon);

    myRenderer = inheritedRenderer;
    if (myWindow == NULL) {
        std::cout << "StarFoxController inherited bogus window" << std::endl;
        delete this;
    }
    if (myRenderer == NULL) {
        std::cout << "StarFoxController inherited bogus renderer" << std::endl;
        delete this;
    }

    gameView = new View(myWindow, myRenderer);

}

StarFoxController::~StarFoxController() {

    SDL_FreeSurface(windowIcon);

    //renderer shared globally
    //SDL_DestroyRenderer(myRenderer);
}

GameType StarFoxController::play() {

    //init
    nextState = MAIN_MENU;

    // LOAD SAVE DATA
    float saveData[totalData];
    SDL_RWops* file = SDL_RWFromFile("resources/saveData.bin", "r+b");
    if (file == NULL) {
        std::cout << "No save data detected! SDL Error: " << SDL_GetError() << std::endl;
        // set values w/o save data
        musicVolume = 80;
        SFXVolume = 30;
        gameView->setRenderMode(FILLED);
    } else {
        SDL_RWread(file, &saveData[0], sizeof(float), totalData);
        SDL_RWclose(file);

        // 0 and 1 are taken by screencontroller
        gameView->setRenderMode((renderMode)saveData[2]);
        SDL_SetWindowBrightness(myWindow, saveData[3]);
        SFXVolume = saveData[4];
        musicVolume = saveData[5];

        SDL_RWclose(file);
        file = NULL;
    }
    // END LOAD

    while(1) {
        if (nextState == END_GAME) break;
        else if (nextState == MAIN_MENU) nextState = mainMenu();
        else if (nextState == LEVEL_E) nextState = levelE();
        else if (nextState == LEVEL_1) nextState = level1();
        else if (nextState == D3_SWITCH) break;
        else {
            std::cout << "Unrecognised next GameState recieved. Aborting" << std::endl;
            break;
        }
    }
    std::cout << "Leaving starfox" << std::endl;

    // SAVE DATA 6 ( D3, fullscreen, render mode, brightness, sfx, music)
    //float saveData[totalData];

    file = SDL_RWFromFile("resources/saveData.bin", "w+b");
    if (file == NULL) {
        std::cout << "Unable to find/create save data! SDL Error: " << SDL_GetError() << std::endl;
    } else {
        std::cout << "Saving data!" << std::endl;

        if (nextState == D3_SWITCH) saveData[0] = 1;
        else saveData[0] = 0;
        if (SDL_GetWindowFlags(myWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP) saveData[1] = 1;
        else saveData[1] = 0;
        saveData[2] = gameView->getRenderMode();
        saveData[3] = SDL_GetWindowBrightness(myWindow);
        saveData[4] = SFXVolume;
        saveData[5] = musicVolume;
        SDL_RWwrite(file, &saveData[0], sizeof(float), totalData);
        SDL_RWclose(file);
    }
    // END SAVE DATA

    if (nextState == D3_SWITCH) return DEDEDE_MEMELAND;
    else return END_GAMES;

}

GameState StarFoxController::mainMenu() {

    SDL_GetWindowSize(myWindow, &windowW, &windowH);

    gameView->normalizeCamera();
    gameView->turnCameraBy(0, M_PI, 0);
    gameView->changeFOV(-500);
    gameView->deleteAllObjectsFromView();

    Object* arwing[4];

    arwing[0] = new Object("resources/swordfish_struct.txt", {500, -100, 300});
    gameView->addObjectToView(arwing[0]);

    arwing[1] = new Object("resources/arwing_struct.txt", {1300, -100, -1500});
    gameView->addObjectToView(arwing[1]);

    arwing[2] = new Object("resources/arwing_struct.txt", {-500, -100, -2300});
    gameView->addObjectToView(arwing[2]);

    arwing[3] = new Object("resources/arwing_struct.txt", {-1300, -100, -500});
    gameView->addObjectToView(arwing[3]);

    int numRocks = 100;
    Object* rocks[numRocks];

    for (int i = 0; i < numRocks; i++) {
        p3d rockLocation = {rand() % 40000 - 20000, rand() % 8000 - 4000, rand() % 40000 - 20000};
        while (abs(rockLocation.x) + abs(rockLocation.z) < 7000) {
                rockLocation.x = rand() % 20000 - 10000;
                rockLocation.z = rand() % 20000 - 10000;
        }
        rocks[i] = new Object("resources/icosahedron_struct.txt", rockLocation);
        gameView->addObjectToView(rocks[i]);
        rocks[i]->rotateBy((float)(rand() % 6290) / 3, (float)(rand() % 6290) / 3, (float)(rand() % 6290) / 3);
        rocks[i]->accelerateAngular((float)(rand() % 10) / 500, (float)(rand() % 10) / 500, (float)(rand() % 10) / 500);
    }

    SDL_Rect titlePlacement = {windowW * 0.015, windowH * 0.01, windowW * 0.81, windowH * 0.3472};
    SDL_Texture* title = loadTextureFromFile("resources/starfox_title.png");

    SDL_Rect d3Placement = {windowW * 0.01, windowH - (windowH * 0.11), windowW * 0.048, windowH * 0.0975};
    SDL_Texture* d3 = loadTextureFromFile("resources/d3_return.png");

    SDL_Rect borderPlacement = {windowW - (windowW * 0.5343), windowH - (windowH * 0.776), windowW * 0.5343, windowH * 0.776};
    SDL_Texture* border = loadTextureFromFile("resources/menu_guilding.png");

    enum buttonState {
    OFF,
    HOVER,
    DOWN
    };

    buttonState campaignState = OFF;
    SDL_Rect buttonCampaignPlacement = {windowW * 0.078, windowH * 0.369, windowW * 0.4297, windowH * 0.1472};
    SDL_Texture* buttonCampaignOff = loadTextureFromFile("resources/campaign_button_off.png");
    SDL_Texture* buttonCampaignHover = loadTextureFromFile("resources/campaign_button_hover.png");
    SDL_Texture* buttonCampaignDown = loadTextureFromFile("resources/campaign_button_down.png");

    buttonState explorerState = OFF;
    SDL_Rect buttonExplorerPlacement = {windowW * 0.078, windowH * 0.515, windowW * 0.4297, windowH * 0.1472};
    SDL_Texture* buttonExplorerOff = loadTextureFromFile("resources/explorer_button_off.png");
    SDL_Texture* buttonExplorerHover = loadTextureFromFile("resources/explorer_button_hover.png");
    SDL_Texture* buttonExplorerDown = loadTextureFromFile("resources/explorer_button_down.png");

    buttonState optionsState = OFF;
    SDL_Rect buttonOptionsPlacement = {windowW * 0.078, windowH * 0.662, windowW * 0.4297, windowH * 0.1472};
    SDL_Texture* buttonOptionsOff = loadTextureFromFile("resources/options_button_off.png");
    SDL_Texture* buttonOptionsHover = loadTextureFromFile("resources/options_button_hover.png");
    SDL_Texture* buttonOptionsDown = loadTextureFromFile("resources/options_button_down.png");

    buttonState exitState = OFF;
    SDL_Rect buttonExitPlacement = {windowW * 0.078, windowH * 0.809, windowW * 0.4297, windowH * 0.1472};
    SDL_Texture* buttonExitOff = loadTextureFromFile("resources/exit_button_off.png");
    SDL_Texture* buttonExitHover = loadTextureFromFile("resources/exit_button_hover.png");
    SDL_Texture* buttonExitDown = loadTextureFromFile("resources/exit_button_down.png");

    Mix_Music* corneriaTheme = Mix_LoadMUS("resources/Starfox_start.mp3");
    if (corneriaTheme == NULL) {
        std::cout << "Failed to load music. SDL_mixer Error: " << Mix_GetError() << std::endl;
    } else {
        Mix_PlayMusic(corneriaTheme, -1);
        Mix_VolumeMusic(musicVolume);
    }

    Mix_Chunk* buttonHover = Mix_LoadWAV( "resources/menuSelect.wav" );
    if (buttonHover == NULL) {
        std::cout << "Failed to load buttonHover. SDL_mixer Error: " << Mix_GetError() << std::endl;
    } else {
        buttonHover->volume = SFXVolume;
    }

    Mix_Chunk* buttonWrong = Mix_LoadWAV( "resources/menuWrong.wav" );
    if (buttonWrong == NULL) {
        std::cout << "Failed to load buttonWrong. SDL_mixer Error: " << Mix_GetError() << std::endl;
    } else {
        buttonWrong->volume = SFXVolume;
    }

    Mix_Chunk* buttonDown = Mix_LoadWAV( "resources/menuConfirm.wav" );
    if (buttonDown == NULL) {
        std::cout << "Failed to load buttonDown. SDL_mixer Error: " << Mix_GetError() << std::endl;
    } else {
        buttonDown->volume = SFXVolume;
    }


    SDL_Event e;
    int mouseX, mouseY;
    bool tracking = false;
    int frameCount = 0;
    int exitFrames = 0;
    GameState exitCode = END_GAME;

    while (1) {

        if (exitFrames > 0) {
            if (exitFrames == 1) break;
            exitFrames--;
        }

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
                return END_GAME;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                    SDL_GetMouseState(&mouseX, &mouseY);
                    if (mouseX >= buttonExplorerPlacement.x && mouseX <= buttonExplorerPlacement.x + buttonExplorerPlacement.w &&
                        mouseY >= buttonExplorerPlacement.y && mouseY <= buttonExplorerPlacement.y + buttonExplorerPlacement.h) {
                        explorerState = DOWN;
                        Mix_PlayChannel( -1, buttonDown, 0 );
                        exitCode = LEVEL_E;
                        exitFrames = 30;
                    } else if (mouseX >= buttonOptionsPlacement.x && mouseX <= buttonOptionsPlacement.x + buttonOptionsPlacement.w &&
                               mouseY >= buttonOptionsPlacement.y && mouseY <= buttonOptionsPlacement.y + buttonOptionsPlacement.h) {
                        optionsState = DOWN;
                        Mix_PlayChannel( -1, buttonDown, 0 );
                        this->pause();

                        //refresh element placement should do by procedure
                        titlePlacement = {windowW * 0.015, windowH * 0.01, windowW * 0.81, windowH * 0.3472};
                        d3Placement = {windowW * 0.01, windowH - (windowH * 0.11), windowW * 0.048, windowH * 0.0975};
                        borderPlacement = {windowW - (windowW * 0.5343), windowH - (windowH * 0.776), windowW * 0.5343, windowH * 0.776};
                        buttonCampaignPlacement = {windowW * 0.078, windowH * 0.369, windowW * 0.4297, windowH * 0.1472};
                        buttonExplorerPlacement = {windowW * 0.078, windowH * 0.515, windowW * 0.4297, windowH * 0.1472};
                        buttonOptionsPlacement = {windowW * 0.078, windowH * 0.662, windowW * 0.4297, windowH * 0.1472};
                        buttonExitPlacement = {windowW * 0.078, windowH * 0.809, windowW * 0.4297, windowH * 0.1472};

                        //oh god
                        buttonDown->volume = SFXVolume;
                        buttonWrong->volume = SFXVolume;
                        buttonHover->volume = SFXVolume;

                    } else if (mouseX >= buttonExitPlacement.x && mouseX <= buttonExitPlacement.x + buttonExitPlacement.w &&
                               mouseY >= buttonExitPlacement.y && mouseY <= buttonExitPlacement.y + buttonExitPlacement.h) {
                        exitState = DOWN;
                        Mix_PlayChannel( -1, buttonDown, 0 );
                        exitCode = END_GAME;
                        exitFrames = 30;
                    } else if (mouseX >= buttonCampaignPlacement.x && mouseX <= buttonCampaignPlacement.x + buttonCampaignPlacement.w &&
                               mouseY >= buttonCampaignPlacement.y && mouseY <= buttonCampaignPlacement.y + buttonCampaignPlacement.h) {
                        campaignState = DOWN;
                        Mix_PlayChannel( -1, buttonDown, 0 );
                        exitCode = LEVEL_1;
                        exitFrames = 30;
                    } else if (mouseX >= d3Placement.x && mouseX <= d3Placement.x + d3Placement.w &&
                               mouseY >= d3Placement.y && mouseY <= d3Placement.y + d3Placement.h) {
                        Mix_PlayChannel( -1, buttonDown, 0 );
                        exitCode = D3_SWITCH;
                        exitFrames = 30;
                    } else tracking = true;
            }
            if (e.type == SDL_MOUSEMOTION && tracking == true) {
                int x, y;
                SDL_GetMouseState( &x, &y);
                int w; int h;
                SDL_GetWindowSize(myWindow, &w, &h);
                gameView->turnCameraBy(0, (float)(mouseX - x) / (w/2), 0);
                mouseX = x; mouseY = y;
            }
            if (e.type == SDL_MOUSEMOTION) {
                SDL_GetMouseState(&mouseX, &mouseY);
                if (mouseX >= buttonExplorerPlacement.x && mouseX <= buttonExplorerPlacement.x + buttonExplorerPlacement.w &&
                    mouseY >= buttonExplorerPlacement.y && mouseY <= buttonExplorerPlacement.y + buttonExplorerPlacement.h) {
                    if (explorerState == OFF) {
                        explorerState = HOVER;
                        Mix_PlayChannel( -1, buttonHover, 0 );
                    }
                } else explorerState = OFF;
                if (mouseX >= buttonOptionsPlacement.x && mouseX <= buttonOptionsPlacement.x + buttonOptionsPlacement.w &&
                    mouseY >= buttonOptionsPlacement.y && mouseY <= buttonOptionsPlacement.y + buttonOptionsPlacement.h) {
                    if (optionsState == OFF) {
                        optionsState = HOVER;
                        Mix_PlayChannel( -1, buttonHover, 0 );
                    }
                } else optionsState = OFF;
                if (mouseX >= buttonExitPlacement.x && mouseX <= buttonExitPlacement.x + buttonExitPlacement.w &&
                    mouseY >= buttonExitPlacement.y && mouseY <= buttonExitPlacement.y + buttonExitPlacement.h) {
                    if (exitState == OFF) {
                        exitState = HOVER;
                        Mix_PlayChannel( -1, buttonHover, 0 );
                    }
                } else exitState = OFF;
                if (mouseX >= buttonCampaignPlacement.x && mouseX <= buttonCampaignPlacement.x + buttonCampaignPlacement.w &&
                    mouseY >= buttonCampaignPlacement.y && mouseY <= buttonCampaignPlacement.y + buttonCampaignPlacement.h) {
                    if (campaignState == OFF) {
                        campaignState = HOVER;
                        Mix_PlayChannel( -1, buttonHover, 0 );
                    }
                } else campaignState = OFF;
            }
            if (e.type == SDL_MOUSEBUTTONUP) tracking = false;
        }

        for (int i = 0; i < numRocks; i++) {
            rocks[i]->advanceFrame();
        }

        if (frameCount % 5 == 0) {
            for (int i = 0; i < 4; i++) {
                if (arwing[i]->getFaces()[5].green == 0) {
                    arwing[i]->getFaces()[5].green = 100;
                    arwing[i]->getFaces()[6].green = 100;
                } else {
                    arwing[i]->getFaces()[5].green = 0;
                    arwing[i]->getFaces()[6].green = 0;
                }
            }
        }
        arwing[0]->rotateBy(0, -0.01, 0);
        arwing[1]->rotateBy(0.01, 0, 0);
        arwing[3]->rotateBy(0, 0, 0.01);
        gameView->turnCameraBy(0, 0.002, 0);
        gameView->display();

        //display menu
        SDL_RenderCopy(myRenderer, title, NULL, &titlePlacement);
        SDL_RenderCopy(myRenderer, border, NULL, &borderPlacement);
        SDL_RenderCopy(myRenderer, d3, NULL, &d3Placement);
        if (campaignState == OFF) SDL_RenderCopy(myRenderer, buttonCampaignOff, NULL, &buttonCampaignPlacement);
        else if (campaignState == HOVER) SDL_RenderCopy(myRenderer, buttonCampaignHover, NULL, &buttonCampaignPlacement);
        else SDL_RenderCopy(myRenderer, buttonCampaignDown, NULL, &buttonCampaignPlacement);

        if (explorerState == OFF) SDL_RenderCopy(myRenderer, buttonExplorerOff, NULL, &buttonExplorerPlacement);
        else if (explorerState == HOVER) SDL_RenderCopy(myRenderer, buttonExplorerHover, NULL, &buttonExplorerPlacement);
        else SDL_RenderCopy(myRenderer, buttonExplorerDown, NULL, &buttonExplorerPlacement);

        if (optionsState == OFF) SDL_RenderCopy(myRenderer, buttonOptionsOff, NULL, &buttonOptionsPlacement);
        else if (optionsState == HOVER) SDL_RenderCopy(myRenderer, buttonOptionsHover, NULL, &buttonOptionsPlacement);
        else SDL_RenderCopy(myRenderer, buttonOptionsDown, NULL, &buttonOptionsPlacement);

        if (exitState == OFF) SDL_RenderCopy(myRenderer, buttonExitOff, NULL, &buttonExitPlacement);
        else if (exitState == HOVER) SDL_RenderCopy(myRenderer, buttonExitHover, NULL, &buttonExitPlacement);
        else SDL_RenderCopy(myRenderer, buttonExitDown, NULL, &buttonExitPlacement);

        //Update screen
        SDL_RenderPresent( myRenderer );

        frameCount++;
        if (frameCount >= 60) frameCount = 0;
    }

    SDL_DestroyTexture(title);
    SDL_DestroyTexture(d3);
    SDL_DestroyTexture(border);
    SDL_DestroyTexture(buttonCampaignOff);
    SDL_DestroyTexture(buttonCampaignHover);
    SDL_DestroyTexture(buttonCampaignOff);
    SDL_DestroyTexture(buttonExplorerOff);
    SDL_DestroyTexture(buttonExplorerHover);
    SDL_DestroyTexture(buttonExplorerDown);
    SDL_DestroyTexture(buttonOptionsOff);
    SDL_DestroyTexture(buttonOptionsHover);
    SDL_DestroyTexture(buttonOptionsDown);
    SDL_DestroyTexture(buttonExitOff);
    SDL_DestroyTexture(buttonExitHover);
    SDL_DestroyTexture(buttonExitDown);

    Mix_FreeMusic(corneriaTheme);
    Mix_FreeChunk(buttonWrong);
    Mix_FreeChunk(buttonHover);
    Mix_FreeChunk(buttonDown);

    return exitCode;
}

GameState StarFoxController::levelE() {

    float hp = 100;
    float fp = 100;
    float cp = 100;
    float tp = 0;
    bool overheat = false;
    bool warp = false;


    gameView->normalizeCamera();
    //gameView->setRenderDist(100000);
    gameView->changeFOV(-600);
    gameView->deleteAllObjectsFromView();

    Object* planet = new Object("resources/planetX_struct.txt", {0, 0, 0});
    gameView->addObjectToView(planet);
    planet->rotateBy(0.4, 0, 0);
    planet->accelerateAngular(0, 0.001, 0);

    Object* sf = new Object("resources/butterfly_ship_struct.txt", {15000, 0, 80000});
    sf->rotateBy(2, -2, 0);
    gameView->addObjectToView(sf);

    Object* arwing = new Object("resources/arwing_struct.txt", {20000, -300, 88100});
    gameView->addObjectToView(arwing);
    arwing->rotateTo(0, M_PI, 0);
    bool autoPilot = true;

    Object* gyro = new Object("Resources/gyro_struct.txt", {0, 10000, 0});
    gameView->addObjectToView(gyro);
    gyro->rotateBy(0, M_PI + 0.3, 0);

    Object* lasers[50];
    int laserIndex = 0;
    for (int i = 0; i < 50; i ++) {
        lasers[i] = new Object("resources/laser_struct.txt", {0, 0, 0});
        gameView->addObjectToView(lasers[i]);
        lasers[i]->setVisible(false);
    }


    Object* satellites[100];

    for (int i = 0; i < 100; i++) {
            //satellites[i] = NULL; continue;
        p3d satelliteLocation = {(rand() * 4) % 100000 - 50000, (rand() * 2) % 40000 - 20000, (rand() * 4) % 100000 - 50000};
        if (i < 50) {
            satellites[i] = new Object("resources/icosahedron_struct.txt", satelliteLocation);
        } else {
            satellites[i] = new Object("resources/asteroid_struct.txt", satelliteLocation);
        }
        gameView->addObjectToView(satellites[i]);

        int orbitX = satellites[i]->getAnchor().x - planet->getAnchor().x;
        int orbitY = satellites[i]->getAnchor().y - planet->getAnchor().y;
        int orbitZ = satellites[i]->getAnchor().z - planet->getAnchor().z;
        satellites[i]->accelerate(2000000 / sqrt( pow(satellites[i]->getAnchor().x - planet->getAnchor().x, 2) +
                                                pow(satellites[i]->getAnchor().y - planet->getAnchor().y, 2) +
                                                pow(satellites[i]->getAnchor().z - planet->getAnchor().z, 2))
                                  , orbitZ, orbitX, orbitY);

        satellites[i]->accelerateAngular((float)(rand() % 10) / 100, (float)(rand() % 10) / 100, (float)(rand() % 10) / 100);
    }

    int numStars = 50;

    Object* stars[numStars];
    for (int i = 0; i < numStars; i++) {
        // 1 million unit diameter. Remember rand goes to 32k
        p3d starLocation = {(rand() * 16) % 500000 - 250000, (rand() * 16) % 500000 - 250000, (rand() * 16) % 500000 - 250000};
        stars[i] = new Object("resources/icosahedron_w_struct.txt", starLocation);
        gameView->addObjectToView(stars[i]);
        std::ostringstream ID;
        ID << stars[i]->getID() << i;
        stars[i]->setID( ID.str() );
    }

    SDL_Texture* dash = loadTextureFromFile("resources/starfox_dash.png");

    SDL_Rect controlsPlacement = {0, 0, windowW * 0.45, windowH * 0.3};
    SDL_Texture* controls = loadTextureFromFile("resources/E_controls.png");

    SDL_Rect hpBarPlacement = {windowW * 0.01458, windowH * 0.8352, windowW * 0.339 / 1, windowH * 0.03148};
    SDL_Texture* hpBar = loadTextureFromFile("resources/starfox_hpguage.png");
    SDL_Rect fuelBarPlacement = {windowW * 0.01458, windowH * 0.8777, windowW * 0.339 / 1, windowH * 0.03148};
    SDL_Texture* fuelBar = loadTextureFromFile("resources/starfox_fuelguage.png");
    SDL_Rect chargeBarPlacement = {windowW * 0.01458, windowH * 0.9203, windowW * 0.339 / 1, windowH * 0.03148};
    SDL_Texture* chargeBar = loadTextureFromFile("resources/starfox_chargeguage.png");
    SDL_Rect heatBarPlacement = {windowW * 0.01458, windowH * 0.955, windowW * 0.339 / 1, windowH * 0.03148};
    SDL_Texture* heatBar = loadTextureFromFile("resources/starfox_heatguage.png");

    SDL_Rect overheatPlacement = {windowW * 0.08, windowH * 0.957, windowW * 0.2, windowH * 0.03148};
    SDL_Texture* overheatLabel = loadTextureFromFile("resources/overheat.png");

    SDL_Rect dialPlacement = {windowW * 0.702, windowH * 0.7259, windowW * 0.1041, windowH * 0.1852};
    SDL_Texture* dial = loadTextureFromFile("resources/starfox_dial.png");

    SDL_Rect steeringPlacement = {windowW * 0.5, windowH * 0.7593, windowW * 0.191666, windowH * 0.09};
    SDL_Texture* steeringOn = loadTextureFromFile("resources/steering_on.png");
    SDL_Texture* steeringOff = loadTextureFromFile("resources/steering_off.png");

    bool commsVisible = true;
    SDL_Rect commsTabPlacement = {windowW * 0.03125, windowH * 0.7593, windowW * 0.191666, windowH * 0.0704};
    SDL_Texture* commsTab = loadTextureFromFile("resources/starfox_comms_tab.png");
    SDL_Rect commsPlacement = {windowW * 0.0281, windowH * 0.555, windowW * 0.33958, windowH * 0.263};
    SDL_Texture* comms = loadTextureFromFile("resources/starfox_comms.png");

    SDL_Rect playPlacement = {windowW * 0.159, windowH * 0.6666, windowW * 0.04166, windowH * 0.074};
    SDL_Rect seekPlacement = {windowW * 0.102, windowH * 0.6666, windowW * 0.04166, windowH * 0.074};
    SDL_Rect scanPlacement = {windowW * 0.05, windowH * 0.6666, windowW * 0.04166, windowH * 0.074};
    SDL_Rect audioSpecPlacement = {windowW * 0.2239, windowH * 0.6852, windowW * 0.115, windowH * 0.095};
    int audioFrame = 0;
    int audioFrameDelay = 0;
    bool audioPlaying = false;
    SDL_Texture* audioSpec[20];
    audioSpec[0] = loadTextureFromFile("resources/audio-spectrum-0.png");
    audioSpec[1] = loadTextureFromFile("resources/audio-spectrum-1.png");
    audioSpec[2] = loadTextureFromFile("resources/audio-spectrum-2.png");
    audioSpec[3] = loadTextureFromFile("resources/audio-spectrum-3.png");
    audioSpec[4] = loadTextureFromFile("resources/audio-spectrum-4.png");
    audioSpec[5] = loadTextureFromFile("resources/audio-spectrum-5.png");
    audioSpec[6] = loadTextureFromFile("resources/audio-spectrum-6.png");
    audioSpec[7] = loadTextureFromFile("resources/audio-spectrum-7.png");
    audioSpec[8] = loadTextureFromFile("resources/audio-spectrum-8.png");
    audioSpec[9] = loadTextureFromFile("resources/audio-spectrum-9.png");
    audioSpec[10] = loadTextureFromFile("resources/audio-spectrum-10.png");
    audioSpec[11] = loadTextureFromFile("resources/audio-spectrum-11.png");
    audioSpec[12] = loadTextureFromFile("resources/audio-spectrum-12.png");
    audioSpec[13] = loadTextureFromFile("resources/audio-spectrum-13.png");
    audioSpec[14] = loadTextureFromFile("resources/audio-spectrum-14.png");
    audioSpec[15] = loadTextureFromFile("resources/audio-spectrum-15.png");
    audioSpec[16] = loadTextureFromFile("resources/audio-spectrum-16.png");
    audioSpec[17] = loadTextureFromFile("resources/audio-spectrum-17.png");
    audioSpec[18] = loadTextureFromFile("resources/audio-spectrum-18.png");
    audioSpec[19] = loadTextureFromFile("resources/audio-spectrum-19.png");

    int totalTrack = 14;
    int currTrack = 1;
    Mix_Music* radioList[totalTrack];
    radioList[0] = Mix_LoadMUS("resources/shipHum.wav");
    radioList[1] = Mix_LoadMUS("resources/starfox_metal.wav");
    radioList[2] = Mix_LoadMUS("resources/baccano_Gun's&Roses.mp3");
    radioList[3] = Mix_LoadMUS("resources/wildguns.wav");
    radioList[4] = Mix_LoadMUS("resources/bebop_Rush.mp3");
    radioList[5] = Mix_LoadMUS("resources/bookofspells.wav");
    radioList[6] = Mix_LoadMUS("resources/D3_16.wav");
    radioList[7] = Mix_LoadMUS("resources/bebop_OddOnes.mp3");
    radioList[8] = Mix_LoadMUS("resources/Equilibrium_Kurzes_Epos.mp3");
    radioList[9] = Mix_LoadMUS("resources/baccano_IntheSpeakEasy.mp3");
    radioList[10] = Mix_LoadMUS("resources/metalwarrior_city.wav");
    radioList[11] = Mix_LoadMUS("resources/bebop_FarewellBlues.mp3");
    radioList[12] = Mix_LoadMUS("resources/baccano_theme.mp3");
    radioList[13] = Mix_LoadMUS("resources/bebop_SpaceLion.mp3");

    std::string trackTextList[totalTrack];
    trackTextList[0] = "";
    trackTextList[1] = "Corneria Theme - StarFox  ";
    trackTextList[2] = "Guns & Roses - Baccano    ";
    trackTextList[3] = "Final Fight - WildGuns    ";
    trackTextList[4] = "Rush - C.Bebop            ";
    trackTextList[5] = "Book of Spells - Runescape";
    trackTextList[6] = "King Dedede's Theme       ";
    trackTextList[7] = "Odd Ones - C.Bebop        ";
    trackTextList[8] = "Kurzes Epos - Equilibrium ";
    trackTextList[9] = "Speak Easy - C.Bebop      ";
    trackTextList[10] = "City Theme - Metal Warrior";
    trackTextList[11] = "Farewell Blues - C.Bebop  ";
    trackTextList[12] = "Main Theme - Baccano      ";
    trackTextList[13] = "Space Lion - C.Bebop      ";

    for (int i = 0; i < totalTrack; i++) {
        if (radioList[i] == NULL) {
            std::cout << "Failed to load music. SDL_mixer Error: " << Mix_GetError() << std::endl;
        }
    }
    Mix_VolumeMusic(musicVolume);
    Mix_PlayMusic(radioList[0], -1);

    Mix_Chunk* noise = Mix_LoadWAV("resources/whiteNoise.wav");
    if (noise == NULL) {
        std::cout << "Failed to load 'noise'. SDL_mixer Error: " << Mix_GetError() << std::endl;
    } noise->volume = SFXVolume;

    Mix_Chunk* laserBlip = Mix_LoadWAV("resources/falco1e.mp3");
    if (laserBlip == NULL) {
        std::cout << "Failed to load laserBlip! SDL_mixer Error: " << Mix_GetError() << std::endl;
    } else laserBlip->volume = SFXVolume;

    TTF_Font* meterFont = TTF_OpenFont("resources/FINALOLD.ttf", 36);
    if (meterFont == NULL) {
        std::cout << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
    }
    SDL_Color textColor = {0, 0, 0};
    SDL_Texture* spdTexture = loadTextureFromText("spd", meterFont, textColor);
    SDL_Rect spdTextPlacement = {windowW * 0.508, windowH * 0.86, windowW * 0.1177, windowH * 0.11};
    SDL_Texture* accTexture = loadTextureFromText("acc", meterFont, textColor);
    SDL_Rect accTextPlacement = {windowW * 0.704, windowH * 0.927, windowW * 0.0729, windowH * 0.0648};
    SDL_Texture* rotTexture = loadTextureFromText("rot", meterFont, textColor);
    SDL_Rect rotTextPlacement = {windowW * 0.847, windowH * 0.928, windowW * 0.0625, windowH * 0.0648};

    SDL_Texture* trackText = loadTextureFromText("ayyy lmao what?", meterFont, textColor);
    SDL_Rect trackTextPlacement = {windowW * 0.058, windowH * 0.592, windowW * 0.28, windowH * 0.068};

    SDL_Event e;

    int mouseX;
    int mouseY;
    bool tracking = false;
    bool turning = false;
    bool quit = false;
    GameState exitCode;
    int frameCount = 0;

    while(!quit) {

        if (hp < 0) {
            quit = true;
            exitCode = MAIN_MENU;
        }

        while( SDL_PollEvent(&e) != 0 ) {
            if (e.type == SDL_QUIT) {
                quit = true;
                exitCode = END_GAME;
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                if (pause() == true) {
                    quit = true;
                    exitCode = MAIN_MENU;
                } else {
                    // refresh elements after window/volume change
                    noise->volume = SFXVolume;
                    laserBlip->volume = SFXVolume;

                    hpBarPlacement = {windowW * 0.01458, windowH * 0.8352, windowW * 0.339, windowH * 0.03148};
                    fuelBarPlacement = {windowW * 0.01458, windowH * 0.8777, windowW * 0.339, windowH * 0.03148};
                    chargeBarPlacement = {windowW * 0.01458, windowH * 0.9203, windowW * 0.339, windowH * 0.03148};
                    heatBarPlacement = {windowW * 0.01458, windowH * 0.955, windowW * 0.339, windowH * 0.03148};

                    overheatPlacement = {windowW * 0.08, windowH * 0.957, windowW * 0.2, windowH * 0.03148};

                    dialPlacement = {windowW * 0.702, windowH * 0.7259, windowW * 0.1041, windowH * 0.1852};

                    steeringPlacement = {windowW * 0.5, windowH * 0.7593, windowW * 0.191666, windowH * 0.09};

                    commsTabPlacement = {windowW * 0.03125, windowH * 0.7593, windowW * 0.191666, windowH * 0.0704};
                    commsPlacement = {windowW * 0.0281, windowH * 0.555, windowW * 0.33958, windowH * 0.263};

                    playPlacement = {windowW * 0.159, windowH * 0.6666, windowW * 0.04166, windowH * 0.074};
                    seekPlacement = {windowW * 0.102, windowH * 0.6666, windowW * 0.04166, windowH * 0.074};
                    scanPlacement = {windowW * 0.05, windowH * 0.6666, windowW * 0.04166, windowH * 0.074};
                    audioSpecPlacement = {windowW * 0.2239, windowH * 0.6852, windowW * 0.115, windowH * 0.095};

                    spdTextPlacement = {windowW * 0.508, windowH * 0.86, windowW * 0.1177, windowH * 0.11};
                    accTextPlacement = {windowW * 0.704, windowH * 0.927, windowW * 0.0729, windowH * 0.0648};
                    rotTextPlacement = {windowW * 0.847, windowH * 0.928, windowW * 0.0625, windowH * 0.0648};

                    trackTextPlacement = {windowW * 0.058, windowH * 0.592, windowW * 0.28, windowH * 0.068};
                }
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_BACKQUOTE) {
                if (warp == false) {
                    SDL_Delay(500);
                    arwing->accelerate(10000);
                    warp = true;
                    gameView->changeFOV(-900);
                } else {
                    warp = false;
                    arwing->accelerate(-9500);
                    gameView->changeFOV(900);
                }
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {
                if (cp >= 25) {
                    Mix_PlayChannel(-1, laserBlip, 0);
                    cp -= 25;
                    lasers[laserIndex]->setVisible(true);
                    lasers[laserIndex]->moveTo(arwing->getAnchor().x, arwing->getAnchor().y, arwing->getAnchor().z);
                    //newtonian laser beams??
                    float baseVel = sqrt( pow(arwing->getVelocityVec_x(), 2) + pow(arwing->getVelocityVec_y(), 2) + pow(arwing->getVelocityVec_z(), 2) );
                    lasers[laserIndex]->accelerate(baseVel + 300, arwing->getDirVec_x(), arwing->getDirVec_y() * 1.1, arwing->getDirVec_z());
                    lasers[laserIndex]->rotateTo(arwing->getPitch(), arwing->getYaw(), arwing->getRoll());
                    laserIndex++;
                    if (laserIndex >= 50) laserIndex = 0;
                }
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_TAB) {
                autoPilot = !(autoPilot);
            }

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                    tracking = true;
                    SDL_GetMouseState(&mouseX, &mouseY);
                    if (mouseX >= commsTabPlacement.x && mouseX <= commsTabPlacement.x + commsTabPlacement.w &&
                        mouseY >= commsTabPlacement.y && mouseY <= commsTabPlacement.y + commsTabPlacement.h) {
                        commsVisible = !(commsVisible);
                    }
                    if (commsVisible == true) {
                        if (mouseX >= playPlacement.x && mouseX <= playPlacement.x + playPlacement.w &&
                                   mouseY >= playPlacement.y && mouseY <= playPlacement.y + playPlacement.h) {
                            audioPlaying = !(audioPlaying);
                            if (audioPlaying == true) {
                                Mix_PlayMusic(radioList[currTrack], -1);
                            } else {
                                Mix_PlayMusic(radioList[0], -1);
                            }
                        } else if (mouseX >= seekPlacement.x && mouseX <= seekPlacement.x + seekPlacement.w &&
                                   mouseY >= seekPlacement.y && mouseY <= seekPlacement.y + seekPlacement.h) {
                            Mix_PlayChannel(-1, noise, 0);
                            currTrack++;
                            if (currTrack >= totalTrack) currTrack = 1;
                            Mix_PlayMusic(radioList[currTrack], -1);
                            audioPlaying = true;
                        } else if (mouseX >= scanPlacement.x && mouseX <= scanPlacement.x + scanPlacement.w &&
                                   mouseY >= scanPlacement.y && mouseY <= scanPlacement.y + scanPlacement.h) {
                            Mix_PlayChannel(-1, noise, 0);
                            currTrack--;
                            if (currTrack < 1) currTrack = totalTrack - 1;
                            Mix_PlayMusic(radioList[currTrack], -1);
                            audioPlaying = true;
                        }
                    }

            }
            if (e.type == SDL_MOUSEMOTION && tracking == true) {
                int x, y;
                SDL_GetMouseState( &x, &y);
                int w; int h;
                SDL_GetWindowSize(myWindow, &w, &h);

                gameView->turnCameraBy((float)(mouseY - y) / (h/2), (float)(mouseX - x) / (w/2), 0);
                mouseX = x; mouseY = y;
            }
            if (e.type == SDL_MOUSEBUTTONUP) tracking = false;
        }

        float srtSpd = arwing->getSpd() * 60 / 100;
        float spd = 0;

        const uint8_t* currentKeyStates = SDL_GetKeyboardState(NULL);
        float turnRate = 0.001;
        if (currentKeyStates[SDL_SCANCODE_A]) {
            turning = true;
            if (fabs(arwing->getAngularVel_p()) < 0.05) arwing->accelerateAngular(turnRate * sin(arwing->getRoll()), 0, 0);
            if (fabs(arwing->getAngularVel_y()) < 0.05) {
                if (fabs(arwing->getPitch()) > M_PI / 2 && fabs(arwing->getPitch()) < M_PI * 3 / 2) {
                    arwing->accelerateAngular(0, turnRate * cos(arwing->getRoll()), 0);
                } else {
                    arwing->accelerateAngular(0, -turnRate * cos(arwing->getRoll()), 0);
                }
            }
            //arwing->rotateBy(0.02 * sin(arwing->getRoll()), -0.02 * cos(arwing->getRoll()), 0);
        }
        if (currentKeyStates[SDL_SCANCODE_D]) {
            turning = true;
            if (fabs(arwing->getAngularVel_p()) < 0.05) arwing->accelerateAngular(-turnRate * sin(arwing->getRoll()), 0, 0);
            if (fabs(arwing->getAngularVel_y()) < 0.05) {
                if (fabs(arwing->getPitch()) > M_PI / 2 && fabs(arwing->getPitch()) < M_PI * 3 / 2) {
                    arwing->accelerateAngular(0, -turnRate * cos(arwing->getRoll()), 0);
                } else {
                    arwing->accelerateAngular(0, turnRate * cos(arwing->getRoll()), 0);
                }
            }
            //arwing->rotateBy(-0.02 * sin(arwing->getRoll()), 0.02 * cos(arwing->getRoll()), 0);
        }
        if (currentKeyStates[SDL_SCANCODE_W]) {
            turning = true;
            if (fabs(arwing->getAngularVel_p()) < 0.05) arwing->accelerateAngular(-turnRate * cos(arwing->getRoll()), 0, 0);
            if (fabs(arwing->getAngularVel_y()) < 0.05) arwing->accelerateAngular(0, -turnRate * sin(arwing->getRoll()), 0);
            //arwing->rotateBy(-0.02 * cos(arwing->getRoll()), -0.02 * sin(arwing->getRoll()), 0);
        }
        if (currentKeyStates[SDL_SCANCODE_S]) {
            turning = true;
            if (fabs(arwing->getAngularVel_p()) < 0.05) arwing->accelerateAngular(turnRate * cos(arwing->getRoll()), 0, 0);
            if (fabs(arwing->getAngularVel_y()) < 0.05) arwing->accelerateAngular(0, turnRate * sin(arwing->getRoll()), 0);
            //arwing->rotateBy(0.02 * cos(arwing->getRoll()), 0.02 * sin(arwing->getRoll()), 0);
        }
        if (currentKeyStates[SDL_SCANCODE_Q]) {
            turning = true;
            if (fabs(arwing->getAngularVel_r()) < 0.05) arwing->accelerateAngular(0, 0, -turnRate);
            //arwing->rotateBy(0, 0, -0.02);
        }
        if (currentKeyStates[SDL_SCANCODE_E]) {
            turning = true;
            if (fabs(arwing->getAngularVel_r()) < 0.05) arwing->accelerateAngular(0, 0, turnRate);
            //arwing->rotateBy(0, 0, 0.02);
        }
        if (currentKeyStates[SDL_SCANCODE_LCTRL]) {
            arwing->brake(3);
        }
        if (currentKeyStates[SDL_SCANCODE_LALT]) {
            arwing->brakeAngular(0.001);
        }
        if (currentKeyStates[SDL_SCANCODE_LSHIFT]) {
            if ( fp > 0 && overheat == false ) {
                arwing->accelerate(3);
                fp -= 0.02;
                tp += 0.4;
                if (tp >= 100) overheat = true;
                controlsPlacement.w = 0;
                controlsPlacement.h = 0;

            }
        }
        /*
        if (currentKeyStates[SDL_SCANCODE_Z]) {
            gameView->changeFOV(10);
        }
        if (currentKeyStates[SDL_SCANCODE_X]) {
            gameView->changeFOV(-10);
        }
        */

        // game updATES

        // GRAVITY

        arwing->accelerate(10000 / sqrt( pow(arwing->getAnchor().x - planet->getAnchor().x, 2) +
                                         pow(arwing->getAnchor().y - planet->getAnchor().y, 2) +
                                         pow(arwing->getAnchor().z - planet->getAnchor().z, 2)),
                           planet->getAnchor().x - arwing->getAnchor().x,
                           planet->getAnchor().y - arwing->getAnchor().y,
                           planet->getAnchor().z - arwing->getAnchor().z);

        for (int i = 0; i < 100; i++) {
            if (satellites[i] == NULL) continue;

            satellites[i]->accelerate(10000 / sqrt( pow(satellites[i]->getAnchor().x - planet->getAnchor().x, 2) +
                                                    pow(satellites[i]->getAnchor().y - planet->getAnchor().y, 2) +
                                                    pow(satellites[i]->getAnchor().z - planet->getAnchor().z, 2)),
                                      planet->getAnchor().x - satellites[i]->getAnchor().x,
                                      planet->getAnchor().y - satellites[i]->getAnchor().y,
                                      planet->getAnchor().z - satellites[i]->getAnchor().z);

            satellites[i]->advanceFrame();
        }


        if (tp > 0) {
            tp -= 0.2;
            if (tp <= 0) overheat = false;
        }
        if (cp < 100) cp += 1;

        hpBarPlacement.w = windowW * 0.336 * (hp / 100);
        fuelBarPlacement.w = windowW * 0.336 * (fp / 100);
        chargeBarPlacement.w = windowW * 0.336 * (cp / 100);
        heatBarPlacement.w = windowW * 0.336 * (tp / 100);

        if (autoPilot == true) {
            if (turning == false) {
                arwing->brakeAngular(0.001);
            }
            arwing->accelerate(3);
            arwing->brake(3);
        }

        turning = false;
        arwing->advanceFrame();

        if (frameCount >= 60) frameCount = 0;
        else frameCount++;
        if (frameCount % 5 == 0) {
            if (arwing->getFaces()[5].green == 0) {
                arwing->getFaces()[5].green = 100;
                arwing->getFaces()[6].green = 100;
            } else {
                arwing->getFaces()[5].green = 0;
                arwing->getFaces()[6].green = 0;
            }
        }

        for (int i = 0; i < 50; i++) {
            if (lasers[i]->isVisible() == false) continue;
            else if ( abs((lasers[i]->getAnchor().x - arwing->getAnchor().x)) +
                      abs((lasers[i]->getAnchor().y - arwing->getAnchor().y)) +
                      abs((lasers[i]->getAnchor().z - arwing->getAnchor().z)) > 500000) {
                lasers[i]->brake(5000);
                lasers[i]->moveTo(0, 0, 0);
                lasers[i]->setVisible(false);
                continue;
            }
            lasers[i]->advanceFrame();
            lasers[i]->accelerate(5, lasers[i]->getVelocityVec_x(), lasers[i]->getVelocityVec_y(), lasers[i]->getVelocityVec_z());
        }

        //                                              planet->advanceFrame();

        // ****** Collision ******
        // collision effects MAY stack, pending testing
        p3d normal;
        for (int i = 0; i < 100; i++) {
            float s = satellites[i]->getSpd();

            if (arwing->polyCollide(satellites[i], normal)) {
                int dmgVal = arwing->getSpd() / 20;
                arwing->accelerate(-arwing->getSpd() * 0.9);
                hp -= dmgVal;
                for (int i= 0; i < arwing->getNumFaces(); i++) {
                    if (arwing->getFaces()[i].red + dmgVal <= 255) arwing->getFaces()[i].red += dmgVal;
                    if (arwing->getFaces()[i].green - dmgVal >= 0) arwing->getFaces()[i].green -= dmgVal;
                    if (arwing->getFaces()[i].blue - dmgVal >= 0) arwing->getFaces()[i].blue -= dmgVal;
                }
                satellites[i]->brake(-1);
                satellites[i]->accelerate(s, normal.x, normal.y, normal.z);
                continue;
            }

            if (satellites[i]->polyCollide(planet, normal)) {
                satellites[i]->brake(-1);
                satellites[i]->accelerate(s, normal.x, normal.y, normal.z);
                //std::cout << "bop ";
                continue;
            }

        }

        for (int i = 0; i < numStars; i++) {
            if (stars[i] == NULL) continue;
            if (arwing->polyCollide(stars[i])) {
                cp = 100; // huehuehue
                if (fp < 100) fp++;
                if (hp < 100) {
                    hp += 1;
                    for (int j = 0; j < arwing->getNumFaces(); j++) {
                        if (arwing->getFaces()[j].red - 1 >= 0) arwing->getFaces()[j].red -= 1;
                        if (arwing->getFaces()[j].green + 1 <= 255) arwing->getFaces()[j].green += 1;
                        if (arwing->getFaces()[j].blue + 1 <= 255) arwing->getFaces()[j].blue += 1;
                    }
                    // use star energy
                    for (int j = 0; j < stars[i]->getNumFaces(); j++) {
                        stars[i]->getFaces()[j].red -= 1;
                        stars[i]->getFaces()[j].green -= 2;
                        stars[i]->getFaces()[j].blue -= 2;
                    }
                    if ( stars[i]->getFaces()[0].red < 170) {
                        gameView->deleteObjectFromView(stars[i]->getID());
                        stars[i] = NULL;
                    }
                } else if (hp == 100) {
                    hp += 1;
                    for (int i= 0; i < arwing->getNumFaces(); i++) {
                        arwing->getFaces()[i].red = arwing->getOgFaces()[i].red;
                        arwing->getFaces()[i].green = arwing->getOgFaces()[i].green;
                        arwing->getFaces()[i].blue = arwing->getOgFaces()[i].blue;
                    }
                }
                break; // save extra checks, unnessisary;
            }
        }
        if (arwing->polyCollide(planet, normal)) {
            int dmgVal = arwing->getSpd() / 20;

            // go back 1 space
            //arwing->accelerate(-2 * arwing->getSpd());
            //arwing->advanceFrame();
            //arwing->brake(-1);
            float nDist = sqrt( pow(normal.x, 2) + pow(normal.y, 2) + pow(normal.z, 2) );

            arwing->moveBy(arwing->getSpd() * normal.x / nDist + 0,
                           arwing->getSpd() * normal.y / nDist + 0,
                           arwing->getSpd() * normal.z / nDist + 0);
            arwing->brake(10);



            hp -= dmgVal;
            for (int i= 0; i < arwing->getNumFaces(); i++) {
                if (arwing->getFaces()[i].red + dmgVal <= 255) arwing->getFaces()[i].red += dmgVal;
                if (arwing->getFaces()[i].green - dmgVal >= 0) arwing->getFaces()[i].green -= dmgVal;
                if (arwing->getFaces()[i].blue - dmgVal >= 0) arwing->getFaces()[i].blue -= dmgVal;
            }

        }

        //set camera behind
        gameView->setCameraBehind(arwing, !(tracking), srtSpd / 500 + 0.02);

        // ITS COMPUTING THE PLANE OF PROJECTION,
        //     AND CREATING A RELATIVELY STATIC POINT!!! HOYL SHIT!

        // gyro placement (shifted 1 over to account for delay)
        // construct Orthagonal Set based on camera sightVec as normal
        // move along the set as nessisary, relative to view (camera focal dist)
        if (warp == true) gyro->setVisible(false);
        else if (warp == false && gyro->isVisible() == false) gyro->setVisible(true);
        float a1 = gameView->getCamera().sightVec_x;
        float a2 = gameView->getCamera().sightVec_y;
        float a3 = gameView->getCamera().sightVec_z;
        float na = sqrt( pow(a1, 2) + pow(a2, 2) + pow(a3, 2) );
        a1 /= na; a2 /= na; a3 /= na;
        float d = sqrt(a2*a2 + a3*a3);

        // cross product with (0, 1, 0)
        float b1 = -1 * a3;
        float b2 = 0;
        float b3 = a1;

        // normalize
        float nb = sqrt( pow(b1, 2) + pow(b2, 2) + pow(b3, 2) );
        b1 /= nb;  b2 /= nb; b3 /= nb;

        float bb1 = b1;
        float bb2 = b2 * a3 / d - b3 * a2 / d;
        float bb3 = b2 * a2 / d + b3 * a3 / d;

        b1 = (bb1 * d) - (bb3 * a1);
        b2 = bb2;
        b3 = (bb1 * a1) + (bb3 * d);

        float rl = gameView->getCamera().roll;
        bb1 = b1 * cos(rl) - b2 * sin(rl);
        bb2 = b1 * sin(rl) + b2 * cos(rl);
        bb3 = b3;

        b1 = (bb1 * d) + (bb3 * a1);
        b2 = bb2;
        b3 = (-bb1 * a1) + (bb3 * d);

        bb1 = b1;
        bb2 =  b2 * a3 / d + b3 * a2 / d;
        bb3 = -b2 * a2 / d + b3 * a3 / d;

        b1 = bb1;
        b2 = bb2;
        b3 = bb3;

        float c1, c2, c3;
        // cross product
        c1 = (a2 * b3 - a3 * b2);
        c2 = (a3 * b1 - a1 * b3);
        c3 = (a1 * b2 - a2 * b1);
        float nc = sqrt( pow(c1, 2) + pow(c2, 2) + pow(c3, 2) );
        c1 /= nc; c2 /= nc; c3 /= nc;

        if (fabs(gameView->getCamera().pitch) < M_PI * 3 / 2 && fabs(gameView->getCamera().pitch) > M_PI / 2) {
            b1 *= -1; b2 *= -1; b3 *= -1;
            c1 *= -1; c2 *= -1; c3 *= -1;
        }

        gyro->moveTo(gameView->getCamera().x + (300 * a1) + (-255 * b1) + (100 * c1),
                     gameView->getCamera().y + (300 * a2) + (-255 * b2) + (100 * c2),
                     gameView->getCamera().z + (300 * a3) + (-255 * b3) + (100 * c3));
        //gyro->rotateTo(gameView->getCamera().pitch - arwing->getPitch() + arwing->getRoll() / 10, -gameView->getCamera().yaw + arwing->getYaw(), -gameView->getCamera().roll + arwing->getRoll());
        // gyro turning is along wrong axis (pitching), fixable?
        gameView->display();

        //Display HUD
        SDL_RenderCopy(myRenderer, controls, NULL, &controlsPlacement);
        SDL_RenderCopy(myRenderer, commsTab, NULL, &commsTabPlacement);
        SDL_RenderCopy(myRenderer, hpBar, NULL, &hpBarPlacement);
        SDL_RenderCopy(myRenderer, fuelBar, NULL, &fuelBarPlacement);
        SDL_RenderCopy(myRenderer, chargeBar, NULL, &chargeBarPlacement);
        SDL_RenderCopy(myRenderer, heatBar, NULL, &heatBarPlacement);
        if (overheat == true) SDL_RenderCopy(myRenderer, overheatLabel, NULL, &overheatPlacement);
        if (autoPilot == true) SDL_RenderCopy(myRenderer, steeringOn, NULL, &steeringPlacement);
        else SDL_RenderCopy(myRenderer, steeringOff, NULL, &steeringPlacement);

        SDL_RenderCopy(myRenderer, dash, NULL, NULL);
        SDL_Point origin = {windowW * 0.052, windowH * 0.0926};
        // assume 1 unit is 1cm, converts to m/s
        spd = arwing->getSpd() * 60 / 100;
        SDL_RenderCopyEx(myRenderer, dial, NULL, &dialPlacement, spd / 3, &origin, SDL_FLIP_NONE);

        // set spedometer text;
        SDL_DestroyTexture(spdTexture);
        if (spd >= 10000) spdTexture = loadTextureFromText("NULL", meterFont, textColor);
        else {
            std::ostringstream convert;
            int spdFormat = spd * 10;
            if (spdFormat < 100) convert << " 00";
            else if (spdFormat < 1000) convert << " 0";
            else if (spdFormat < 10000) convert << " ";
            convert << spdFormat / 10 << "." << spdFormat % 10;
            spdTexture = loadTextureFromText(convert.str(), meterFont, textColor);
        }
        SDL_RenderCopy(myRenderer, spdTexture, NULL, &spdTextPlacement);

        SDL_DestroyTexture(accTexture);
        {
            int accVal = (spd - srtSpd) * 60;
            std::ostringstream convert;
            if (accVal < 10 && accVal >= 0) convert << "   ";
            else if (accVal < 100 && accVal >= 0) convert << "  ";
            else if (accVal < 1000 && accVal >= 0) convert << " ";
            else if (accVal > -10 && accVal <= 0) convert << "  ";
            else if (accVal > -100 && accVal <= 0) convert << " ";
            convert << accVal;
            accTexture = loadTextureFromText(convert.str(), meterFont, textColor);
        }
        SDL_RenderCopy(myRenderer, accTexture, NULL, &accTextPlacement);

        SDL_DestroyTexture(rotTexture);
        {
            std::ostringstream convert;
            int radsFormat = (float)(fabs(arwing->getAngularVel_p()) + fabs(arwing->getAngularVel_y()) + fabs(arwing->getAngularVel_r())) * 2000;
            convert << radsFormat / 100 << "." << radsFormat / 10 << radsFormat % 10;
            rotTexture = loadTextureFromText(convert.str(), meterFont, textColor);
        }
        SDL_RenderCopy(myRenderer, rotTexture, NULL, &rotTextPlacement);

        SDL_DestroyTexture(trackText);
        trackText = loadTextureFromText(trackTextList[currTrack], meterFont, textColor);

        if (commsVisible == true) {
            SDL_RenderCopy(myRenderer, comms, NULL, &commsPlacement);
            SDL_RenderCopy(myRenderer, trackText, NULL, &trackTextPlacement);

            if (audioPlaying == true) {
                SDL_RenderCopy(myRenderer, audioSpec[audioFrame], NULL, &audioSpecPlacement);

                audioFrameDelay++;
                if (audioFrameDelay > 4) {
                    audioFrame++;
                    audioFrameDelay = 0;
                }
                if (audioFrame >= 20) audioFrame = 0;
            }
        }

        //Update screen
        SDL_RenderPresent( myRenderer );
    }

    SDL_DestroyTexture(dash);
    SDL_DestroyTexture(dial);
    SDL_DestroyTexture(comms);
    SDL_DestroyTexture(commsTab);
    SDL_DestroyTexture(hpBar);
    SDL_DestroyTexture(fuelBar);
    SDL_DestroyTexture(chargeBar);
    SDL_DestroyTexture(heatBar);
    SDL_DestroyTexture(steeringOff);
    SDL_DestroyTexture(steeringOn);

    for (int i = 0; i < 20; i++) {
        SDL_DestroyTexture(audioSpec[i]);
    }

    for (int i = 0; i < 14; i++) {
        Mix_FreeMusic(radioList[i]);
    }

    gameView->deleteAllObjectsFromView();

    return exitCode;

}

void StarFoxController::level1_intro() {
    Mix_Music* emergencyTheme = Mix_LoadMUS("resources/starfox-emergency-call.mp3");
    if (emergencyTheme == NULL) {
        std::cout << "Failed to load music. SDL_mixer Error: " << Mix_GetError() << std::endl;
    }
    Mix_VolumeMusic(musicVolume);
    Mix_PlayMusic(emergencyTheme, -1);

    Mix_Chunk* engineStart = Mix_LoadWAV("resources/Start_Engines.wav");
    if (engineStart == NULL) {
        std::cout << "Failed to load engineStart. SDL_mixer Error: " << Mix_GetError() << std::endl;
    } else engineStart->volume = SFXVolume;

    gameView->deleteAllObjectsFromView();
    gameView->setRenderDist(41000);
    gameView->changeFOV(400);

    gameView->moveCameraTo(-8250, 8000, 12200);
    int frames = 0;

    Object* crew[4];
    crew[0] = new Object("resources/arwing_struct.txt", {0, 900, 100});
    gameView->addObjectToView(crew[0]);
    crew[1] = new Object("resources/arwing_struct.txt", {-400, 1050, -600});
    gameView->addObjectToView(crew[1]);
    crew[2] = new Object("resources/arwing_struct.txt", {400, 1050, -800});
    gameView->addObjectToView(crew[2]);
    crew[3] = new Object("resources/arwing_struct.txt", {-250, 1200, -1500});
    gameView->addObjectToView(crew[3]);

    Object* structures[70];

    for (int i = 0; i < 30; i++ ) {
        structures[i] = new Object("resources/C_arch_struct.txt", {0, 0, 4000 * (i - 0) + 1000});
        gameView->addObjectToView(structures[i]);
    }
    for (int i = 30; i < 50; i++ ) {
        structures[i] = new Object("resources/C_tunnel_struct.txt", {0, 1000, 2000 * (i - 30) + 1000});
        gameView->addObjectToView(structures[i]);
    }
    for (int i = 50; i < 70; i++ ) {
        structures[i] = new Object("resources/C_tunnel_colour_struct.txt", {0, 1000, 3200 * (i - 50) + 42000});
        gameView->addObjectToView(structures[i]);
    }

    SDL_Event e;
    bool quit = false;

    while (!quit) {

        while( SDL_PollEvent(&e) != 0 ) {
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                quit = true;
            }
        }

        for (int i = 0; i < 4; i++) {
            crew[i]->moveBy(0, 0, 65);
            if (frames % 5 == 0) {
                if (crew[i]->getFaces()[5].green == 0) {
                    crew[i]->getFaces()[5].green = 100;
                    crew[i]->getFaces()[6].green = 100;
                } else {
                    crew[i]->getFaces()[5].green = 0;
                    crew[i]->getFaces()[6].green = 0;
                }
            }
        }
        gameView->turnCameraToward(crew[0]);

        gameView->display();

        frames++;
        if (frames >= 1200) quit = true;

        if (frames >= 250) {
            gameView->moveCameraBy(0, 0, 71);
            if (gameView->getCamera().y >= 900) {
                gameView->moveCameraBy(22, -19, 0);
            }
        }

        if (frames < 811) {
            crew[0]->rotateTo(0, 0, cos( (float)((frames % 120) - 60) / 20) / 10);
            crew[1]->rotateTo(0, 0, cos( (float)(((frames + 20) % 120) - 60) / 20) / 10);
            crew[2]->rotateTo(0, 0, cos( (float)(((frames + 40) % 120) - 60) / 20) / 10);
            crew[3]->rotateTo(0, 0, cos( (float)(((frames + 70) % 120) - 60) / 20) / 10);
        }

        if (frames == 850) Mix_PlayChannel(1, engineStart, 0);
        else if (frames > 850) crew[0]->moveBy(0, 0, 100);
        if (frames == 800) Mix_PlayChannel(2, engineStart, 0);
        else if (frames > 800) crew[1]->moveBy(0, 0, 100);
        if (frames == 750) Mix_PlayChannel(3, engineStart, 0);
        else if (frames > 750) crew[2]->moveBy(0, 0, 100);

        //Update screen
        SDL_RenderPresent( myRenderer );
    }

    gameView->deleteAllObjectsFromView();
    //delete[] structures;
    //delete[] crew;
    Mix_PauseMusic();
    if (Mix_Playing(1)) Mix_Pause(1);
    if (Mix_Playing(2)) Mix_Pause(2);
    if (Mix_Playing(3)) Mix_Pause(3);
    Mix_FreeChunk(engineStart);
    Mix_FreeMusic(emergencyTheme);
}

GameState StarFoxController::level1() {

    this->level1_intro();

    SDL_Rect controlsPlacement = {windowW * 0.3, 0, windowW * 0.3458, windowH * 0.3093};
    SDL_Texture* controls = loadTextureFromFile("resources/1_controls.png");

    Mix_Music* starfoxTheme = Mix_LoadMUS("resources/starfox_metal.wav");
    if (starfoxTheme == NULL) {
        std::cout << "Failed to load music. SDL_mixer Error: " << Mix_GetError() << std::endl;
    }
    Mix_VolumeMusic(musicVolume);
    Mix_PlayMusic(starfoxTheme, -1);

    Mix_Chunk* dead = Mix_LoadWAV("resources/Explosion.wav");
    if (dead == NULL) {
        std::cout << "Failed to load dead. SDL_mixer Error: " << Mix_GetError() << std::endl;
    } else dead->volume = SFXVolume;

    gameView->setBackGround(loadTextureFromFile("resources/StarFox-Corneria.png"));

    StarFoxGame* gameModel = new StarFoxGame(gameView);
    gameModel->setSFXVolume(SFXVolume);


    SDL_Event e;

    int mouseX;
    int mouseY;
    bool quit = false;

    //INTRO PAN
    while (!quit) {
        while( SDL_PollEvent(&e) != 0 ) {
            if (e.type == SDL_QUIT) return END_GAME;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                gameModel->getArwing()->moveTo(0, 620, 13000);
                //gameView->moveCameraTo(0, 200, -2200);
                //gameView->turnCameraToward(gameModel->getArwing());
                break;
            }
        }

        gameModel->update();

        if (gameModel->getArwing()->getAnchor().z > 13000) quit = true;

        //Update screen
        SDL_RenderPresent( myRenderer );
    }

    quit = false;
    int exitFrames = 0;
    GameState exitCode;
    while(!quit) {

        if (gameModel->getArwingHP() <= 0) {

            gameModel->moveDown();
            gameModel->barrelRoll();
            if (exitFrames == 0) {
                Mix_PlayChannel(-1, dead, 0);
                exitFrames = 100;
            }
            exitFrames--;
            if (exitFrames <= 1) {
                exitCode = MAIN_MENU;
                quit = true;
            }

            gameModel->update();

            //Update screen
            SDL_RenderPresent( myRenderer );
            continue;
        }

        while( SDL_PollEvent(&e) != 0 ) {
            if (e.type == SDL_QUIT) {
                quit = true;
                exitCode = END_GAME;
            }
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                if (pause() == true) {
                    quit = true;
                    exitCode = MAIN_MENU;
                } else {
                    // refresh elements after window/volume change
                    gameModel->setSFXVolume(SFXVolume);
                    dead->volume = SFXVolume;
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                SDL_GetMouseState(&mouseX, &mouseY);

            }
            if (e.type == SDL_MOUSEMOTION) {
                int x, y;
                SDL_GetMouseState( &x, &y);
                int w; int h;
                SDL_GetWindowSize(myWindow, &w, &h);
                mouseX = x; mouseY = y;
            }
            if (e.type == SDL_MOUSEBUTTONUP){

            }
        }

        const uint8_t* currentKeyStates = SDL_GetKeyboardState(NULL);
        if (currentKeyStates[SDL_SCANCODE_A]) {
            gameModel->moveLeft();
        }
        if (currentKeyStates[SDL_SCANCODE_D]) {
            gameModel->moveRight();
        }
        if (currentKeyStates[SDL_SCANCODE_S]) {
            gameModel->moveUp();
        }
        if (currentKeyStates[SDL_SCANCODE_W]) {
            gameModel->moveDown();
        }
        if (currentKeyStates[SDL_SCANCODE_Q]) {
            gameModel->roll(false);
        }
        if (currentKeyStates[SDL_SCANCODE_E]) {
            gameModel->roll(true);
        }
        if (currentKeyStates[SDL_SCANCODE_LSHIFT]) {
            gameModel->boosters();
        }
        if (currentKeyStates[SDL_SCANCODE_SPACE]) {
                gameModel->shootLaser();
                controlsPlacement.y = -500;
        }
/*
        if (currentKeyStates[SDL_SCANCODE_Z]) {
            gameView->changeFOV(10);
        }
        if (currentKeyStates[SDL_SCANCODE_X]) {
            gameView->changeFOV(-10);
        }
*/
        // game updATES

        gameModel->update();

        //TEMP EJECT
        if (gameModel->getArwing()->getAnchor().z >= 260000) {
            quit = true;
            exitCode = MAIN_MENU;
        }

        // HUD
        SDL_RenderCopy(myRenderer, controls, NULL, &controlsPlacement);

        //Update screen
        SDL_RenderPresent( myRenderer );
    }

    delete gameModel;
    SDL_DestroyTexture(controls);
    Mix_FreeMusic(starfoxTheme);
    Mix_FreeChunk(dead);

    gameView->setBackGround(NULL);

    return exitCode;
}


SDL_Texture* StarFoxController::loadTextureFromFile(std::string filePath) {

    SDL_Surface* loadedSurface = IMG_Load(filePath.c_str());
    if (loadedSurface == NULL) {
        std::cout << "Unable to load image " << filePath.c_str() << " SDL_image Error: " << IMG_GetError() << std::endl;
    }

    SDL_Texture* t = SDL_CreateTextureFromSurface(myRenderer, loadedSurface);
    if (t == NULL) {
        std::cout << "Unable to create texture from " << filePath.c_str() << " SDL Error: " << SDL_GetError() << std::endl;
    }
    //***    Get dimensions??
    SDL_FreeSurface(loadedSurface);

    return t;
}

SDL_Texture* StarFoxController::loadTextureFromText(std::string txt, TTF_Font* font, SDL_Color colour) {

    SDL_Surface* loadedSurface = TTF_RenderText_Solid(font, txt.c_str(), colour);
    if (loadedSurface == NULL) {
        std::cout << "Unable to render text surface! SDL_ttf Error " << TTF_GetError() << std::endl;
        return NULL;
    }

    SDL_Texture* t = SDL_CreateTextureFromSurface(myRenderer, loadedSurface);
    if (t == NULL) {
        std::cout << "Unable to create texture from '" << txt << "' SDL Error: " << SDL_GetError() << std::endl;
    }
    SDL_FreeSurface(loadedSurface);

    return t;
}

bool StarFoxController::pause() {

    Mix_Chunk* buttonDown = Mix_LoadWAV( "resources/menuSelect.wav" );
    if (buttonDown == NULL) {
        std::cout << "Failed to load buttonDown. SDL_mixer Error: " << Mix_GetError() << std::endl;
    } else {
        buttonDown->volume = SFXVolume;
    }

    SDL_Rect screenPlacement = {windowW * 0.1041, windowH * 0.09259, windowW * 0.8052, windowH * 0.8093};
    SDL_Texture* screen = loadTextureFromFile("resources/options_creen.png");

    SDL_Rect backButtonPlacement = {windowW * 0.65, windowH * 0.7037, windowW * 0.1469, windowH * 0.124};
    SDL_Texture* backButtonOff = loadTextureFromFile("resources/backButton_Off.png");
    SDL_Texture* backButtonHover = loadTextureFromFile("resources/backButton_Hover.png");
    bool backButtonState = 0;

    SDL_Rect musicPlacement = {musicVolume * (windowW * 0.0045) + (windowW * 0.62) - (windowW * 0.1823), windowH * 0.1666, windowW * 0.0416, windowH * 0.074};
    bool adjustMusic = 0;
    SDL_Rect SFXPlacement = {SFXVolume * (windowW * 0.0045) + (windowW * 0.62) - (windowW * 0.1823), windowH * 0.305, windowW * 0.0416, windowH * 0.074};
    bool adjustSFX = 0;
    SDL_Rect brightnessPlacement = {SDL_GetWindowBrightness(myWindow) * (windowW * 0.15) + (windowW * 0.62) - (windowW * 0.1823), windowH * 0.444, windowW * 0.0416, windowH * 0.074};
    bool adjustBrightness = 0;
    SDL_Rect wireframePlacement = {windowW * 0.4479, windowH * 0.59, windowW * 0.0416, windowH * 0.074};
    SDL_Rect polygonPlacement = {windowW * 0.6458, windowH * 0.59, windowW * 0.0416, windowH * 0.074};
    SDL_Rect fullscreenPlacement = {windowW * 0.4479, windowH * 0.7407, windowW * 0.0416, windowH * 0.074};
    SDL_Texture* toggleButtonBlue = loadTextureFromFile("resources/toggleButton_blue.png");
    SDL_Texture* toggleButtonRed = loadTextureFromFile("resources/toggleButton_red.png");
    SDL_Texture* toggleButtonGreen = loadTextureFromFile("resources/toggleButton_green.png");

    //for in game
    SDL_Rect quitButtonPlacement = {0, 0, 0, 0};
    if (this->nextState != D3_SWITCH && this->nextState != MAIN_MENU) {
        quitButtonPlacement = {windowW * 0.32, windowH * 0.85, windowW * 0.3606, windowH * 0.1065}; //windowW * 0.43275, windowH * 0.1277};
    }
    SDL_Texture* quitButtonOff = loadTextureFromFile("resources/QuitMission_Off.png");
    SDL_Texture* quitButtonHover = loadTextureFromFile("resources/QuitMission_Hover.png");
    bool quitButtonState = 0;

    SDL_Event e;

    int mouseX, mouseY;
    bool quit = false;
    int returnVal = 0;

    while(!quit) {
        while( SDL_PollEvent(&e) != 0 ) {
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                quit = true;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                    SDL_GetMouseState(&mouseX, &mouseY);
                    if (mouseX >= fullscreenPlacement.x && mouseX <= fullscreenPlacement.x + fullscreenPlacement.w &&
                        mouseY >= fullscreenPlacement.y && mouseY <= fullscreenPlacement.y + fullscreenPlacement.h) {
                        if (SDL_GetWindowFlags(myWindow) & SDL_WINDOW_FULLSCREEN_DESKTOP) {
                            SDL_SetWindowFullscreen(myWindow, 0);
                        } else SDL_SetWindowFullscreen(myWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
                        SDL_GetWindowSize(myWindow, &windowW, &windowH);
                        gameView->resizeScreen();
                        SDL_RenderClear(myRenderer);
                        Mix_PlayChannel( -1, buttonDown, 0 );
                        pause();
                        quit = true; // recurces so that elements refresh
                    } else if (mouseX >= polygonPlacement.x && mouseX <= polygonPlacement.x + polygonPlacement.w &&
                               mouseY >= polygonPlacement.y && mouseY <= polygonPlacement.y + polygonPlacement.h) {
                        if (gameView->getRenderMode() == WIREFRAME) {
                            gameView->setRenderMode(FILLED_WIREFRAME);
                        } else if (gameView->getRenderMode() == FILLED_WIREFRAME) {
                            gameView->setRenderMode(WIREFRAME);
                        } else gameView->setRenderMode(FILLED);
                        Mix_PlayChannel( -1, buttonDown, 0 );
                    } else if (mouseX >= wireframePlacement.x && mouseX <= wireframePlacement.x + wireframePlacement.w &&
                               mouseY >= wireframePlacement.y && mouseY <= wireframePlacement.y + wireframePlacement.h) {
                        if (gameView->getRenderMode() == FILLED) {
                            gameView->setRenderMode(FILLED_WIREFRAME);
                        } else if (gameView->getRenderMode() == FILLED_WIREFRAME) {
                            gameView->setRenderMode(FILLED);
                        } else gameView->setRenderMode(WIREFRAME);
                        Mix_PlayChannel( -1, buttonDown, 0 );
                    } else if (mouseX >= backButtonPlacement.x && mouseX <= backButtonPlacement.x + backButtonPlacement.w &&
                               mouseY >= backButtonPlacement.y && mouseY <= backButtonPlacement.y + backButtonPlacement.h) {
                        //Mix_PlayChannel( -1, buttonDown, 0 );
                        quit = true;
                    } else if (mouseX >= quitButtonPlacement.x && mouseX <= quitButtonPlacement.x + quitButtonPlacement.w &&
                               mouseY >= quitButtonPlacement.y && mouseY <= quitButtonPlacement.y + quitButtonPlacement.h) {
                        quit = true;
                        returnVal = 1;
                    } else if (mouseX >= musicPlacement.x && mouseX <= musicPlacement.x + musicPlacement.w &&
                               mouseY >= musicPlacement.y && mouseY <= musicPlacement.y + musicPlacement.h) {
                        adjustMusic = 1;
                    } else if (mouseX >= SFXPlacement.x && mouseX <= SFXPlacement.x + SFXPlacement.w &&
                               mouseY >= SFXPlacement.y && mouseY <= SFXPlacement.y + SFXPlacement.h) {
                        adjustSFX = 1;
                    } else if (mouseX >= brightnessPlacement.x && mouseX <= brightnessPlacement.x + brightnessPlacement.w &&
                               mouseY >= brightnessPlacement.y && mouseY <= brightnessPlacement.y + brightnessPlacement.h) {
                        adjustBrightness = 1;
                    }
            }
            if (e.type == SDL_MOUSEMOTION) {
                int x = mouseX;
                SDL_GetMouseState(&mouseX, &mouseY);
                int Dx = mouseX - x;
                // 350 px total dist -> windowW * 0.3646
                // middle @ WindowW * 0.625
                // 0.4427 -> 0.8073
                if (adjustMusic == 1) {
                    musicPlacement.x += Dx;
                    if (musicPlacement.x > windowW * 0.8073) musicPlacement.x = windowW * 0.8073;
                    if (musicPlacement.x < windowW * 0.4427) musicPlacement.x = windowW * 0.4427;
                    musicVolume = ((float)musicPlacement.x - (windowW * 0.62) + (windowW * 0.1823)) / (windowW * 0.0045);
                    Mix_VolumeMusic(musicVolume);
                } else if (adjustSFX == 1) {
                    SFXPlacement.x += Dx;
                    if (SFXPlacement.x > windowW * 0.8073) SFXPlacement.x = windowW * 0.8073;
                    if (SFXPlacement.x < windowW * 0.4427) SFXPlacement.x = windowW * 0.4427;
                    SFXVolume = ((float)SFXPlacement.x - (windowW * 0.62) + (windowW * 0.1823)) / (windowW * 0.0045);
                    Mix_VolumeChunk(buttonDown, SFXVolume);
                } else if (adjustBrightness == 1) {
                    brightnessPlacement.x += Dx;
                    if (brightnessPlacement.x > windowW * 0.8073) brightnessPlacement.x = windowW * 0.8073;
                    if (brightnessPlacement.x < windowW * 0.4427) brightnessPlacement.x = windowW * 0.4427;
                    SDL_SetWindowBrightness(myWindow, ((float)brightnessPlacement.x - (windowW * 0.62) + (windowW * 0.1823)) / (windowW * 0.15));
                }

                if (mouseX >= backButtonPlacement.x && mouseX <= backButtonPlacement.x + backButtonPlacement.w &&
                    mouseY >= backButtonPlacement.y && mouseY <= backButtonPlacement.y + backButtonPlacement.h) {
                    backButtonState = 1;
                } else backButtonState = 0;
                if (mouseX >= quitButtonPlacement.x && mouseX <= quitButtonPlacement.x + quitButtonPlacement.w &&
                    mouseY >= quitButtonPlacement.y && mouseY <= quitButtonPlacement.y + quitButtonPlacement.h) {
                    quitButtonState = 1;
                } else quitButtonState = 0;

            }
            if (e.type == SDL_MOUSEBUTTONUP) {
                if (adjustMusic == 1 || adjustSFX == 1 || adjustBrightness == 1) {
                    adjustMusic = 0;
                    adjustSFX = 0;
                    adjustBrightness = 0;
                    Mix_PlayChannel( -1, buttonDown, 0 );
                }
            }
        }

        //render options screen
        SDL_RenderCopy(myRenderer, screen, NULL, &screenPlacement);

        if (backButtonState == 0) {
            SDL_RenderCopy(myRenderer, backButtonOff, NULL, &backButtonPlacement);
        } else SDL_RenderCopy(myRenderer, backButtonHover, NULL, &backButtonPlacement);

        SDL_RenderCopy(myRenderer, toggleButtonBlue, NULL, &musicPlacement);
        SDL_RenderCopy(myRenderer, toggleButtonBlue, NULL, &SFXPlacement);
        SDL_RenderCopy(myRenderer, toggleButtonBlue, NULL, &brightnessPlacement);

        if (gameView->getRenderMode() == WIREFRAME || gameView->getRenderMode() == FILLED_WIREFRAME) {
        SDL_RenderCopy(myRenderer, toggleButtonGreen, NULL, &wireframePlacement);
        } else SDL_RenderCopy(myRenderer, toggleButtonRed, NULL, &wireframePlacement);

        if (gameView->getRenderMode() == FILLED || gameView->getRenderMode() == FILLED_WIREFRAME) {
            SDL_RenderCopy(myRenderer, toggleButtonGreen, NULL, &polygonPlacement);
        }else SDL_RenderCopy(myRenderer, toggleButtonRed, NULL, &polygonPlacement);

        if (SDL_GetWindowFlags(myWindow) & SDL_WINDOW_FULLSCREEN) {
            SDL_RenderCopy(myRenderer, toggleButtonGreen, NULL, &fullscreenPlacement);
        } else SDL_RenderCopy(myRenderer, toggleButtonRed, NULL, &fullscreenPlacement);

        if (quitButtonState == false) {
            SDL_RenderCopy(myRenderer, quitButtonOff, NULL, &quitButtonPlacement);
        } else SDL_RenderCopy(myRenderer, quitButtonHover, NULL, &quitButtonPlacement);

        // dont clear renderer
        //Update screen
        SDL_RenderPresent( myRenderer );
    }

    Mix_FreeChunk(buttonDown);

    SDL_DestroyTexture(screen);
    SDL_DestroyTexture(backButtonOff);
    SDL_DestroyTexture(backButtonHover);
    SDL_DestroyTexture(toggleButtonBlue);
    SDL_DestroyTexture(toggleButtonRed);
    SDL_DestroyTexture(toggleButtonGreen);
    return returnVal;
}
