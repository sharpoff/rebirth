#include "util/filesystem.h"
#include "core/application.h"

int main()
{
    // set consistent root path
    filesystem::setCurrentPath(filesystem::getExecutablePath().parent_path().parent_path());

    Application app("Application", 1280, 720);
    app.run();

    return 0;
}