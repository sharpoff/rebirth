#pragma once

#include <stdarg.h>

#ifndef NDEBUG
#define DEBUG 1
#else
#define DEBUG 0
#endif

namespace logger
{
    void log(const char *fmt, ...);
}

#define LOGI(fmt, ...) if (DEBUG) logger::log("[%s][INFO ] " fmt "\n", __TIME__, __VA_ARGS__)
#define LOGW(fmt, ...) if (DEBUG) logger::log("\033[33m" "[%s][WARN ] " fmt "\033[0m\n", __TIME__, __VA_ARGS__)
#define LOGE(fmt, ...) if (DEBUG) logger::log("\033[31m" "[%s][ERROR] " fmt "\033[0m\n", __TIME__, __VA_ARGS__)