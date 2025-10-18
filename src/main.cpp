#include "util/filesystem.h"
#include "core/engine.h"

int main()
{
    // set consistent root path
    filesystem::setCurrentPath(filesystem::getExecutablePath().parent_path().parent_path());

    Engine engine;
    engine.initialize();

    engine.run();

    engine.shutdown();
    return 0;
}