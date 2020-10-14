#ifndef VIEW_HPP_INCLUDED
#define VIEW_HPP_INCLUDED

#include <SDL.h>
#include <SDL_image.h>
//#include <SDL_mixer.h>
#include <iostream>
#include <vector>
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include <map>
#include "Object.hpp"

// NOTE on vectors: right, up, and foreward are posative.
// so screen y value is still inverted. Should be a ratio and easily flippable.
// For implementation requires SDL to init, and constructor passed a window and associated renderer like so:
/*
    //Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
		success = false;
	}

	//texture filtering for later
	if (success != false) {
        if ( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
            std::cout << "Warning: Linear texture filtering not enabled." << std::endl;
        }
	}

	// create window
	if (success != false) {
        globalWindow = SDL_CreateWindow( "Loading...", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (globalWindow == NULL) {
            std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
            success = false;
        }
	}

	//Create vsynced renderer for window
	if (success != false) {
        globalRenderer = SDL_CreateRenderer( globalWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
        if( globalRenderer == NULL ) {
            std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
            success = false;
        }
	}
*/

#ifndef CAMERA_DECLARED
#define CAMERA_DECLARED
struct camera {
int x;
int y;
int z;
int focalLength;
int maxSight;
int minSight;
//direction of sight, components of a unit vector (square sum is 1)
// composed based on diraction angles
float sightVec_x;
float sightVec_y;
float sightVec_z;
// angles are in rads, 0 <= roll < 2pi
// (0, 0, 0) alights with (x, y, z) = (0, 0, 1)
// clockwise is postive
float roll;
float pitch;
float yaw;
};
#endif // CAMERA_DECLARED

#ifndef PERSPECTIVEPOLY_DECLARED
#define PERSPECTIVEPOLY_DECLARED
struct perspectivePoly {
    SDL_Point* p1;
    SDL_Point* p2;
    SDL_Point* p3;
    float polyDist;
    int red;
    int green;
    int blue;
    int alpha;
};
#endif // PERSPECTIVEPOLY_DECLARED

#ifndef RENDERMODE_DECLARED
#define RENDERMODE_DECLARED
enum renderMode {
WIREFRAME,
FILLED,
FILLED_WIREFRAME,
NO_RENDER
};
#endif // RENDERMODE_DECLARED

class View {
private:

    int View_Width;
    int View_Height;

    SDL_Window* myWindow;
    SDL_Renderer* myRenderer;
    SDL_Texture* bkGround;
    std::vector<Object*> objectList;
    camera myCamera;
    renderMode myRenderMode;
    int SCREEN_WIDTH;
    int SCREEN_HEIGHT;

    void sortPerspectivePolyList(perspectivePoly pList[], int left, int right);

public:

    View(SDL_Window* inheritedWindow = NULL, SDL_Renderer* inheritedRenderer = NULL);
    ~View();
    void setBackGround(SDL_Texture* bkg);
    void addObjectToView(Object* o, bool dupOverride = false);
    bool objectExists(std::string ID);
    void deleteObjectFromView(std::string ID);
    void deleteAllObjectsFromView();
    void display();
    void drawPoly(int x1, int y1, int x2, int y2, int x3, int y3,
                  Uint8 r, Uint8 g, Uint8 b, Uint8 a);
    SDL_Point* perspectiveTransform(camera c, p3d p, p3d anch);

    camera getCamera();
    void moveCameraTo(int xNew, int yNew, int zNew);
    void moveCameraBy(int xShft, int yShft, int zShft);
    void moveCameraByRelative(int right, int up, int foreward);
    void turnCameraBy(float pitchRads, float yawRads, float rollRads);
    void turnCameraTo(float pitchRads, float yawRads, float rollRads);
    void changeFOV(int fovShft);
    void normalizeCamera();
    void setCameraBehind(Object* o, bool lockCamera = true, float fracMove = 1);
    void turnCameraToward(Object* o);
    void setRenderMode(renderMode r);
    renderMode getRenderMode();
    void setRenderDist(int d);
    void resizeScreen();

};


#endif // VIEW_HPP_INCLUDED
