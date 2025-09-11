#pragma once

#include <iostream>

namespace
{
constexpr const char *beginEscape = "\x1B[";
constexpr const char *endEscape = "\x1B[m";
} // namespace

namespace util
{

template <class... Args>
static void logInfo(Args... args)
{
    std::cout << beginEscape << 37 << "m" << "[INFO] ";
    (std::cout << ... << args);
    std::cout << endEscape << "\n";
}

template <class... Args>
static void logWarn(Args... args)
{
    std::cout << beginEscape << 33 << "m" << "[WARN] ";
    (std::cout << ... << args);
    std::cout << endEscape << "\n";
}

template <class... Args>
static void logError(Args... args)
{
    std::cout << beginEscape << 31 << "m" << "[ERROR] ";
    (std::cout << ... << args);
    std::cout << endEscape << "\n";
}

} // namespace util