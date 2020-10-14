
#include "ScreenController.hpp"

int main(int argc, char* args[]) {

    // I love this kind of block of code.
    // This is the whole program: Make an object, call one method, delete it, return 0.

    ScreenController* globalController = new ScreenController();
    globalController->Start();
    delete globalController;
    return 0;
}
