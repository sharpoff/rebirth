#include <fstream>
#include <rebirth/util/filesystem.h>
#include <rebirth/util/logger.h>

#ifdef __linux__
#include <unistd.h>
#endif

namespace filesystem
{
    std::filesystem::path getExecutablePath()
    {
        // TODO: add windows support
#ifdef __linux__
        const int maxPath = 100;

        char path[maxPath];
        ssize_t count = readlink("/proc/self/exe", path, maxPath);
        if (count > 0)
            return std::filesystem::path(path).parent_path();
#endif
        return "";
    }

    void setCurrentPath(std::filesystem::path path) { std::filesystem::current_path(path); }

    eastl::vector<char> readFile(std::filesystem::path path)
    {
        std::ifstream file(path, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            logger::logError("Failed to open file - ", path);
            return {};
        }

        eastl::vector<char> buffer;
        size_t size = file.tellg();
        buffer.resize(size);
        file.seekg(0);
        file.read(buffer.data(), size);
        file.close();

        return buffer;
    }
} // namespace filesystem