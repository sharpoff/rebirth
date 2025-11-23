#include "logger.h"
#include <cstdarg>
#include <stdio.h>

namespace logger
{
    void log(const char *fmt, ...)
    {
        va_list list;
        va_start(list, fmt);
        vfprintf(stdout, fmt, list);
        va_end(list);
    }
} // namespace logger