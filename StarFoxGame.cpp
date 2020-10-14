#include "StarFoxGame.hpp"
// just incase
#include <iostream>
#include <stdlib.h>
#include <string>
#include <math.h>

StarFoxGame::StarFoxGame(View* inheritedView) {

    if (inheritedView == NULL) {
        std::cout <<"StarfoxGame did not recieve view to couple with." << std::endl;
        delete this;
    }
    myView = inheritedView;
    SFXVolume = 0;

    myView->normalizeCamera();
    myView->setRenderDist(40000);
    myView->moveCameraTo(-7000, 1000, 1000);
    //myView->changeFOV(-100);
    myView->deleteAllObjectsFromView();

    for (int i = 0; i < numStructures; i++) structures[i] = NULL;
    for (int i = 0; i < numElements; i++) elements[i] = NULL;
    for (int i = 0; i < numCollect; i++) {
        collect[i] = NULL;
        collectTrack[i] = false;
    }


    scrollRate = 150;
    yawing = false;
    pitching = false;
    rolling = false;
    rollVal = 0;

    //base has no interaction so not included in structures. Will get deleted by deleteAllFromView
    Object* base = new Object("resources/C_base_struct.txt", {0, 0, -20000});
    myView->addObjectToView(base);

    for (int i = 0; i < 22; i++) {
        structures[i] = new Object("resources/building2_struct.txt", {12000 * (i % 2) - 6000, 0, 12000 * (i / 2) + 8000});
        myView->addObjectToView(structures[i]);
    }
    structures[0]->moveBy(1500, 0, 0);
    for (int i = 22; i < 26; i++) {
        structures[i] = new Object("resources/C_arch2_struct.txt", {2000 * ((i - 22) % 2) - 1000, 0, 14000 * (i - 22) + 120000});
        myView->addObjectToView(structures[i]);
    }
    structures[25]->moveBy(0, 0, 35000);
    for (int i = 26; i < 30; i++) {
        structures[i] = new Object("resources/building2_fat_struct.txt", {8000 * ((i - 26) % 2) - 4000, 0, 210000 + ((i-26) / 2 * 40000)});
        myView->addObjectToView(structures[i]);
    }

    structures[30] = new Object("resources/C_arch_g_struct.txt", {0, 0, 220000});
    myView->addObjectToView(structures[30]);
    structures[31] = new Object("resources/dish_base_struct.txt", {3000, 0, 170000});
    myView->addObjectToView(structures[31]);
    structures[32] = new Object("resources/dish_head_struct.txt", {3000, 500, 170000});
    myView->addObjectToView(structures[32]);

    collect[0] = new Object("resources/asteroid_struct.txt", {(2000 * 0) - 1000, 300, 14000 * 0 + 120000});
    myView->addObjectToView(collect[0]);
    collect[0]->setVisible(false);
    collect[1] = new Object("resources/asteroid_struct.txt", {(2000 * 1) - 1000, 300, 14000 * 1 + 120000});
    myView->addObjectToView(collect[1]);
    collect[1]->setVisible(false);
    collect[2] = new Object("resources/asteroid_struct.txt", {(2000 * 0) - 1000, 300, 14000 * 2 + 120000});
    myView->addObjectToView(collect[2]);
    collect[2]->setVisible(false);
    collect[3] = new Object("resources/asteroid_struct.txt", {(2000 * 1) - 1000, 300, 14000 * 3 + 155000});
    myView->addObjectToView(collect[3]);
    collect[3]->setVisible(false);
    collect[4] = new Object("resources/pyramid_struct.txt", {0, 300, 220000});
    collect[4]->accelerateAngular(0, 0, 0.06);
    collect[4]->rotateBy(0, M_PI, 0);
    myView->addObjectToView(collect[4]);
    collect[4]->setVisible(false);

    for (int i = 0; i < 50; i++) {
        lasers[i] = new Object("resources/laser_struct.txt", {-10000, 0, -3000});
        myView->addObjectToView(lasers[i]);
        lasers[i]->setVisible(false);
    }
    nextLaser = 0;

    for (int i = 0; i < 8 * 12; i++) {
        markers[i] = new Object("resources/marker_struct.txt", {(i % 12) * 1500 - 8250, 0, (i / 12) * 7000 - 20000});
        myView->addObjectToView(markers[i]);
    }

    //elements scripting
    {
    elements[0] = new Object("resources/butterfly_ship_struct.txt", {-6000, 1000, 30000});
    myView->addObjectToView(elements[0]);
    elements[0]->setVisible(false);
    elements[0]->accelerate(250, 15, 4, 90);
    elements[0]->setID(elements[0]->getID() + "0");

    elements[1] = new Object("resources/butterfly_ship_struct.txt", {-6000, 5000, 60000});
    myView->addObjectToView(elements[1]);
    elements[1]->setVisible(false);
    elements[1]->accelerate(250, 15, -4, 90);
    elements[1]->setID(elements[1]->getID() + "1");
    }

    arwing = new Object("resources/arwing_struct.txt", {0, 3000, -20000});
    myView->addObjectToView(arwing);
    arwingHP = 5;
    arwing->accelerate(scrollRate, 0, 0, 1);

    shadow = new Object("resources/arwing_struct.txt", {0, 0, -20000});
    myView->addObjectToView(shadow);
    for (int i = 0; i < shadow->getNumFaces(); i++) {
        shadow->getFaces()[i].red = 0;
        shadow->getFaces()[i].green = 0;
        shadow->getFaces()[i].blue = 0;
        shadow->getFaces()[i].alpha = 50;
    }

    explos = Mix_LoadWAV("resources/Explosion+1.wav");
    if (explos == NULL) {
        std::cout << "Failed to load explosion+1! SDL_Mixer error: " << Mix_GetError() << std::endl;
    } else explos->volume = SFXVolume;

    laserBlip = Mix_LoadWAV("resources/falco1e.mp3");
    if (laserBlip == NULL) {
        std::cout << "Failed to load laserBlip! SDL_mixer Error: " << Mix_GetError() << std::endl;
    } else laserBlip->volume = SFXVolume;

    frameCount = 0;
    laserCooldown = 0;
    boosterCooldown = 0;
    invinFrames = 0;
}

StarFoxGame::~StarFoxGame() {
    //view kills objects

    Mix_FreeChunk(explos);
    Mix_FreeChunk(laserBlip);
}


void StarFoxGame::update() {

    //tracking
    //std::cout << "At: " << arwing->getAnchor().z << std::endl;

    //update game
    frameCount++;
    if (frameCount >= 60) frameCount = 0;
    if (laserCooldown > 0) laserCooldown--;

    for (int i = 0; i < 8 * 12; i++) {
        if (markers[i]->getAnchor().z < arwing->getAnchor().z - 20000)
            markers[i]->moveBy(0, 0, 56000);
    }

    for (int i = 0; i < numElements; i++) {
        if (elements[i] == NULL) {
            continue;
        } else if (elements[i]->getAnchor().z <= arwing->getAnchor().z + 2000) {
            elements[i]->setVisible(true);
        }
        if (elements[i]->isVisible() == true) elements[i]->advanceFrame();
    }

    for (int i = 0; i < 50; i++) {
        //lasers[i]->rotateBy(0, 0, 0.2);
        lasers[i]->advanceFrame();
        if (lasers[i]->getAnchor().z > 40000 + arwing->getAnchor().z || lasers[i]->getAnchor().y < minY - 100) {
            lasers[i]->moveTo(0, 0, -3000);
            lasers[i]->brake(1000);
            lasers[i]->setVisible(false);
        }
    }
    // turn satelite dish manually
    structures[32]->rotateBy(0, 0.05, 0);

    if (arwing->getAnchor().z < 15000) {
        arwing->moveBy(0, -10, 0);
        myView->turnCameraToward(arwing);
        if (arwing->getAnchor().z > 10000) {
            scrollRate = 150;
            myView->moveCameraBy(212, 0, 340);
            myView->turnCameraBy(-0.01, 0, 0);
        }
        if (arwing->getAnchor().z > 14800) myView->turnCameraTo(0, 0, 0);
    } else {
        myView->moveCameraTo(arwing->getAnchor().x * 9 / 10, (arwing->getAnchor().y - 2000) * 8 / 10 + 1900, arwing->getAnchor().z - 2200);
    }

    if (arwing->getAngularVel_r() > 0) arwing->accelerateAngular(0, 0, -0.002);
    else if (arwing->getAngularVel_r() < 0) arwing->accelerateAngular(0, 0, 0.002);

    if (boosterCooldown > 120) {
        if (arwing->getVelocityVec_z() < 300) {
            arwing->accelerate(10, 0, 0, 1);
            myView->changeFOV(-20);
        }
    }
    if (boosterCooldown > 0) {
        boosterCooldown--;
    }


    arwing->advanceFrame();

    // COLLISION
    if (invinFrames == 0) {
        for (int i = 0; i < numStructures; i++) {
            if (structures[i] == NULL) continue;
            else if (arwing->polyCollide(structures[i])) {
                arwingHP--;
                Mix_PlayChannel(-1, explos, 0);
                invinFrames = 40;
                for (int i= 0; i < arwing->getNumFaces(); i++) {
                    if (arwing->getFaces()[i].red + 10 <= 255) arwing->getFaces()[i].red += 15;
                    if (arwing->getFaces()[i].green - 10 >= 0) arwing->getFaces()[i].green -= 15;
                    if (arwing->getFaces()[i].blue - 10 >= 0) arwing->getFaces()[i].blue -= 15;
                }
            }
        }
    }
    for (int i = 0; i < numCollect; i++) {
        if (collect[i] != NULL && arwing->polyCollide(collect[i])) collectTrack[i] = true;
        if (i == 4 && collect[4]->isVisible() && arwing->polyCollide(collect[4])) {
            collect[0] = false; collect[1] = false; collect[2] = false; collect[3] = false;
            collect[4]->setVisible(false);
            invinFrames = 40;
        }
    }
    if (collectTrack[0] && collectTrack[1] && collectTrack[2] && collectTrack[3]) collect[4]->setVisible(true);

    collect[4]->advanceFrame();

    if (arwing->getAnchor().x < minX) arwing->moveTo(minX, arwing->getAnchor().y, arwing->getAnchor().z);
    if (arwing->getAnchor().x > maxX) arwing->moveTo(maxX, arwing->getAnchor().y, arwing->getAnchor().z);
    if (arwing->getAnchor().y < minY) arwing->moveTo(arwing->getAnchor().x, minY, arwing->getAnchor().z);
    if (arwing->getAnchor().y > maxY) arwing->moveTo(arwing->getAnchor().x, maxY, arwing->getAnchor().z);

    // update shadow
    for (int i = 0; i < arwing->getNumFaces(); i++) {
        shadow->getFaces()[i].v1 = arwing->getFaces()[i].v1;
        shadow->getFaces()[i].v1.y = 0;
        shadow->getFaces()[i].v2 = arwing->getFaces()[i].v2;
        shadow->getFaces()[i].v2.y = 0;
        shadow->getFaces()[i].v3 = arwing->getFaces()[i].v3;
        shadow->getFaces()[i].v3.y = 0;
    }
    shadow->moveTo(arwing->getAnchor().x, 0, arwing->getAnchor().z);

    //myView->moveCameraTo(arwing->getAnchor().x, arwing->getAnchor().y + 200, -1500);
    //myView->turnCameraToward(arwing);

    if (yawing == false && arwing->getDirVec_x() != 0) {
        if (arwing->getDirVec_x() > 0.011) {
            arwing->rotateBy(0, -rotateRate, -rotateRate * 1.5);
            myView->turnCameraBy(0, 0, rotateRate / 10);
        } else if (arwing->getDirVec_x() < -0.011) {
            arwing->rotateBy(0, rotateRate, rotateRate * 1.5);
            myView->turnCameraBy(0, 0, -rotateRate / 10);
        }
        //else arwing->rotateTo(0, 0, 0);
        if (arwing->getVelocityVec_x() > 0) arwing->accelerate(3, -1, 0, 0);
        else if (arwing->getVelocityVec_x() < 0) arwing->accelerate(3, 1, 0, 0);
    }
    if (pitching == false && arwing->getDirVec_y() != 0) {
        if (arwing->getDirVec_y() > 0.011) arwing->rotateBy(-rotateRate, 0, 0);
        else if (arwing->getDirVec_y() < -0.011) arwing->rotateBy(rotateRate, 0, 0);
        //else arwing->rotateTo(0, 0, 0);
        if (arwing->getVelocityVec_y() > 0) arwing->accelerate(2, 0, -1, 0);
        else if (arwing->getVelocityVec_y() < 0) arwing->accelerate(2, 0, 1, 0);
    }

    // works for rolling, not barrel roll
    if (rolling == false && arwing->getRoll() != 0 && fabs(rollVal) >= 0.02) {
        if (arwing->getRoll() >= M_PI && arwing->getRoll() < M_PI * 2) {
            arwing->rotateBy(0, 0, 0.04);
            rollVal += 0.04;

        }
        if (arwing->getRoll() <= M_PI && arwing->getRoll() > 0) {
            arwing->rotateBy(0, 0, -0.04);
            rollVal -= 0.04;
        }
        if (arwing->getRoll() < 0.01 && arwing->getRoll() > 0 || arwing->getRoll() > M_PI * 2 - 0.01) {
            arwing->rotateTo(arwing->getPitch(), arwing->getYaw(), 0);
            rollVal = 0;
        }
    }

    if (arwing->getVelocityVec_z() > scrollRate) {
        arwing->accelerate(5, 0, 0, -1);
        myView->changeFOV(10);
    }

    for (int i = 0; i < numElements; i++) {
        // shortcircuits if no element allocated
        // exits if element is invisible (sentinel for inactive)
        if (elements[i] == NULL || elements[i]->isVisible() == false) continue;
        for (int j = 0; j < 50; j++) {
            if (lasers[j]->getVelocityVec_z() == 0) continue;
            if (lasers[j]->getAnchor().x > elements[i]->getAnchor().x - 400 &&
                lasers[j]->getAnchor().x < elements[i]->getAnchor().x + 400 &&
                lasers[j]->getAnchor().y > elements[i]->getAnchor().y - 400 &&
                lasers[j]->getAnchor().y < elements[i]->getAnchor().y + 400 &&
                lasers[j]->getAnchor().z > elements[i]->getAnchor().z - 400 &&
                lasers[j]->getAnchor().z < elements[i]->getAnchor().z + 400) {
                // collide happened between i and j
                // delete still requires unique ID's
                Mix_PlayChannel(-1, explos, 0);
                myView->deleteObjectFromView(elements[i]->getID());
                elements[i] = NULL;
                lasers[j]->moveTo(0, 0, -3000);
                lasers[j]->brake(1000);
                lasers[j]->setVisible(false);
                break; // if laser kills element no more checks needed for this element
            }
        }
    }

    // texture flashing
    if (frameCount % 5 == 0) {
        if (arwing->getFaces()[5].green == 0) {
            arwing->getFaces()[5].green = 100;
            arwing->getFaces()[6].green = 100;
        } else {
            arwing->getFaces()[5].green = 0;
            arwing->getFaces()[6].green = 0;
        }
        for (int i = 0; i < 50; i++) {
            if (lasers[i]->isVisible()) {
                if (lasers[i]->getFaces()[0].red == 255) {
                    for (int j = 0; j < 8; j++) {
                        lasers[i]->getFaces()[j].red = 150;
                        lasers[i]->getFaces()[j].green = 150;
                    }
                } else {
                    for (int j = 0; j < 8; j++) {
                        lasers[i]->getFaces()[j].red = 255;
                        lasers[i]->getFaces()[j].green = 255;
                    }
                }
            }
        }
    }
    if (invinFrames > 0) {
        if (invinFrames % 8 == 0) arwing->setVisible(false);
        else arwing->setVisible(true);
        invinFrames--;
    }
    //myView->setCameraBehind(arwing);

    //display
    myView->display();

    //reset sentinals
    yawing = false;
    pitching = false;
    rolling = false;

}

Object* StarFoxGame::getArwing() {
    return arwing;
}

int StarFoxGame::getArwingHP() {
    return arwingHP;
}

void StarFoxGame::moveLeft() {
    yawing = true;
    if ( arwing->getVelocityVec_x() > -60 )
            arwing->accelerate(3, -1, 0, 0);
    if (arwing->getDirVec_x() > -(maxRotate)) {
        arwing->rotateBy(0, -rotateRate, -rotateRate * 1.5);
        myView->turnCameraBy(0, 0, rotateRate / 10);
    }
}

void StarFoxGame::moveRight() {
    yawing = true;
    if ( arwing->getVelocityVec_x() < 60 )
            arwing->accelerate(3, 1, 0, 0);
    if (arwing->getDirVec_x() < maxRotate) {
        arwing->rotateBy(0, rotateRate, rotateRate * 1.5);
        myView->turnCameraBy(0, 0, -rotateRate / 10);
    }
}

void StarFoxGame::moveUp() {
    pitching = true;
    if ( arwing->getVelocityVec_y() < 30 )
            arwing->accelerate(3, 0, 1, 0);
    if (arwing->getDirVec_y() < maxRotate) {
        arwing->rotateBy(rotateRate, 0, 0);
    }
}

void StarFoxGame::moveDown() {
    pitching = true;
    if ( arwing->getVelocityVec_y() > -30 )
            arwing->accelerate(6, 0, -1, 0);
    if (arwing->getDirVec_y() > -(maxRotate)) {
        arwing->rotateBy(-rotateRate, 0, 0);
    }
}

void StarFoxGame::roll(bool clockwise) {
    rolling = true;
    if (clockwise == true && (arwing->getRoll() > M_PI * 3 / 2 || arwing->getRoll() < M_PI / 2)) {
        arwing->rotateBy(0, 0, 0.05);
        rollVal += 0.05;
    } else if (clockwise == false && (arwing->getRoll() > M_PI * 3 / 2 || arwing->getRoll() < M_PI / 2)) {
        arwing->rotateBy(0, 0, -0.05);
        rollVal -= 0.05;
    }
}

void StarFoxGame::barrelRoll(bool clockwise) {
    rolling = true;
    if (clockwise == true) arwing->accelerateAngular(0, 0, 0.01);
    else arwing->accelerateAngular(0, 0, -0.01);
}

void StarFoxGame::boosters() {
    if (boosterCooldown <= 0)
        boosterCooldown = 200;

    return;
}

void StarFoxGame::shootLaser() {
    if (laserCooldown > 0) return;
    Mix_PlayChannel(-1, laserBlip, 0);
    lasers[nextLaser]->moveTo(arwing->getAnchor().x, arwing->getAnchor().y, arwing->getAnchor().z + 100);
    lasers[nextLaser]->accelerate(800, arwing->getDirVec_x(), arwing->getDirVec_y(), arwing->getDirVec_z());
    lasers[nextLaser]->rotateTo( arwing->getPitch(), arwing->getYaw(), 0);
    lasers[nextLaser]->setVisible(true);
    nextLaser++;
    if (nextLaser >= 50) nextLaser = 0;
    laserCooldown = 10;
}

void StarFoxGame::setSFXVolume(int v) {
    SFXVolume = v;
    laserBlip->volume = SFXVolume;
    explos->volume = SFXVolume;
}
