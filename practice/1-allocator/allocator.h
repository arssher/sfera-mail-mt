#include <stdexcept>
#include <string>
#include <list>
#include <iostream>

enum class AllocErrorType {
    InvalidFree,
    NoMemory,
};

class AllocError: std::runtime_error {
private:
    AllocErrorType type;

public:
    AllocError(AllocErrorType _type, std::string message):
            runtime_error(message),
            type(_type)
    {}

    AllocErrorType getType() const { return type; }
};

class DataBlock {
public:
    DataBlock(char *_start, size_t _size) : start(_start), size(_size) {
        end = start + size;
    }
    DataBlock(char *_start, char *_end) : start(_start), end(_end) {
        size = end - start;
    }
    char *start;
    char *end;
    size_t size;
};

class Pointer {
public:
    void *get() const { return 0; } 
};

class Allocator {
public:
    Allocator(void *base, size_t size);
    // Allocate N bytes and return pointer to them.
    // 1) List all free blocks
    // 2) Find the one with minimum size, but big enough for @size
    // 3) Proclaim it. It means, if you store busy blocks, add the block
    //   to the busy ones.
    Pointer alloc(size_t N);
    
    // Realloc memory block starting from @p. What we need here:
    // remove old block from busy block list
    // list of free blocks sorted by the size to choose the suitable one.
    // + we need to add to this list @p block itself and merge it with the others.
    // then alloc new size as usual and move the data there.
    // we can also track should we move the data at all.
    void realloc(Pointer &p, size_t N) {}
    
    // Free memory block starting from @p.
    // If you store busy blocks, just find it and remove.
    void free(Pointer &p) {}

    // iterate over busy blocks and move each to the left
    void defrag() {}
    std::string dump() { return ""; }
private:
    // calculate free blocks
    std::list<DataBlock> get_free_blocks();

    char *buffer_start;
    char *buffer_end;
    // ordered sequentially
    std::list<DataBlock> busy_blocks;
};

