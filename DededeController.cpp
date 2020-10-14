#include "DededeController.hpp"

DededeController::DededeController(SDL_Window* inheritedWindow, SDL_Renderer* inheritedRenderer) : GameController() {
    myWindow = inheritedWindow;
    // keep title a secret
    SDL_SetWindowTitle(myWindow, "");
    myRenderer = inheritedRenderer;
    if (myWindow == NULL) {
        std::cout << "DededeController inherited bogus window" << std::endl;
        delete this;
    }
    if (myRenderer == NULL) {
        std::cout << "DededeController inherited bogus renderer" << std::endl;
        delete this;
    }

    background = loadTextureFromFile("resources/Sea_Sky_Background.png");
    logo = loadTextureFromFile("resources/Dedede_logo.png");
    surfingDedede = loadTextureFromFile("resources/Dedede_surfing.png");
    dededeHeight = 0;
    lensFlare = loadTextureFromFile("resources/lens-flare-effect.png");
    play_spritesheet = loadTextureFromFile("resources/Play_spritesheet.png");
    quit_spritesheet = loadTextureFromFile("resources/Quit_spritesheet.png");
    wave[0] = loadTextureFromFile("resources/surf0.png");
    wave[1] = loadTextureFromFile("resources/surf1.png");
    wave[2] = loadTextureFromFile("resources/surf2.png");
    wave[3] = loadTextureFromFile("resources/surf3.png");
    wave[4] = loadTextureFromFile("resources/surf4.png");
    wave[5] = loadTextureFromFile("resources/surf5.png");
    wave[6] = loadTextureFromFile("resources/surf6.png");
    wave[7] = loadTextureFromFile("resources/surf7.png");
    wave[8] = loadTextureFromFile("resources/surf8.png");
    wave[9] = loadTextureFromFile("resources/surf9.png");
    wave[10] = loadTextureFromFile("resources/surf10.png");
    wave[11] = loadTextureFromFile("resources/surf11.png");
    sparkle[0] = loadTextureFromFile("resources/sparkle0.png");
    sparkle[1] = loadTextureFromFile("resources/sparkle1.png");
    sparkle[2] = loadTextureFromFile("resources/sparkle2.png");
    sparkle[3] = loadTextureFromFile("resources/sparkle3.png");
    sparkle[4] = loadTextureFromFile("resources/sparkle4.png");
    sparkle[5] = loadTextureFromFile("resources/sparkle5.png");
    waveFrame = 0;
    introFrames = 600; // 10 second intro
    exitFrames = 0;
    exitCode = END_GAMES;
    playButtonMode = 0;
    quitButtonMode = 0;

    credits = loadTextureFromFile("resources/cpyr.png");
    opening = loadTextureFromFile("resources/opening.png");
    justKidding = loadTextureFromFile("resources/goof.png");
    repeat = loadTextureFromFile("resources/symbol_repeat.png");

    dededeTheme = Mix_LoadMUS("resources/King-Dedede-Theme.wav"); // Lightning_Mandrill.wav King-Dedede-Theme.wav
    if (dededeTheme == NULL) {
        std::cout << "Failed to load music. SDL_mixer Error: " << Mix_GetError() << std::endl;
    } else {
        //Mix_PlayMusic(dededeTheme, -1);
        Mix_VolumeMusic(20);
    }

    waveSound = Mix_LoadWAV( "resources/oceanwave.wav" );
    if (waveSound == NULL) {
        std::cout << "Failed to load waveSound. SDL_mixer Error: " << Mix_GetError() << std::endl;
    } else {
        Mix_PlayChannel( -1, waveSound, 100000 );
        waveSound->volume = 30;
    }

    clash = Mix_LoadWAV( "resources/Swords_Collide.wav" );
    if (clash == NULL) {
        std::cout << "Failed to load clash. SDL_mixer Error: " << Mix_GetError() << std::endl;
    }

    menuSelect = Mix_LoadWAV( "resources/menuSelect.wav" );
    if (menuSelect == NULL) {
        std::cout << "Failed to load menuSelect. SDL_mixer Error: " << Mix_GetError() << std::endl;
    }
    menuConfirm = Mix_LoadWAV( "resources/menuConfirm.wav" );
    if (menuConfirm == NULL) {
        std::cout << "Failed to load menuConfirm. SDL_mixer Error: " << Mix_GetError() << std::endl;
    }

    dededeLaugh = Mix_LoadWAV("resources/Dedede_laugh.wav");
    if (dededeLaugh == NULL) {
        std::cout << "Failed to load dededeLaugh. SDL_mixer Error: " << Mix_GetError() << std::endl;
    }
}


DededeController::~DededeController() {
    SDL_FreeSurface(windowIcon);
    SDL_DestroyTexture(background);
    SDL_DestroyTexture(logo);
    SDL_DestroyTexture(surfingDedede);
    for (int i = 0; i < 12; i++) SDL_DestroyTexture(wave[i]);
    for (int i = 0; i < 6; i++) SDL_DestroyTexture(sparkle[i]);
    SDL_DestroyTexture(play_spritesheet);
    SDL_DestroyTexture(quit_spritesheet);
    SDL_DestroyTexture(credits);
    SDL_DestroyTexture(opening);
    SDL_DestroyTexture(justKidding);
    SDL_DestroyTexture(repeat);
    Mix_FreeMusic(dededeTheme);
    Mix_FreeChunk(waveSound);
    Mix_FreeChunk(clash);
    Mix_FreeChunk(menuConfirm);
    Mix_FreeChunk(menuSelect);
    Mix_FreeChunk(dededeLaugh);
    //Renderer is sharded globally now
    //SDL_DestroyRenderer(myRenderer);
}

GameType DededeController::play() {

    windowIcon = IMG_Load("resources/NULL_Icon.png");
	if (windowIcon == NULL) {
        std::cout << "Unable to load image 'resources/NULL_Icon.png' SDL_image Error: " << IMG_GetError() << std::endl;
    }
	SDL_SetWindowIcon(myWindow, windowIcon);

    SDL_Event e;

    while(1) {

        if (exitFrames > 0) {
            exitFrames--;
            if (exitFrames == 0) {
                if (exitCode == STARFOX_TEMPURATURES) {
                    SDL_SetRenderDrawColor( myRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE );
                    SDL_RenderClear(myRenderer);
                    SDL_RenderCopy(myRenderer, justKidding, NULL, NULL);
                    //Update screen
                    SDL_RenderPresent( myRenderer );
                    Mix_HaltMusic();
                    Mix_HaltChannel(-1);
                    SDL_Delay(1500);
                    return exitCode;
                }
                    return exitCode;
            }else {
                this->display();
                continue;
            }
        }
        while( SDL_PollEvent(&e) != 0 ) {
            if (e.type == SDL_QUIT) {
                return END_GAMES;
            }

            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_SPACE && introFrames > 0) introFrames = 0;
            }

            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);
            if (e.type == SDL_MOUSEMOTION && introFrames < 0) {
                if (mouseX >= playPlacement.x && mouseX <= playPlacement.x + playPlacement.w &&
                    mouseY >= playPlacement.y && mouseY <= playPlacement.y + playPlacement.h) {
                    if (playButtonMode != 1) {
                        playButtonMode = 1;
                        Mix_PlayChannel( -1, menuSelect, 0 );
                    }
                } else playButtonMode = 0;
                if (mouseX >= quitPlacement.x && mouseX <= quitPlacement.x + quitPlacement.w &&
                    mouseY >= quitPlacement.y && mouseY <= quitPlacement.y + quitPlacement.h) {
                    if (quitButtonMode != 1) {
                        quitButtonMode = 1;
                        Mix_PlayChannel( -1, menuSelect, 0 );
                    }
                } else quitButtonMode = 0;
                if (mouseX >= dededePlacement.x && mouseX <= dededePlacement.x + dededePlacement.w &&
                    mouseY >= dededePlacement.y && mouseY <= dededePlacement.y + dededePlacement.h) {
                    if (dededeHover == false) {
                        SDL_DestroyTexture(surfingDedede);
                        surfingDedede = loadTextureFromFile("resources/Dedede_surfing_select.png");
                        dededeHover = true;
                    }
                } else {
                    if (dededeHover == true) {
                        SDL_DestroyTexture(surfingDedede);
                        surfingDedede = loadTextureFromFile("resources/Dedede_surfing.png");
                        dededeHover = false;
                    }
                }
            }
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (introFrames < 0) {

                if (mouseX >= playPlacement.x && mouseX <= playPlacement.x + playPlacement.w &&
                    mouseY >= playPlacement.y && mouseY <= playPlacement.y + playPlacement.h) {
                    Mix_PlayChannel( 2, menuConfirm, 0 );
                    //just kidding split to actual game now
                    exitCode = STARFOX_TEMPURATURES;
                    exitFrames = 40;
                }
                if (mouseX >= quitPlacement.x && mouseX <= quitPlacement.x + quitPlacement.w &&
                    mouseY >= quitPlacement.y && mouseY <= quitPlacement.y + quitPlacement.h) {
                    Mix_PlayChannel( -1, menuConfirm, 0 );
                    exitCode = END_GAMES;
                    exitFrames = 40;
                }
                if (mouseX >= repeatPlacement.x && mouseX <= repeatPlacement.x + repeatPlacement.w &&
                    mouseY >= repeatPlacement.y && mouseY <= repeatPlacement.y + repeatPlacement.h) {
                    Mix_PlayChannel( -1, menuConfirm, 0 );
                    exitCode = DEDEDE_MEMELAND;
                    exitFrames = 40;
                }
                if (mouseX >= dededePlacement.x && mouseX <= dededePlacement.x + dededePlacement.w &&
                    mouseY >= dededePlacement.y && mouseY <= dededePlacement.y + dededePlacement.h) {
                    Mix_PlayChannel( -1, dededeLaugh, 0 );
                }

                }
            }
        }

        this->display();
    }

}

void DededeController::display() {
    SDL_SetRenderDrawColor( myRenderer, 245, 255, 255, SDL_ALPHA_OPAQUE );
	SDL_RenderClear( myRenderer );

	SDL_GetWindowSize(myWindow, &windowW, &windowH);

	if (introFrames >= 0 ) renderIntro();
	else if (introFrames <= 0 && introFrames >= -25) {
            SDL_SetWindowTitle(myWindow, "King Dedede's Return To MemeLand");
            SDL_RenderClear( myRenderer );
            introFrames--;
	} else renderMain();


	//Update screen
    SDL_RenderPresent( myRenderer );
}

void DededeController::renderIntro() {
    //bground starts 330 over
    SDL_Rect backgroundPlacement = {-0.059 * windowW + (introFrames * windowW * 0.0001), 0, 1.883 * windowH, windowH};
	SDL_RenderCopy(myRenderer, background, NULL, &backgroundPlacement);

	if (introFrames == 356) Mix_PlayMusic(dededeTheme, -1);

	if (introFrames <= 50) {
        dededePlacement = {0.54 * windowW + (introFrames * windowW * 0.01), windowH*0.28 + (cos(dededeHeight) * windowH * 0.0583), windowH * 0.6236, windowH * 0.5};
        dededeHeight += M_PI / 50;
        if (dededeHeight >= M_PI * 2) dededeHeight = 0;
        SDL_RenderCopy(myRenderer, surfingDedede, NULL, &dededePlacement);

        SDL_Rect wavePlacement = {windowW * 0.315 + (introFrames * windowW * 0.01), windowH * 0.225, windowH * 1.218, windowH* 0.91};
        SDL_RenderCopy(myRenderer, wave[waveFrame/4], NULL, &wavePlacement);
        waveFrame++;
        if (waveFrame >= 48) waveFrame = 0;
	}

	if (introFrames >= 100) {
        SDL_Rect openingPlacement = {windowW * 0.28, windowH * 0.366, windowH * 0.7116, windowH * 0.16};
        Uint8 alphaMod = (-1 * abs(introFrames - 350) + 250 );
        if (alphaMod < 0) alphaMod = 0;
        SDL_SetTextureAlphaMod(opening, alphaMod);
        SDL_RenderCopy(myRenderer, opening, NULL, &openingPlacement);
	}

	if (introFrames == 0) {
            SDL_FreeSurface(windowIcon);
            windowIcon = IMG_Load("resources/d3_Icon.png");
            if (windowIcon == NULL) {
                std::cout << "Unable to load image 'resources/d3_logo.png' SDL_image Error: " << IMG_GetError() << std::endl;
            }
            SDL_SetWindowIcon(myWindow, windowIcon);
            //Mix_PlayChannel( -1, clash, 0 );
            Mix_VolumeMusic(70);
            if (Mix_PlayingMusic() == false) Mix_PlayMusic(dededeTheme, -1);
	}

	introFrames--;
}

void DededeController::renderMain() {


    SDL_Rect backgroundPlacement = { -0.059 * windowW, 0, 1.883 * windowH, windowH};
	SDL_RenderCopy(myRenderer, background, NULL, &backgroundPlacement);

	dededePlacement = {0.54 * windowW + sin(dededeHeight) * windowW*0.0125, windowH*0.28 + (cos(dededeHeight) * windowH * 0.0583), windowH * 0.6236, windowH * 0.5};
	dededeHeight += M_PI / 40;
	if (dededeHeight >= M_PI * 2) dededeHeight = 0;
	SDL_RenderCopy(myRenderer, surfingDedede, NULL, &dededePlacement);

	SDL_Rect wavePlacement = {windowW * 0.315, windowH * 0.225, windowH * 1.218, windowH* 0.91};
	SDL_Rect waveClip = {2,2,731,546};
	SDL_RenderCopy(myRenderer, wave[waveFrame/4], &waveClip , &wavePlacement);
	waveFrame++;
	if (waveFrame >= 48) waveFrame = 0;

    SDL_Rect lensFlarePlacement = {windowW * 0.44, 0, windowH * 1.0, windowH * 0.75};
	SDL_RenderCopy(myRenderer, lensFlare, NULL, &lensFlarePlacement);

	SDL_Rect buttonOff = {0, 0, 200, 75};
	SDL_Rect buttonHover = {198, 0, 200, 75};
	SDL_Rect buttonDown = {198, 75, 200, 75};

    playPlacement = {windowW * 0.155, windowH * 0.6, windowH * 0.466, windowH * 0.166};
    quitPlacement = {windowW * 0.155, windowH * 0.77, windowH * 0.466, windowH * 0.166};
    repeatPlacement = {windowW * 0.02, windowH * 0.05, windowH * 0.1, windowH * 0.1};

    SDL_Rect buttonRect;
    if (playButtonMode == 0) buttonRect = buttonOff;
    else if (playButtonMode == 1) buttonRect = buttonHover;
    else buttonRect = buttonDown;
    SDL_RenderCopy(myRenderer, play_spritesheet, &buttonRect, &playPlacement);

    if (quitButtonMode == 0) buttonRect = buttonOff;
    else if (quitButtonMode == 1) buttonRect = buttonHover;
    else buttonRect = buttonDown;
	SDL_RenderCopy(myRenderer, quit_spritesheet, &buttonRect, &quitPlacement);

	SDL_Rect sparklePlacement = {windowW * 0.037, windowH * -0.028, windowH * 0.9, windowH * 0.5};
	SDL_RenderCopy(myRenderer, sparkle[(waveFrame / 4) % 6], NULL, &sparklePlacement);

	SDL_Rect logoPlacement = {windowW * 0.08, windowH * 0.03, windowH * 0.7459, windowH * 0.6};
	SDL_RenderCopy(myRenderer, logo, NULL, &logoPlacement);

	SDL_Rect creditsPlacement = {windowW * 0.0125, windowH * 0.95, windowH * 1.1083, windowH * 0.0333};
	SDL_RenderCopy(myRenderer, credits, NULL, &creditsPlacement);

	SDL_RenderCopy(myRenderer, repeat, NULL, &repeatPlacement);
}

SDL_Texture* DededeController::loadTextureFromFile(std::string filePath) {

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
