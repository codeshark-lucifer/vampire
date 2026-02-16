#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#define DEBUG_BREAK() __debugbreak()
#define EXPORT_FN __declspec(dllexport)
#elif __linux__
#define DEBUG_BREAK() __builtin_debugtrap()
#define EXPORT_FN
#elif __APPLE__
#define DEBUG_BREAK() __builtin_trap()
#define EXPORT_FN
#endif

enum TextColor
{
    TEXT_COLOR_BLACK,
    TEXT_COLOR_RED,
    TEXT_COLOR_GREEN,
    TEXT_COLOR_BLUE,
    TEXT_COLOR_MAGENTA,
    TEXT_COLOR_CYAN,
    TEXT_COLOR_WHITE,
    TEXT_COLOR_BRIGHT_BLACK,
    TEXT_COLOR_BRIGHT_RED,
    TEXT_COLOR_BRIGHT_GREEN,
    TEXT_COLOR_BRIGHT_YELLOW,
    TEXT_COLOR_BRIGHT_BLUE,
    TEXT_COLOR_BRIGHT_MAGENTA,
    TEXT_COLOR_BRIGHT_CYAN,
    TEXT_COLOR_BRIGHT_WHITE,
    TEXT_COLOR_COUNT,
};

static const char *TextColorTable[TEXT_COLOR_COUNT] = {
    "\x1b[30m",
    "\x1b[31m",
    "\x1b[32m",
    "\x1b[34m",
    "\x1b[35m",
    "\x1b[36m",
    "\x1b[37m",
    "\x1b[90m",
    "\x1b[91m",
    "\x1b[92m",
    "\x1b[93m",
    "\x1b[94m",
    "\x1b[95m",
    "\x1b[96m",
    "\x1b[97m"};

template <typename... Args>
void _log(const char *level, TextColor color, const char *file, int line, const char *fmt, Args... args)
{
    char message[4096];
    snprintf(message, sizeof(message), fmt, args...);

    // Wrap file in quotes to handle spaces in paths
    printf("%s%s: \"%s\"(%d): %s\x1b[0m\n",
           TextColorTable[color], level, file, line, message);
}

#define LOG_DEBUG(fmt, ...) \
    _log("LOG: ", TEXT_COLOR_GREEN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) \
    _log("WARN:  ", TEXT_COLOR_BRIGHT_YELLOW, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) \
    _log("ERROR: ", TEXT_COLOR_RED, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define LOG_ASSERT(x, msg, ...)            \
    do                                     \
    {                                      \
        if (!(x))                          \
        {                                  \
            LOG_ERROR(msg, ##__VA_ARGS__); \
            DEBUG_BREAK();                 \
        }                                  \
    } while (0)
