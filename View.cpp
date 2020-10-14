#include "View.hpp"

// returns the poiter to an SDL_Point on the heap, caller is responsable for memory
// converts 3d point to 2d point based on camera
// POINT AT (0, 0, x) gets negative values but it dont want to touch the function atm!
SDL_Point* View::perspectiveTransform(camera c, p3d p, p3d anch) {

    // adjust point relative to normal camera
    // nullify camera pos/angle
    p3d relativeP = p;
    //set to world anchor
    relativeP.x += anch.x;
    relativeP.y += anch.y;
    relativeP.z += anch.z;
    // set to camera pos

    //**Add focallength in camera direction to make rotations relative to the viewplane

    relativeP.x -= c.x;
    relativeP.y -= c.y;
    relativeP.z -= c.z;

    //abort points out of view (sqaure to reduce lag)
    float totalDist = sqrt(pow(relativeP.x, 2) + pow(relativeP.y, 2) + pow(relativeP.z, 2));
    if (totalDist > c.maxSight || totalDist < c.minSight) {
        return NULL;
    }

    //set to camera angle
    // do pitch before yaw? pitch is multi planar, but transforming before yaw
    //      reduces it to one plane
    // do yaw first because it is relative to the starting vector?

    //v = [ cos(y)*cos(p), sin(y), cos(y)*sin(p) ]
    //std::cout << "Point at: (" << relativeP.x << ", " << relativeP.y << ", " << relativeP.z << ")" << std::endl;

    //***   YAW: along x/z
    float xzDist = sqrt(pow(relativeP.x, 2) + pow(relativeP.z, 2));
    float xzAngle;
    if (xzDist == 0) xzAngle = 0;
    else xzAngle = asin((float)relativeP.x / xzDist);
    // CAST RULE
    if (relativeP.z < 0) {
            xzAngle = M_PI - xzAngle;
    }
    xzAngle -= c.yaw;

    if (xzAngle >= (2 * M_PI)) xzAngle -= (2 * M_PI);
    if (xzAngle < 0) xzAngle += (2 * M_PI);

    relativeP.x = sin(xzAngle) * xzDist;
    relativeP.z = cos(xzAngle) * xzDist;


    //***   PITCH along y/(x+z)
    float zyDist = sqrt(pow(relativeP.z, 2) + pow(relativeP.y, 2));
    float zyAngle;
    if (zyDist == 0) zyAngle = 0;
    else zyAngle = asin((float)relativeP.z / zyDist);
    // CAST RULE
    if (relativeP.y < 0) {
            zyAngle = M_PI - zyAngle;
    }
    zyAngle -= c.pitch;

    if (zyAngle >= (2 * M_PI)) zyAngle -= (2 * M_PI);
    if (zyAngle < 0) zyAngle += (2 * M_PI);

    relativeP.z = sin(zyAngle) * zyDist;
    relativeP.y = cos(zyAngle) * zyDist;


    //***   ROLL along y/(x+z)
    float yxDist = sqrt(pow(relativeP.y, 2) + pow(relativeP.x, 2));
    float yxAngle;
    if (yxDist == 0) yxAngle = 0;
    else yxAngle = asin((float)relativeP.y / yxDist);
    // CAST RULE
    if (relativeP.x < 0) {
            yxAngle = M_PI - yxAngle;
    }
    yxAngle -= c.roll;

    if (yxAngle >= (2 * M_PI)) yxAngle -= (2 * M_PI);
    if (yxAngle < 0) yxAngle += (2 * M_PI);

    relativeP.y = sin(yxAngle) * yxDist;
    relativeP.x = cos(yxAngle) * yxDist;

    //std::cout << "Pitch of: " << yAngle << " Yaw of: " << xzAngle << std::endl;
    //if (relativeP.y > 1000) std::cout << "(" << relativeP.x << ", " << relativeP.y << ", " << relativeP.z << ")" << std::endl;;

    //abort points behind camera
    if (relativeP.z <= 1) {
        return NULL;
    }

    // perspective projection RELATIVE TO CAMERA
    int projX = ((float)c.focalLength/(float)relativeP.z) * relativeP.x;
    int projY = ((float)c.focalLength/(float)relativeP.z) * relativeP.y;
    //std::cout << "Projected at: (" << projX << ", " << projY << ")" << std::endl;
    SDL_Point* newP = new SDL_Point;
    newP->x = projX;
    newP->y = projY;

    // convert to point on screen
    //newP->x = SCREEN_WIDTH /2 + newP->x;
    //newP->y = SCREEN_HEIGHT/2 - newP->y;

    newP->x = SCREEN_WIDTH /2 + ( (SCREEN_WIDTH /2) * ((float)newP->x / (float)(View_Width /2)) );
    newP->y = SCREEN_HEIGHT/2 - ( (SCREEN_HEIGHT/2) * ((float)newP->y / (float)(View_Height/2)) );
    //if (newP->y > 1000) std::cout << "(" << newP->x << ", " << newP->y << ")" << std::endl;
    return newP;
}

View::View(SDL_Window* inheritedWindow, SDL_Renderer* inheritedRenderer) {

    bool success = true;

	if(SDL_WasInit(SDL_INIT_VIDEO) == 0) {
        std::cout << "SDL video not initialized!" << std::endl;
		success = false;
	}

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
		success = false;
	}

	if (success != false) {
        if (inheritedWindow == NULL) {
            myWindow = SDL_CreateWindow( "3D viewer owned window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        } else {
            myWindow = inheritedWindow;
        }
        SDL_GetWindowSize(myWindow, &SCREEN_WIDTH, &SCREEN_HEIGHT);
        if (myWindow == NULL) {
            std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
            success = false;
        } else {
            //
        }
	}

	if (success != false) {
        if (inheritedRenderer == NULL) {
            myRenderer = SDL_CreateRenderer(myWindow, -1, SDL_RENDERER_ACCELERATED| SDL_RENDERER_PRESENTVSYNC);
        } else {
            myRenderer = inheritedRenderer;
        }
        if (myRenderer == NULL) {
            std::cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
            success = false;
        } else {
            SDL_SetRenderDrawColor( myRenderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
        }
	}

	if (success == false) delete this;

	normalizeCamera();
	View_Height = 2 * 540;
	View_Width = 2 * 960;

	myRenderMode = FILLED;
	bkGround = NULL;
}

View::~View() {
    SDL_DestroyRenderer(myRenderer);
    myRenderer = NULL;
    SDL_DestroyTexture(bkGround);

    for (uint32_t i = 0; i < objectList.size(); i++) {
        delete objectList[i];
    }
}

void View::setBackGround(SDL_Texture* bkg) {
    if (bkGround != NULL) SDL_DestroyTexture(bkGround);
    bkGround = bkg;
}

// sorts polygons from closest to farthest!!!
void View::sortPerspectivePolyList(perspectivePoly pList[], int left, int right) {
    if (left >= right) return;
//std::cout << "Sorting..." << std::endl;
    int i = left;
    int j = right;
    float pivot = pList[(i+j) / 2].polyDist;

    while (left < j || i < right) {
        while (pList[i].polyDist > pivot) i++;
        while (pList[j].polyDist < pivot) j--;

        if (i <= j) {
            perspectivePoly temp = pList[i];
            pList[i] = pList[j];
            pList[j] = temp;
            i++;
            j--;
        } else {
            if (left < j) sortPerspectivePolyList(pList, left, j);
            if (i < right) sortPerspectivePolyList(pList, i, right);
            return;
        }
    }
}

void View::display() {
    SDL_SetRenderDrawColor( myRenderer, 0, 0, 0, 255 );
	SDL_RenderClear( myRenderer );

	if (bkGround != NULL) {
        SDL_RenderCopy(myRenderer, bkGround, NULL, NULL);
	}

	std::vector<perspectivePoly> renderList;

	for (uint32_t i = 0; i < objectList.size(); i++) {
        if (objectList[i] == NULL || objectList[i]->isVisible() == false) continue;
        for (int j = 0; j < objectList[i]->getNumFaces(); j++) {

            polygon poly = objectList[i]->getFaces()[j];
            p3d polyAnchor = objectList[i]->getAnchor();

            //cast v1-3 from poly into p1-3
            SDL_Point* p1 = perspectiveTransform(myCamera, poly.v1, polyAnchor);
            SDL_Point* p2 = perspectiveTransform(myCamera, poly.v2, polyAnchor);
            SDL_Point* p3 = perspectiveTransform(myCamera, poly.v3, polyAnchor);
            //-----------

            // point not valid
            if (p1 == NULL || p2 == NULL || p3 == NULL) {
                delete p1; delete p2; delete p3;
                continue;
            } else if ((p1->x < 0 && p2->x < 0 && p3->x < 0) ||
                       (p1->y < 0 && p2->y < 0 && p3->y < 0)) {
                delete p1; delete p2; delete p3;
                continue;
            } else if ((p1->x > SCREEN_WIDTH && p2->x > SCREEN_WIDTH && p3->x > SCREEN_WIDTH) ||
                       (p1->y > SCREEN_HEIGHT && p2->y > SCREEN_HEIGHT && p3->y > SCREEN_HEIGHT) ) {
                delete p1; delete p2; delete p3;
                continue;
            }
            //std::cout << "poly at (" << p1->x << ", " << p1->y << ") (" << p2->x << ", " << p2->y << ") (" << p3->x << ", " << p3->y << ")" << std::endl;
            // store in vector with distance

            float polyDist = sqrt(pow(poly.center.x + polyAnchor.x - myCamera.x, 2)
                                + pow(poly.center.y + polyAnchor.y - myCamera.y, 2)
                                + pow(poly.center.z + polyAnchor.z - myCamera.z, 2));

            // Better polDist is avg of vertecies, not center?
            /*
            float polyDist = (sqrt(pow(poly.v1.x + polyAnchor.x - myCamera.x, 2) + pow(poly.v1.y + polyAnchor.y - myCamera.y, 2) + pow(poly.v1.z + polyAnchor.z - myCamera.z, 2)) +
                              sqrt(pow(poly.v2.x + polyAnchor.x - myCamera.x, 2) + pow(poly.v2.y + polyAnchor.y - myCamera.y, 2) + pow(poly.v2.z + polyAnchor.z - myCamera.z, 2)) +
                              sqrt(pow(poly.v3.x + polyAnchor.x - myCamera.x, 2) + pow(poly.v3.y + polyAnchor.y - myCamera.y, 2) + pow(poly.v3.z + polyAnchor.z - myCamera.z, 2))) / 3;
            */
            perspectivePoly tri = {p1, p2, p3, polyDist, poly.red, poly.green, poly.blue, poly.alpha};
            renderList.push_back(tri);
        }
	}

    // SORT (goes closest to farthest)
    this->sortPerspectivePolyList(renderList.data(), 0, renderList.size() - 1);


	// Draw from vector (now sorted
	for (uint32_t i = 0; i < renderList.size(); i++) {
        drawPoly(renderList[i].p1->x, renderList[i].p1->y,
                 renderList[i].p2->x, renderList[i].p2->y,
                 renderList[i].p3->x, renderList[i].p3->y,
                 renderList[i].red, renderList[i].green, renderList[i].blue, renderList[i].alpha);
        delete renderList[i].p1;
        delete renderList[i].p2;
        delete renderList[i].p3;
	}

	//update responsability given to game's controller (for HUD purposes)
	//SDL_RenderPresent(myRenderer);
}

void View::addObjectToView(Object* o, bool dupOverride) {
    objectList.push_back(o);
}

bool View::objectExists(std::string ID) {
    for (uint32_t i = 0; i < objectList.size(); i++) {
        if (objectList[i]->getID() == ID) return true;
    }
    return false;
}

void View::deleteObjectFromView(std::string ID) {
// Data with first matching ID is erased
    for (uint32_t i = 0; i < objectList.size(); i++) {
        if (objectList[i] != NULL && objectList[i]->getID() == ID) {
            delete objectList[i];
            objectList[i] = NULL;
            objectList.erase(objectList.begin() + i);

            break;
        }
    }
}

void View::deleteAllObjectsFromView() {
    for (uint32_t i = objectList.size(); i > 0; i--) {
        delete objectList[i-1];
        objectList[i-1] = NULL;
    }
    objectList.resize(0);
    // may still leak memory?
    //std::cout << "Now the view has " << objectList.size() << " objects" << std::endl;
}

struct line {
bool v;
float m;
float b;
};

void View::drawPoly(int x1, int y1, int x2, int y2, int x3, int y3,
                    Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    if (myRenderMode == NO_RENDER) return;
    if (myRenderMode == FILLED || myRenderMode == FILLED_WIREFRAME){

    // QUICK F1X : Cuts Corners to Remove line intersections
    int minY = std::min(y1, std::min(y2, y3)) - 1;
    int maxY = std::max(y1, std::max(y2, y3)) + 1;
    if (maxY < minY) return;

    int minX = std::min(x1, std::min(x2, x3));
    int maxX = std::max(x1, std::max(x2, x3));

    //if (maxY - minY > 500 || maxX - minX > 500) std::cout << "drawing in bounds: " << minX << ", " << minY << " -> " << maxX << ", " << maxY << std::endl;

    line f1 = {x1 == x2, 0, (float)x1};
    if (f1.v == 0) {
        f1.m = ((float)(y1 - y2)) / ((float)(x1 - x2));
        f1.b = y1 - (f1.m * x1);
    }

    line f2 = {x2 == x3, 0, (float)x2};
    if (f2.v == 0) {
        f2.m = ((float)(y2 - y3)) / ((float)(x2 - x3));
        f2.b = y2 - (f2.m * x2);
    }

    line f3 = {x3 == x1, 0, (float)x3};
    if (f3.v == 0) {
        f3.m = ((float)(y3 - y1)) / ((float)(x3 - x1));
        f3.b = y3 - (f3.m * x3);
    }

    if (minX < 0) minX = 0;
    if (maxX > SCREEN_WIDTH) maxX = SCREEN_WIDTH;
    // cant handle this much nested looping in time?
    // yes. try building SDL_Point list and calling drawlineS once.

    SDL_Point pList[(maxX - minX) * 2];

    for (int i = minX; i < maxX; i++) {
        int a = (f1.b + (f1.m * i));
        if (a < 0) a = 0;
        else if (a > SCREEN_HEIGHT) a = SCREEN_HEIGHT;

        int b = (f2.b + (f2.m * i));
        if (b < 0) b = 0;
        else if (b > SCREEN_HEIGHT) b = SCREEN_HEIGHT;

        int c = (f3.b + (f3.m * i));
        if (c < 0) c = 0;
        else if (c > SCREEN_HEIGHT) c = SCREEN_HEIGHT;
        // if vertical line a/b/c = x intercept + (0x)
        // otherwise it is the true y value;

        //all vertical lines
        if (f1.v != 0 && f2.v != 0 && f3.v != 0) {
            pList[(i-minX) * 2] = {i, minY};
            pList[(i-minX) * 2 + 1] = {i, maxY};
            //SDL_RenderDrawLine(myRenderer, i, minY, i, maxY);
        // individual vertical lines
        } else if (f1.v != 0) {
            pList[(i-minX) * 2] = {i, b};
            pList[(i-minX) * 2 + 1] = {i, c};
            //SDL_RenderDrawLine(myRenderer, i, b, i, c);
        } else if (f2.v != 0) {
            pList[(i-minX) * 2] = {i, a};
            pList[(i-minX) * 2 + 1] = {i, c};
            //SDL_RenderDrawLine(myRenderer, i, a, i, c);
        } else if (f3.v != 0) {
            pList[(i-minX) * 2] = {i, a};
            pList[(i-minX) * 2 + 1] = {i, b};
            //SDL_RenderDrawLine(myRenderer, i, a, i, b);
        } else {
                   if (a < minY || a > maxY) {
                pList[(i-minX) * 2] = {i, b};
                pList[(i-minX) * 2 + 1] = {i, c};
                //SDL_RenderDrawLine(myRenderer, i, b, i, c);
            } else if (b < minY || b > maxY) {
                pList[(i-minX) * 2] = {i, c};
                pList[(i-minX) * 2 + 1] = {i,a};
                //SDL_RenderDrawLine(myRenderer, i, c, i, a);
            } else if (c < minY || c > maxY) {
                pList[(i-minX) * 2] = {i, a};
                pList[(i-minX) * 2 + 1] = {i, b};
                //SDL_RenderDrawLine(myRenderer, i, a, i, b);
            } else {
                // obtuse tri, all function intercepts are within bounds, ambiguous
                //always use middle value
                //use value of line with slope closer to middle value's line
                int midV = std::max( std::min(a, b), std::max( std::min(b, c), std::min(a, c)));
                //std::cout << a << ", " << b << ", " << c << " select: " << midV << std::endl;

                if (midV == a) {
                    if (std::abs(f1.m - f2.m) < std::abs(f1.m - f3.m)) {
                        pList[(i-minX) * 2] = {i, midV};
                        pList[(i-minX) * 2 + 1] = {i, b};
                        //SDL_RenderDrawLine(myRenderer, i, midV, i, b);
                    } else {
                        pList[(i-minX) * 2] = {i, midV};
                        pList[(i-minX) * 2 + 1] = {i, c};
                        //SDL_RenderDrawLine(myRenderer, i, midV, i, c);
                    }
                } else if (midV == b) {
                    if (std::abs(f2.m - f3.m) < std::abs(f2.m - f1.m)) {
                        pList[(i-minX) * 2] = {i, midV};
                        pList[(i-minX) * 2 + 1] = {i, c};
                        //SDL_RenderDrawLine(myRenderer, i, midV, i, c);
                    } else {
                        pList[(i-minX) * 2] = {i, midV};
                        pList[(i-minX) * 2 + 1] = {i, a};
                        //SDL_RenderDrawLine(myRenderer, i, midV, i, a);
                    }

                } else if (midV == c) {
                    if (std::abs(f3.m - f1.m) < std::abs(f3.m - f2.m)) {
                        pList[(i-minX) * 2] = {i, midV};
                        pList[(i-minX) * 2 + 1] = {i, a};
                        //SDL_RenderDrawLine(myRenderer, i, midV, i, a);
                    } else {
                        pList[(i-minX) * 2] = {i, midV};
                        pList[(i-minX) * 2 + 1] = {i, b};
                        //SDL_RenderDrawLine(myRenderer, i, midV, i, b);
                    }

                } else std::cout << "HOW??" << std::endl;
            }
        }

    }

    SDL_SetRenderDrawColor(myRenderer, r, g, b, a);
    SDL_RenderDrawLines(myRenderer, pList, ((maxX - minX) * 2));

    }

    if (myRenderMode == FILLED_WIREFRAME) {
        // correct tri frame
        SDL_SetRenderDrawColor(myRenderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(myRenderer, x1, y1, x2, y2);
        SDL_RenderDrawLine(myRenderer, x2, y2, x3, y3);
        SDL_RenderDrawLine(myRenderer, x3, y3, x1, y1);
    } else if (myRenderMode == WIREFRAME) {
        // correct tri frame
        SDL_SetRenderDrawColor(myRenderer, r, g, b, a);
        SDL_RenderDrawLine(myRenderer, x1, y1, x2, y2);
        SDL_RenderDrawLine(myRenderer, x2, y2, x3, y3);
        SDL_RenderDrawLine(myRenderer, x3, y3, x1, y1);
    }
}

camera View::getCamera() {
    return myCamera;
}

void View::moveCameraTo(int xNew, int yNew, int zNew) {
    myCamera.x = xNew;
    myCamera.y = yNew;
    myCamera.z = zNew;
}


void View::moveCameraBy(int xShft, int yShft, int zShft) {
    myCamera.x += xShft;
    myCamera.y += yShft;
    myCamera.z += zShft;
}

void View::moveCameraByRelative(int right, int up, int foreward) {
    // ratio of vector in respective direction vs plane vector
    // does not account for roll

    float xratio = cos(myCamera.yaw);
    float zratio = sin(myCamera.yaw);
    float yratio = cos(myCamera.pitch);
    float xzratio = sin(myCamera.pitch);

    myCamera.x += foreward * zratio  + (right * xratio) + (up * xzratio * zratio);
    myCamera.y += -(foreward * xzratio) + (up    * yratio);
    myCamera.z += foreward * xratio  - (right * zratio) + (up * xzratio * xratio);
}

void View::turnCameraBy(float pitchRads, float yawRads, float rollRads) {
    myCamera.pitch += pitchRads;
    while (myCamera.pitch >= (2 * M_PI)) myCamera.pitch -= (2 * M_PI);
    while (myCamera.pitch < 0) myCamera.pitch += (2 * M_PI);
    //camera should be able to turn upside down

    myCamera.yaw   += yawRads;
    while (myCamera.yaw   >= (2 * M_PI)) myCamera.yaw -= (2 * M_PI);
    while (myCamera.yaw < 0) myCamera.yaw += (2 * M_PI);
    //std::cout << "Yaw: " << myCamera.yaw << std::endl;
    myCamera.roll  += rollRads;
    while (myCamera.roll  >= (2 * M_PI)) myCamera.roll  -= (2 * M_PI);
    while (myCamera.roll < 0) myCamera.roll += (2 * M_PI);

    // unit vector -> radius of 1
    // therefore circumference is 2pi
    // amount of turn is turnRads/2pi

    //compose unit vector based on angles
    // cheat and use formulas
    // does not account for roll
    myCamera.sightVec_z = cos(myCamera.yaw) * cos(myCamera.pitch);
    myCamera.sightVec_x = sin(myCamera.yaw) * cos(myCamera.pitch);
    myCamera.sightVec_y = - sin(myCamera.pitch);

}

void View::turnCameraTo(float pitchRads, float yawRads, float rollRads) {
    myCamera.pitch = pitchRads;
    while (myCamera.pitch >= (2 * M_PI)) myCamera.pitch -= (2 * M_PI);
    while (myCamera.pitch < 0) myCamera.pitch += (2 * M_PI);
    //camera should be able to turn upside down

    myCamera.yaw   = yawRads;
    while (myCamera.yaw   >= (2 * M_PI)) myCamera.yaw -= (2 * M_PI);
    while (myCamera.yaw < 0) myCamera.yaw += (2 * M_PI);
    myCamera.roll  = rollRads;
    while (myCamera.roll  >= (2 * M_PI)) myCamera.roll  -= (2 * M_PI);
    while (myCamera.roll < 0) myCamera.roll += (2 * M_PI);

    myCamera.sightVec_z = cos(myCamera.yaw) * cos(myCamera.pitch);
    myCamera.sightVec_x = sin(myCamera.yaw) * cos(myCamera.pitch);
    myCamera.sightVec_y = - sin(myCamera.pitch);

}

void View::changeFOV(int fovShft) {
    //std::cout << "Focal length: " << myCamera.focalLength << std::endl;
    if (myCamera.focalLength + fovShft < 300 ||
        myCamera.focalLength + fovShft > 3000) return;
    myCamera.focalLength += fovShft;
    //myCamera.z -= fovShft;
    myCamera.maxSight += fovShft;

    float viewRatio = (float)View_Width/(float)View_Height;
    View_Height -= fovShft;
    View_Width -= fovShft * viewRatio;
}

void View::normalizeCamera() {
    myCamera = {0, 0, -1000, 2000, 500000, 50, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0};
    View_Height = 2 * 540;
	View_Width = 2 * 960;
}

void View::setCameraBehind(Object* o, bool lockCamera, float fracMove) {
    if (lockCamera == true) {
        myCamera.roll = -(o->getRoll());
        myCamera.pitch = -(o->getPitch());
        myCamera.yaw = -(o->getYaw());
    }

    myCamera.sightVec_z = cos(myCamera.yaw) * cos(myCamera.pitch);
    myCamera.sightVec_x = sin(myCamera.yaw) * cos(myCamera.pitch);
    myCamera.sightVec_y = - sin(myCamera.pitch);

    if (fracMove > 1) fracMove = 1;
    if (fracMove <= 0) fracMove = 0.1;

    myCamera.x += ((o->getAnchor().x - (1500 * o->getDirVec_x())) - myCamera.x) * fracMove;
    myCamera.y += ((o->getAnchor().y - (1500 * o->getDirVec_y())) - myCamera.y) * fracMove;
    myCamera.z += ((o->getAnchor().z - (1500 * o->getDirVec_z())) - myCamera.z) * fracMove;
}

void View::turnCameraToward(Object* o) {

    float Dx = o->getAnchor().x - myCamera.x;
    float Dy = o->getAnchor().y - myCamera.y;
    float Dz = o->getAnchor().z - myCamera.z;
    //float Dx = myCamera.x - o->getAnchor().x;
    //float Dy = myCamera.y - o->getAnchor().y;
    //float Dz = myCamera.z - o->getAnchor().z;
    float xzDist = sqrt( pow(Dx, 2) + pow( Dz, 2));

    float pitch;
    if (xzDist == 0) pitch = M_PI / 2;
    else pitch = -atan(Dy / xzDist);
    //if (Dy < 0) pitch = 2 * M_PI - pitch;;

    float yaw;
    if (xzDist == 0) yaw = 0;
    else yaw = asin(Dx / xzDist);
    if (Dz < 0) yaw = M_PI - yaw;

    this->turnCameraTo( pitch, yaw, 0);

    myCamera.sightVec_z = cos(myCamera.yaw) * cos(myCamera.pitch);
    myCamera.sightVec_x = sin(myCamera.yaw) * cos(myCamera.pitch);
    myCamera.sightVec_y = - sin(myCamera.pitch);
}

void View::setRenderMode(renderMode r) {
    myRenderMode = r;
}

renderMode View::getRenderMode() {
    return myRenderMode;
}

void View::setRenderDist(int d) {
    myCamera.maxSight = d;
}

void View::resizeScreen() {
    SDL_GetWindowSize(myWindow, &SCREEN_WIDTH, &SCREEN_HEIGHT);
}
