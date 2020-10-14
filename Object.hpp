#ifndef OBJECT_HPP_INCLUDED
#define OBJECT_HPP_INCLUDED

#define _GLIBCXX_ABI_TAG_CXX11_ABI 0

#include <SDL.h>
#include <string>
#include <iostream>
#include <fstream>
#include <math.h>
#include <stdlib.h>

// points are cast into int to be placed on screen,
// float keeps some acuracy for manipulation
struct p3d {
float x;
float y;
float z;
};

struct polygon {
p3d v1;
p3d v2;
p3d v3;
p3d center;
int red;
int green;
int blue;
int alpha;
};

class Object {
private:
    std::string ID;
    p3d anchor;
    int numVerticies;
    int numFaces;
    p3d* verticies; // <- possibly not nessisary any longer
    polygon* faces;
    polygon* ogFaces;
    bool visible;

    // for collision optemisation
    float maxDimension;

    // direction values are relative to global direction constants: +x, +y, +z
    // dirVec is a unit vector
    float dirVec_x;
    float dirVec_y;
    float dirVec_z;
    float roll;
    float pitch;
    float yaw;

    // not a unit vector!! actual movement per frame
    float velocityVec_x;
    float velocityVec_y;
    float velocityVec_z;

    float angularVel_p;
    float angularVel_y;
    float angularVel_r;

public:

    Object(std::string filePath, p3d location);
    ~Object();

    p3d* getVerticies();
    polygon* getFaces();
    polygon* getOgFaces();
    p3d getAnchor();
    std::string getID();
    void setID(std::string newID);
    int getNumVerticies();
    int getNumFaces();
    bool isVisible();
    void setVisible(bool on);
    float getMaxDimension();

    float getDirVec_x();
    float getDirVec_y();
    float getDirVec_z();
    float getRoll();
    float getPitch();
    float getYaw();
    float getVelocityVec_x();
    float getVelocityVec_y();
    float getVelocityVec_z();
    float getSpd();
    float getAngularVel_p();
    float getAngularVel_y();
    float getAngularVel_r();

    void moveTo(int xNew, int yNew, int zNew);
    void moveBy(int xShft, int yShft, int zShft);
    void moveByRelative(int right, int up, int foreward);
    void rotateBy(float pitchRads, float yawRads, float rollRads);
    void rotateTo(float pitchRads, float yawRads, float rollRads);

    void accelerate(float acc, float Xv = 0, float Yv = 0, float Zv = 0);
    void accelerateAngular(float pRads, float yRads, float rRads);
    void brake(float decc);
    void brakeAngular(float rads);
    void advanceFrame();

    bool polyCollide(Object* o);
    bool polyCollide(Object* o, p3d &normal);

    // Rotation about anchor?
};

#endif // OBJECT_HPP_INCLUDED
