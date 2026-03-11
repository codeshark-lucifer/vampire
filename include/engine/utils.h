#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
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

#define EXPECT(ERROR, FORMAT, ...)                                                     \
    do                                                                                \
    {                                                                                 \
        if (ERROR)                                                                    \
        {                                                                             \
            fprintf(stderr, "%s -> %s -> %i -> Error(%i):\n\t" FORMAT "\n",           \
                    __FILENAME__, __FUNCTION__, __LINE__, (int)ERROR, ##__VA_ARGS__); \
            raise(SIGABRT);                                                           \
        }                                                                             \
    } while (0)


char* read_file(uint32_t size, const char* path) {
    // Validate parameters
    if (path == NULL || size == 0) {
        fprintf(stderr, "Invalid arguments: path is NULL or size is zero.\n");
        return NULL;
    }

    FILE* file = fopen(path, "rb"); // Open in binary mode
    if (!file) {
        fprintf(stderr, "Error opening file '%s': %s\n", path, strerror(errno));
        return NULL;
    }

    // Allocate buffer (+1 for null terminator)
    char* buffer = (char*)malloc(size + 1);
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed.\n");
        fclose(file);
        return NULL;
    }

    // Read file data
    size_t bytesRead = fread(buffer, 1, size, file);
    if (ferror(file)) {
        fprintf(stderr, "Error reading file '%s'.\n", path);
        free(buffer);
        fclose(file);
        return NULL;
    }

    buffer[bytesRead] = '\0'; // Null-terminate for safety
    fclose(file);
    return buffer;
}

bool write_file(char* data, const char* path) {
    // Validate inputs
    if (data == NULL || path == NULL) {
        return false; // Invalid arguments
    }

    // Open file for writing (overwrite mode)
    FILE* file = fopen(path, "w");
    if (file == NULL) {
        perror("Error opening file");
        return false;
    }

    // Write data to file
    size_t len = strlen(data);
    size_t written = fwrite(data, sizeof(char), len, file);

    // Close file
    if (fclose(file) != 0) {
        perror("Error closing file");
        return false;
    }

    // Check if all data was written
    return (written == len);
}