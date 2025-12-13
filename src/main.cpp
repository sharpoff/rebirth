#include <core/filesystem.h>
#include <core/engine.h>
#include <core/logger.h>
#include <math/math.h>

int main()
{
    // set consistent root path
    filesystem::setCurrentPath(filesystem::getExecutablePath().parent_path().parent_path());

    Engine *engine = new Engine("Application", 1280, 720);
    engine->run();

    delete engine;
    return 0;
}