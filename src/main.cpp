#include "core/filesystem.h"
#include "core/engine.h"
#include "core/logger.h"

int main()
{
    // set consistent root path
    LOGI("%s", filesystem::getExecutablePath().parent_path().parent_path().c_str());
    filesystem::setCurrentPath(filesystem::getExecutablePath().parent_path().parent_path());

    Engine *engine = new Engine("Application", 1280, 720);
    engine->run();

    delete engine;
    return 0;
}