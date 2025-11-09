#include "util/filesystem.h"
#include "core/engine.h"
#include "util/logger.h"

int main()
{
    // set consistent root path
    LOGI("%s", filesystem::getExecutablePath().parent_path().parent_path().c_str());
    filesystem::setCurrentPath(filesystem::getExecutablePath().parent_path().parent_path());

    Engine engine;
    engine.initialize(ApplicationInfo{
        .name = "Application",
        .width = 1280,
        .height = 720,
    });

    engine.run();

    engine.shutdown();
    return 0;
}