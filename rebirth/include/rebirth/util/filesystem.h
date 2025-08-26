#pragma once

#include <filesystem>
#include <vector>

namespace rebirth::util
{
std::filesystem::path getExecutablePath();
void setCurrentPath(std::filesystem::path path);
std::vector<char> readFile(std::filesystem::path path);
} // namespace rebirth::util