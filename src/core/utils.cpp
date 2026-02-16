#include <utils.h>
#include <cstdlib>

BumpAllocator MakeAllocator(size_t size)
{
    BumpAllocator alloc{};
    alloc.memory = (char*)malloc(size);
    alloc.capacity = size;
    alloc.used = 0;
    return alloc;
}

inline size_t AlignForward(size_t ptr, size_t align)
{
    size_t mod = ptr & (align - 1);
    if (mod) ptr += (align - mod);
    return ptr;
}

void* BumpAllocAligned(BumpAllocator* alloc, size_t size, size_t align)
{
    size_t current = (size_t)alloc->memory + alloc->used;
    size_t aligned = AlignForward(current, align);
    size_t newUsed = aligned - (size_t)alloc->memory + size;

    if (newUsed > alloc->capacity)
        return nullptr;

    alloc->used = newUsed;
    return (void*)aligned;
}

char* read_file(str filepath, int* filesize)
{
    FILE* f = fopen(filepath, "rb");
    if (!f) return nullptr;

    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    rewind(f);

    char* data = (char*)malloc(size + 1);
    if (!data) {
        fclose(f);
        return nullptr;
    }

    fread(data, 1, size, f);
    data[size] = 0; // null-terminate (safe for text)

    fclose(f);

    if (filesize)
        *filesize = size;

    return data;
}

bool write_file(str filepath, const void* data, int size)
{
    FILE* f = fopen(filepath, "wb");
    if (!f) return false;

    fwrite(data, 1, size, f);
    fclose(f);

    return true;
}

long long get_timestamp(const char* file)
{
    LOG_ASSERT(file, "get_timestemp: file is null");

    struct stat file_stat = {};
    if (stat(file, &file_stat) != 0)
        return 0;

    return file_stat.st_mtime;
}