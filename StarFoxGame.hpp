#ifndef STARFOXGAME_HPP_INCLUDED
#define STARFOXGAME_HPP_INCLUDED

#include "View.hpp"
#include "SDL_mixer.h"

const int numStructures = 100;
const int numElements = 20;
const int numCollect = 5;

class StarFoxGame {
private:
    View* myView;

    const int minX = -7000;
    const int maxX = 7000;
    const int minY = 100;
    const int maxY = 4500;
    const float maxRotate = 0.40; // unit vector component
    const float rotateRate = 0.008;// radians
    bool yawing;
    bool pitching;
    bool rolling;
    float rollVal;
    int SFXVolume;

    Object* arwing;
    Object* shadow;
    int arwingHP;
    //Object* ground[20];
    Object* structures[numStructures];
    Object* markers[8 * 20];
    Object* lasers[50];
    Object* elements[numElements];
    Object* collect[numCollect];
    bool collectTrack[numCollect];
    int nextLaser;
    int laserCooldown;
    int boosterCooldown;
    int invinFrames;
    int scrollRate;

    int frameCount;

    Mix_Chunk* explos;
    Mix_Chunk* laserBlip;

public:
   StarFoxGame(View* inheritedView);
   ~StarFoxGame();

   void update();

   Object* getArwing();
   int getArwingHP();
   void moveLeft();
   void moveRight();
   void moveUp();
   void moveDown();
   void roll(bool clockwise = true);
   void barrelRoll(bool clockwise = true);
   void boosters();
   void shootLaser();
   void setSFXVolume(int v);

};

#endif // STARFOXGAME_HPP_INCLUDED
