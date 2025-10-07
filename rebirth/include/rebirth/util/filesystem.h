#pragma once

#include <filesystem>
#include <EASTL/vector.h>

namespace filesystem
{

std::filesystem::path getExecutablePath();
void setCurrentPath(std::filesystem::path path);
eastl::vector<char> readFile(std::filesystem::path path);

} // namespace util