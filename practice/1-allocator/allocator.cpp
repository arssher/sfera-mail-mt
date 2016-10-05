#include "allocator.h"
#include <iostream>

std::ostream& operator<<(std::ostream &strm, const DataBlock &db) {
  return strm << (void *)db.start << "--" << (void *)db.end << ", size "  << db.size;
}

Allocator::Allocator(void *base, size_t size) {
    buffer_start = reinterpret_cast<char*>(base);
    buffer_end = buffer_start + size;
    std::cout << "constructing allocator, start address %p, end address %p\n" << buffer_start << buffer_end << "\n";
    get_free_blocks();
}

std::list<DataBlock> Allocator::get_free_blocks() {
    std::list<DataBlock> free_blocks;
    char *free_block_start = buffer_start;
    char *free_block_end = nullptr;
    std::list<DataBlock>::const_iterator busy_blocks_iterator = busy_blocks.begin();
    while (free_block_end != buffer_end) {
        // get next block end
        if (busy_blocks_iterator == busy_blocks.end()) {
	    free_block_end = buffer_end;
	} 
	else {
            DataBlock busy_block = *busy_blocks_iterator;
	    free_block_end = busy_block.start;
            busy_blocks_iterator++;
            // update block start for the next iteration
            free_block_start = busy_block.end;
	}
        // now we ready to create block and push it to the result list
        DataBlock free_block(free_block_start, free_block_end);
        std::cout << "found free block " << free_block << "\n";
        if (free_block.end - free_block.start > 0) {
            std::cout << "pushed it\n";
	    free_blocks.push_back(free_block);
	}
    }
    printf("exiting free_block\n");
    return free_blocks;
}

Pointer Allocator::alloc(size_t size) {
    return Pointer();
}
