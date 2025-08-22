#pragma once

#include <chrono>

using namespace std::chrono;

namespace rebirth::util
{
struct Timer
{
    inline void start() { time_ = now(); }

    inline double elapsedSeconds() { return (now() - time_).count(); }

    inline double elapsedMiliseconds() { return elapsedSeconds() * 1000; }

    inline high_resolution_clock::time_point time() { return time_; }

    inline high_resolution_clock::time_point now() { return high_resolution_clock::now(); }

private:
    high_resolution_clock::time_point time_;
};
} // namespace rebirth::util