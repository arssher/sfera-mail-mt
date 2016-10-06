#include "allocator.h"
#include <iostream>

std::ostream& operator<<(std::ostream &strm, const DataBlock &db) {
  return strm << (void *)db.start << "--" << (void *)db.end << ", size "  << db.size;
}

Allocator::Allocator(void *base, size_t size) {
    buffer_start = static_cast<char*>(base);
    buffer_end = buffer_start + size;
    std::cout << "constructing allocator, start address " <<
                 (void*)buffer_start << ", end address " << (void*)buffer_end <<
                 ", size " << buffer_end - buffer_start << "\n";
}

std::list<DataBlock> Allocator::get_free_blocks() const{
    std::list<DataBlock> free_blocks;
    char *free_block_start = buffer_start;
    char *free_block_end = nullptr;
    std::list<std::shared_ptr<DataBlock>>::const_iterator busy_blocks_iterator = busy_blocks.begin();
    while (free_block_end != buffer_end) {
        char *next_iteration_block_start = nullptr;
        // get current free block end
        if (busy_blocks_iterator == busy_blocks.end()) {
            free_block_end = buffer_end;
        } else {
            DataBlock busy_block = **busy_blocks_iterator;
            free_block_end = busy_block.start;
            // update block start for the next iteration
            next_iteration_block_start = busy_block.end;
            busy_blocks_iterator++;
        }
        // now we ready to create block and push it to the result list
        if (free_block_end - free_block_start > 0) {
            std::cout << "found free block " << DataBlock(free_block_start, free_block_end) << "\n";
            free_blocks.push_back(DataBlock(free_block_start, free_block_end));
        }
        free_block_start = next_iteration_block_start;
    }
    return free_blocks;
}


Pointer Allocator::alloc(size_t size) {
    std::list<DataBlock> free_blocks = get_free_blocks();
    DataBlock block_to_allocate_from(nullptr, nullptr);
    for (DataBlock free_db : free_blocks) {
        if (free_db.size >= size &&  // this block is big enough for us
            ((block_to_allocate_from.start == nullptr) || (block_to_allocate_from.size > free_db.size))) {
                block_to_allocate_from = free_db;
            }
    }
    if (block_to_allocate_from.start == nullptr)
        throw AllocError(AllocErrorType::NoMemory, "Out of memory");
    else {
        DataBlock res_bloc = DataBlock(block_to_allocate_from.start, size);
        return Pointer(add_busy_block(res_bloc));
    }
}

std::shared_ptr<DataBlock> Allocator::add_busy_block(DataBlock db) {
    std::cout << "Adding new busy block " << db << "\n";
    auto it = busy_blocks.begin();
    while (it != busy_blocks.end() && (*it)->start <= db.start)
        it++;
    return *busy_blocks.insert(it, std::make_shared<DataBlock>(db));
}

void Allocator::free(Pointer &p) {
    for (auto busy_block_it = busy_blocks.begin(); busy_block_it != busy_blocks.end(); busy_block_it++) {
        if (*busy_block_it == p.dbptr) {
            std::cout << "Removing busy block " << **busy_block_it << "\n";
            busy_blocks.erase(busy_block_it);
            p.dbptr = nullptr;
            return;
        }
    }
    throw AllocError(AllocErrorType::InvalidFree, "Invalid free");
}

void Allocator::realloc(Pointer &p, size_t N) {
    if (p.dbptr == nullptr) {
        p = alloc(N);
        return;
    }
    char *old_block_start = p.dbptr->start;
    size_t old_size = p.dbptr->size;
    free(p); // free the block so get_free_blocks will notice it
    p = alloc(N);
    char *new_block_start = p.dbptr->start;
    // now copy data
    for (size_t i = 0; i < old_size; i++) {
        new_block_start[i] = old_block_start[i];
    }
}

void Allocator::defrag() {
    char *new_busy_block_start = buffer_start;
    for (auto busy_block_ptr : busy_blocks) {
        if (busy_block_ptr->start != new_busy_block_start) { // perhaps we don't need to move anything at all
            size_t block_size = busy_block_ptr->size;
            for (size_t i = 0; i < block_size; i++) {
                new_busy_block_start[i] = busy_block_ptr->start[i];
            }
            std::cout << "moved block " << *busy_block_ptr;
            *busy_block_ptr = DataBlock(new_busy_block_start, block_size);
            std::cout << " to " << *busy_block_ptr << "\n";
            new_busy_block_start += block_size;
        }
    }
}