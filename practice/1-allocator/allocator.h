#include <stdexcept>
#include <string>
#include <list>
#include <iostream>
#include <memory>

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
    Pointer() : dbptr(nullptr) {}
    Pointer(std::shared_ptr<DataBlock> _dbptr) : dbptr(_dbptr) {}
    void *get() const { return dbptr ? static_cast<void*>(dbptr->start) : nullptr; }
    std::shared_ptr<DataBlock> dbptr;
};

class Allocator {
public:
    Allocator(void *base, size_t size);
    // Allocate N bytes and return pointer to them. It lists all free blocks, finds the one with minimum size,
    // but big enough for @N and adds it to busy_blocks
    Pointer alloc(size_t N);
    
    // Realloc memory block pointed by @p.
    void realloc(Pointer &p, size_t N);
    
    // Free memory block starting from @p.
    // Just removed the block from busy_blocks and resets the pointer
    void free(Pointer &p);

    // iterate over busy blocks and move each to the left side
    void defrag();
    std::string dump() { return ""; }
private:
    // calculate free blocks
    std::list<DataBlock> get_free_blocks() const;
    // insert new busy block to correct place
    std::shared_ptr<DataBlock> add_busy_block(DataBlock db);

    char *buffer_start;
    char *buffer_end;
    // ordered sequentially
    std::list<std::shared_ptr<DataBlock>> busy_blocks;
};

