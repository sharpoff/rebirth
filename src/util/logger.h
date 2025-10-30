#pragma once

#include <stdarg.h>


namespace logger
{
    void log(const char *fmt, ...);
}

#define LOGI(fmt, ...) logger::log("[%s][INFO ] " fmt "\n", __TIME__, __VA_ARGS__)
#define LOGW(fmt, ...) logger::log("\033[33m" "[%s][WARN ] " fmt "\033[0m\n", __TIME__, __VA_ARGS__)
#define LOGE(fmt, ...) logger::log("\033[31m" "[%s][ERROR] " fmt "\033[0m\n", __TIME__, __VA_ARGS__)