#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <cstdint>
#include <csignal>

#include <string.h>
#include <string>
#include <vector>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef char32_t c32;

typedef std::string str;

template <typename T>
using array = std::vector<T>;

#define PI 3.14159265358979323846
// Robust filename extractor
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__))
#ifdef _WIN32
#define DEBUG_BREAK() __builtin_trap() // Or __debugbreak() for MSVC
#else
#define DEBUG_BREAK() raise(SIGTRAP)
#endif

#define PANIC(ERROR, FORMAT, ...)                                                     \
    do                                                                                \
    {                                                                                 \
        if (ERROR)                                                                    \
        {                                                                             \
            fprintf(stderr, "%s -> %s -> %i -> Error(%i):\n\t" FORMAT "\n",           \
                    __FILENAME__, __FUNCTION__, __LINE__, (int)ERROR, ##__VA_ARGS__); \
            raise(SIGABRT);                                                           \
        }                                                                             \
    } while (0)
