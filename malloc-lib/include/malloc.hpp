#ifndef MALLOC_HPP
#define MALLOC_HPP

extern "C" void* mem_alloc(long numbytes);

extern "C" void mem_free(void *firstbyte);

struct mem_control_block {
    int is_available;
    int size;
};

#endif
