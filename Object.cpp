#include "Object.hpp"

Object::Object(std::string filePath, p3d location) {
    visible = true;
    dirVec_x = 0.0;
    dirVec_y = 0.0;
    dirVec_z = 1.0;
    roll = 0.0;
    pitch = 0.0;
    yaw = 0.0;
    anchor.x = location.x; anchor.y = location.y; anchor.z = location.z;
    velocityVec_x = 0;
    velocityVec_y = 0;
    velocityVec_z = 0;
    angularVel_p = 0;
    angularVel_y = 0;
    angularVel_r = 0;
    maxDimension = 0;

    std::ifstream objectFile (filePath.c_str());
    objectFile >> this->ID;
    std::cout << "Loading Object '" << this->getID() << "'" << std::endl;
    objectFile >> numVerticies;
    //std::cout << numVerticies << " verticies." << std::endl;
    verticies = new p3d[numVerticies];
    for (int i = 0; i < numVerticies; i++) {
        objectFile >> verticies[i].x;
        objectFile >> verticies[i].y;
        objectFile >> verticies[i].z;
    }

    objectFile >> numFaces;
    //std::cout << numFaces << " faces." << std::endl;
    faces = new polygon[numFaces];
    ogFaces = new polygon[numFaces];
    for (int i = 0; i < numFaces; i++) {
        int p1; int p2; int p3;
        objectFile >> p1;
        objectFile >> p2;
        objectFile >> p3;

        faces[i].v1 = verticies[p1];
        faces[i].v2 = verticies[p2];
        faces[i].v3 = verticies[p3];
        faces[i].center = {(faces[i].v1.x + faces[i].v2.x + faces[i].v3.x)/3,
                           (faces[i].v1.y + faces[i].v2.y + faces[i].v3.y)/3,
                           (faces[i].v1.z + faces[i].v2.z + faces[i].v3.z)/3 };

        objectFile >> faces[i].red;
        objectFile >> faces[i].green;
        objectFile >> faces[i].blue;
        objectFile >> faces[i].alpha;

        //copy
        ogFaces[i].v1 = faces[i].v1;
        ogFaces[i].v2 = faces[i].v2;
        ogFaces[i].v3 = faces[i].v3;
        ogFaces[i].center = faces[i].center;
        ogFaces[i].red = faces[i].red;
        ogFaces[i].green = faces[i].green;
        ogFaces[i].blue = faces[i].blue;
        ogFaces[i].alpha = faces[i].alpha;

        float localMaxDimension = std::max(fabs(ogFaces[i].v1.x), std::max(fabs(ogFaces[i].v1.y), std::max(fabs(ogFaces[i].v1.z),
                                  std::max(fabs(ogFaces[i].v2.x), std::max(fabs(ogFaces[i].v2.y), std::max(fabs(ogFaces[i].v2.z),
                                  std::max(fabs(ogFaces[i].v3.x), std::max(fabs(ogFaces[i].v3.y), fabs(ogFaces[i].v3.z)))))))));

        if (localMaxDimension > maxDimension) maxDimension = localMaxDimension;
    }

    // covers angles
    maxDimension *= 1.42;

    char c;
    objectFile >> c;
    if (c == '$') {}//std::cout << this->getID() << " loaded correctly." << std::endl;
    else std::cout << this->getID() << " failed to load!" << std::endl;
}

Object::~Object() {
    delete[] verticies;
    delete[] faces;
    delete[] ogFaces;
}

p3d* Object::getVerticies() {
    return verticies;
}

polygon* Object::getFaces() {
    return faces;
}

polygon* Object::getOgFaces() {
    return ogFaces;
}

p3d Object::getAnchor() {
    return this->anchor;
}

std::string Object::getID() {
    return ID;
}

void Object::setID(std::string newID) {
    this->ID = newID;
}

int Object::getNumVerticies() {
    return numVerticies;
}

int Object::getNumFaces() {
    return numFaces;
}

bool Object::isVisible() {
    return this->visible;
}

void Object::setVisible(bool on) {
    this->visible = on;
}

float Object::getMaxDimension() {
    return this->maxDimension;
}

float Object::getDirVec_x() {
    return this->dirVec_x;
}

float Object::getDirVec_y() {
    return this->dirVec_y;
}
float Object::getDirVec_z() {
    return this->dirVec_z;
}

float Object::getRoll() {
    return this->roll;
}
float Object::getPitch() {
    return this->pitch;
}

float Object::getYaw() {
    return this->yaw;
}

float Object::getVelocityVec_x() {
    return velocityVec_x;
}

float Object::getVelocityVec_y() {
    return velocityVec_y;
}

float Object::getVelocityVec_z() {
    return velocityVec_z;
}

float Object::getSpd() {
    return sqrt(pow(velocityVec_x, 2) + pow(velocityVec_y, 2) + pow(velocityVec_z, 2));
}

float Object::getAngularVel_p() {
    return angularVel_p;
}

float Object::getAngularVel_y() {
    return angularVel_y;
}

float Object::getAngularVel_r() {
    return angularVel_r;
}

void Object::moveTo(int xNew, int yNew, int zNew) {
    this->anchor.x = xNew;
    this->anchor.y = yNew;
    this->anchor.z = zNew;
}

void Object::moveBy(int xShft, int yShft, int zShft) {
    this->anchor.x += xShft;
    this->anchor.y += yShft;
    this->anchor.z += zShft;
}

// setup vectors / angles to make objects spacially dynamic
void Object::moveByRelative(int right, int up, int foreward) {

    // use roll for right/up
    this->anchor.x += right;
    this->anchor.y += up;

    //foreward
    this->anchor.x += this->dirVec_x * foreward;
    this->anchor.y += this->dirVec_y * foreward;
    this->anchor.z += this->dirVec_z * foreward;
}

void Object::accelerate(float acc, float Xv, float Yv, float Zv) {
    //direction magnitude is disregarded, acc determines
    // moves foreward if no direction given
    if (Xv == 0 && Yv == 0 && Zv == 0) {
        Xv = dirVec_x;
        Yv = dirVec_y;
        Zv = dirVec_z;
    }
    //reset to unit vector
    float vD = sqrt(pow(Xv, 2) + pow(Yv, 2) + pow(Zv, 2));
    velocityVec_x += Xv / vD * acc;
    velocityVec_y += Yv / vD * acc;
    velocityVec_z += Zv / vD * acc;
}

void Object::accelerateAngular(float pRads, float yRads, float rRads) {
    angularVel_p += pRads;
    angularVel_y += yRads;
    angularVel_r += rRads;
}

// slows abs(velocity) regardless of direction, -1 slows completely
void Object::brake(float decc) {
    float vD = sqrt(pow(velocityVec_x, 2) + pow(velocityVec_y, 2) + pow(velocityVec_z, 2));
    if (vD <= decc || decc == -1) {
        velocityVec_x = 0;
        velocityVec_y = 0;
        velocityVec_z = 0;
        return;
    }
    velocityVec_x -= velocityVec_x / vD * decc;
    velocityVec_y -= velocityVec_y / vD * decc;
    velocityVec_z -= velocityVec_z / vD * decc;
}

void Object::brakeAngular(float rads){
    rads = fabs(rads);
    if (fabs(angularVel_p) < rads) angularVel_p = 0;
    else angularVel_p = (fabs(angularVel_p) - rads) * (angularVel_p / fabs(angularVel_p));

    if (fabs(angularVel_y) < rads) angularVel_y = 0;
    else angularVel_y = (fabs(angularVel_y) - rads) * (angularVel_y / fabs(angularVel_y));

    if (fabs(angularVel_r) < rads) angularVel_r = 0;
    else angularVel_r = (fabs(angularVel_r) - rads) * (angularVel_r / fabs(angularVel_r));
}

void Object::advanceFrame() {
    this->moveBy(velocityVec_x, velocityVec_y, velocityVec_z);
    this->rotateBy(angularVel_p, angularVel_y, angularVel_r);
}

p3d rotatePoint(p3d p, float pitchRads, float yawRads, float rollRads) {
    //alter p based on rotation radians and return

    //***   ROLL along y/(x+z)
    float yxDist = sqrt(pow(p.y, 2) + pow(p.x, 2));
    float yxAngle;
    if (yxDist == 0) yxAngle = 0;
    else yxAngle = asin((float)p.y / yxDist);
    // CAST RULE
    if (p.x < 0) {
            yxAngle = M_PI - yxAngle;
    }
    yxAngle -= rollRads;

    if (yxAngle >= (2 * M_PI)) yxAngle -= (2 * M_PI);
    if (yxAngle < 0) yxAngle += (2 * M_PI);

    p.y = sin(yxAngle) * yxDist;
    p.x = cos(yxAngle) * yxDist;

    //***   PITCH along y/(x+z)
    float zyDist = sqrt(pow(p.z, 2) + pow(p.y, 2));
    float zyAngle;
    if (zyDist == 0) zyAngle = 0;
    else zyAngle = asin((float)p.z / zyDist);
    // CAST RULE
    if (p.y < 0) {
            zyAngle = M_PI - zyAngle;
    }
    zyAngle -= pitchRads;

    if (zyAngle >= (2 * M_PI)) zyAngle -= (2 * M_PI);
    if (zyAngle < 0) zyAngle += (2 * M_PI);

    p.z = sin(zyAngle) * zyDist;
    p.y = cos(zyAngle) * zyDist;

    //***   YAW: along x/z
    float xzDist = sqrt(pow(p.x, 2) + pow(p.z, 2));
    float xzAngle;
    if (xzDist == 0) xzAngle = 0;
    else xzAngle = asin((float)p.x / xzDist);
    // CAST RULE
    if (p.z < 0) {
            xzAngle = M_PI - xzAngle;
    }
    xzAngle -= yawRads;

    if (xzAngle >= (2 * M_PI)) xzAngle -= (2 * M_PI);
    if (xzAngle < 0) xzAngle += (2 * M_PI);

    p.x = sin(xzAngle) * xzDist;
    p.z = cos(xzAngle) * xzDist;


    return p;
}

// store turn angles and calc at runtime (more processor work) or
// turn values now, with innacuracies?
//Test for unintended dimension warping@!!
void Object::rotateBy(float pitchRads, float yawRads, float rollRads) {

    //rotatebyrelative: pitch is limited to 180 deg. going past rolls 180.
    //pitch is relative to roll so the pitch direction is continuous
    // eliminates ambiguous case of pitch(180) + roll(180) = pitch (0) + roll(0)


    this->pitch += pitchRads;
    while (this->pitch >= M_PI * 2) this->pitch -= M_PI * 2;
    while (this->pitch <= 0)        this->pitch += M_PI * 2;

    this->yaw -= yawRads;
    while (this->yaw >= M_PI * 2) this->yaw -= M_PI * 2;
    while (this->yaw <= 0)        this->yaw += M_PI * 2;

    this->roll += rollRads;
    while (this->roll >= M_PI * 2) this->roll -= M_PI * 2;
    while (this->roll <= 0)        this->roll += M_PI * 2;

    //roll doesnt change direction vector

    this->dirVec_y = sin(this->pitch); // * 1 for unit vector
    float xzDist = cos(this->pitch);
    this->dirVec_x = sin(-(this->yaw)) * xzDist;
    this->dirVec_z = cos(this->yaw) * xzDist;

    //this->faces[0].v1.x = -300;

    // set faces[i] to ogFaces[i] & rotation;
    for (int i = 0; i < this->getNumFaces(); i++) {
        this->faces[i].v1 = rotatePoint(this->getOgFaces()[i].v1, this->pitch, this->yaw, this->roll);
        this->faces[i].v2 = rotatePoint(this->getOgFaces()[i].v2, this->pitch, this->yaw, this->roll);
        this->faces[i].v3 = rotatePoint(this->getOgFaces()[i].v3, this->pitch, this->yaw, this->roll);
        this->faces[i].center = rotatePoint(this->getOgFaces()[i].center, this->pitch, this->yaw, this->roll);

    }
}

void Object::rotateTo(float pitchRads, float yawRads, float rollRads) {

    //rotatebyrelative: pitch is limited to 180 deg. going past rolls 180.
    //pitch is relative to roll so the pitch direction is continuous
    // eliminates ambiguous case of pitch(180) + roll(180) = pitch (0) + roll(0)


    this->pitch = pitchRads;
    while (this->pitch >= M_PI * 2) this->pitch -= M_PI * 2;
    while (this->pitch <= 0)        this->pitch += M_PI * 2;

    this->yaw = yawRads;
    while (this->yaw >= M_PI * 2) this->yaw -= M_PI * 2;
    while (this->yaw <= 0)        this->yaw += M_PI * 2;

    this->roll = rollRads;
    while (this->roll >= M_PI * 2) this->roll -= M_PI * 2;
    while (this->roll <= 0)        this->roll += M_PI * 2;

    //roll doesnt change direction vector

    this->dirVec_y = sin(this->pitch); // * 1 for unit vector
    float xzDist = cos(this->pitch);
    this->dirVec_x = sin(-(this->yaw)) * xzDist;
    this->dirVec_z = cos(this->yaw) * xzDist;

    //this->faces[0].v1.x = -300;

    // set faces[i] to ogFaces[i] & rotation;
    for (int i = 0; i < this->getNumFaces(); i++) {
        this->faces[i].v1 = rotatePoint(this->getOgFaces()[i].v1, this->pitch, this->yaw, this->roll);
        this->faces[i].v2 = rotatePoint(this->getOgFaces()[i].v2, this->pitch, this->yaw, this->roll);
        this->faces[i].v3 = rotatePoint(this->getOgFaces()[i].v3, this->pitch, this->yaw, this->roll);
        this->faces[i].center = rotatePoint(this->getOgFaces()[i].center, this->pitch, this->yaw, this->roll);

    }
}

bool Object::polyCollide(Object* o) {
    p3d garbage = {0, 0, 0};
    return this->polyCollide(o, garbage);
}

bool Object::polyCollide(Object* o, p3d &normal) {

    //return normal vector of face collided with?

    // coarse full cube check
    // shortcircuits objects far from one another
    // can be further improved by shrinking maxDimension to each dimension
    if (this->getAnchor().x + this->getMaxDimension() < o->getAnchor().x - o->getMaxDimension() ||
        this->getAnchor().x - this->getMaxDimension() > o->getAnchor().x + o->getMaxDimension() ||
        this->getAnchor().y + this->getMaxDimension() < o->getAnchor().y - o->getMaxDimension() ||
        this->getAnchor().y - this->getMaxDimension() > o->getAnchor().y + o->getMaxDimension() ||
        this->getAnchor().z + this->getMaxDimension() < o->getAnchor().z - o->getMaxDimension() ||
        this->getAnchor().z - this->getMaxDimension() > o->getAnchor().z + o->getMaxDimension()) {
        return false;
    }

    // unoptemized full polygon checks
    for (int i = 0; i < this->getNumFaces(); i++) {
        for (int j = 0; j < o->getNumFaces(); j++) {
            // ease of access;
            polygon face1 = o->getFaces()[j];
            face1.v1.x += o->getAnchor().x; face1.v1.y += o->getAnchor().y; face1.v1.z += o->getAnchor().z;
            face1.v2.x += o->getAnchor().x; face1.v2.y += o->getAnchor().y; face1.v2.z += o->getAnchor().z;
            face1.v3.x += o->getAnchor().x; face1.v3.y += o->getAnchor().y; face1.v3.z += o->getAnchor().z;

            polygon face2 = this->getFaces()[i];
            face2.v1.x += this->getAnchor().x; face2.v1.y += this->getAnchor().y; face2.v1.z += this->getAnchor().z;
            face2.v2.x += this->getAnchor().x; face2.v2.y += this->getAnchor().y; face2.v2.z += this->getAnchor().z;
            face2.v3.x += this->getAnchor().x; face2.v3.y += this->getAnchor().y; face2.v3.z += this->getAnchor().z;

            //std::cout << "Points (" << face1.v1.x << ", " << face1.v1.y << ", " << face1.v1.z << "), ";
            //std::cout << "(" << face1.v2.x << ", " << face1.v2.y << ", " << face1.v2.z << "), ";
            //std::cout << "(" << face1.v3.x << ", " << face1.v3.y << ", " << face1.v3.z << ") ";
            // build vectors from face1 polygon verticies (v1 is origin)
            p3d vec1 = {face1.v2.x - face1.v1.x, face1.v2.y - face1.v1.y, face1.v2.z - face1.v1.z};
            p3d vec2 = {face1.v3.x - face1.v1.x, face1.v3.y - face1.v1.y, face1.v3.z - face1.v1.z};

            // cross product makes normal to face1 plane
            p3d norm = {vec1.y * vec2.z - vec1.z * vec2.y,
                        vec1.z * vec2.x - vec1.x * vec2.z,
                        vec1.x * vec2.y - vec1.y * vec2.x};

            // ensure norm points outward
            //    if normal vector is not moving toward colliding face then reverse
            //    only uses x direction (might be nessisary to use all?
            if (norm.x < 0 && (face1.center.x > face2.center.x) ) {
                norm.x *= -1; norm.y *= -1; norm.z *= -1;
            } else if (norm.x > 0 && (face1.center.x < face2.center.x) ) {
                norm.x *= -1; norm.y *= -1; norm.z *= -1;
            }

            //std::cout << " produce normal (" << norm.x << ", " << norm.y << ", " << norm.z << ")" << std::endl;

            float d = (-norm.x * face1.v1.x) + (-norm.y * face1.v1.y) + (-norm.z * face1.v1.z);
            /*
            if (fabs(norm.x * face1.v1.x + norm.y * face1.v1.y + norm.z * face1.v1.z + d) <= 1) {
                std::cout << "point 1 correct, ";
            }
            if (fabs(norm.x * face1.v2.x + norm.y * face1.v2.y + norm.z * face1.v2.z + d) <= 1) {
                std::cout << "point 2 correct, ";
            }
            if (fabs(norm.x * face1.v3.x + norm.y * face1.v3.y + norm.z * face1.v3.z + d) <= 1) {
                std::cout << "point 3 correct." << std::endl;
            } else {
                std::cout << "No points correct. " << std::endl;
            }
            */

            p3d line1_b = face2.v1;
            p3d line1_m = {face2.v2.x - face2.v1.x, face2.v2.y - face2.v1.y, face2.v2.z - face2.v1.z};
            p3d line2_b = face2.v1;
            p3d line2_m = {face2.v3.x - face2.v1.x, face2.v3.y - face2.v1.y, face2.v3.z - face2.v1.z};

            p3d p1 = {0, 0, 0};
            p3d p2 = {0, 0, 0};

            // if line will intersect plane
            if ( ( (norm.x * line1_m.x) + (norm.y * line1_m.y) + ( norm.z * line1_m.z) ) != 0 ) {
                float t1 = ( (-norm.x * line1_b.x) + (-norm.y * line1_b.y) + (-norm.z * line1_b.z) + (-d) )
                           / ((norm.x * line1_m.x) + ( norm.y * line1_m.y) + ( norm.z * line1_m.z) );

                p3d p = {line1_b.x + line1_m.x * t1,
                         line1_b.y + line1_m.y * t1,
                         line1_b.z + line1_m.z * t1};

                //limit by line segment boundaries
                if ( (p.x >= face2.v1.x || p.x >= face2.v2.x) && (p.x <= face2.v1.x || p.x <= face2.v2.x)) {
                    p1 = p;
                }
            }

            if ( ( (norm.x * line2_m.x) + (norm.y * line2_m.y) + ( norm.z * line2_m.z) ) != 0 ) {
                float t2 = ( (-norm.x * line2_b.x) + (-norm.y * line2_b.y) + (-norm.z * line2_b.z) + (-d) )
                           / ((norm.x * line2_m.x) + ( norm.y * line2_m.y) + ( norm.z * line2_m.z) );

                p3d p = {line2_b.x + line2_m.x * t2,
                         line2_b.y + line2_m.y * t2,
                         line2_b.z + line2_m.z * t2};

                //limit by line segment boundaries
                if ( (p.x >= face2.v1.x || p.x >= face2.v3.x) && (p.x <= face2.v1.x || p.x <= face2.v3.x)) {
                    p2 = p;
                }
            }

            // if p1 or p2 are still {0, 0, 0} then there was no intersection
            // ignores care where intersection is ACTUALLY at {0, 0, 0}, negligable
            // max offset from plane was ~0.12

            // if relevant point
            if (p1.x != 0 && p1.y != 0 && p1.z != 0) {
                //std::cout << "Plane offset: " << norm.x * p1.x + norm.y * p1.y + norm.z * p1.z + d << std::endl;

                // rough square check around polygon
                if ( (p1.x <= face1.v1.x || p1.x <= face1.v2.x || p1.x <= face1.v3.x) &&
                     (p1.x >= face1.v1.x || p1.x >= face1.v2.x || p1.x >= face1.v3.x) &&
                     (p1.y <= face1.v1.y || p1.y <= face1.v2.y || p1.y <= face1.v3.y) &&
                     (p1.y >= face1.v1.y || p1.y >= face1.v2.y || p1.y >= face1.v3.y) &&
                     (p1.z <= face1.v1.z || p1.z <= face1.v2.z || p1.z <= face1.v3.z) &&
                     (p1.z >= face1.v1.z || p1.z >= face1.v2.z || p1.z >= face1.v3.z) ) {
                    normal = norm;
                    return true;
                }
            }

            // if relevant point
            if (p2.x != 0 && p2.y != 0 && p2.z != 0) {
                //std::cout << "Plane offset: " << norm.x * p2.x + norm.y * p2.y + norm.z * p2.z + d << std::endl;

                // rough square check around polygon
                if ( (p2.x <= face1.v1.x || p2.x <= face1.v2.x || p2.x <= face1.v3.x) &&
                     (p2.x >= face1.v1.x || p2.x >= face1.v2.x || p2.x >= face1.v3.x) &&
                     (p2.y <= face1.v1.y || p2.y <= face1.v2.y || p2.y <= face1.v3.y) &&
                     (p2.y >= face1.v1.y || p2.y >= face1.v2.y || p2.y >= face1.v3.y) &&
                     (p2.z <= face1.v1.z || p2.z <= face1.v2.z || p2.z <= face1.v3.z) &&
                     (p2.z >= face1.v1.z || p2.z >= face1.v2.z || p2.z >= face1.v3.z) ) {
                    normal = norm;
                    return true;
                }
            }

        }
    }

    return false;
}
