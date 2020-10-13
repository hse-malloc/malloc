#include "malloc/malloc.h"

#include <unistd.h>

namespace mymalloc {
    struct mem_control_block {
        int is_available;
        int size;
    };

    static int has_initialized = 0;
    static void *managed_memory_start;
    static void *last_valid_address;

    void malloc_init() {
        last_valid_address = ::sbrk(0);
        managed_memory_start = last_valid_address;
        has_initialized = 1;
    }

    void* mem_alloc(std::size_t size) {
        if (!has_initialized)
            malloc_init();

        size += sizeof(mem_control_block);
        
    
        void *current_location = managed_memory_start;
        void *memory_location = 0;

        while (current_location != last_valid_address) {
            auto current_mcb = reinterpret_cast<mem_control_block*>(current_location);
            if (current_mcb->is_available && current_mcb->size >= size) {
                current_mcb->is_available = 0;
                memory_location = current_location;
            }
            
            current_location += current_mcb->size;
        }

        if (!memory_location) {
            sbrk(size);
            memory_location = last_valid_address;
            last_valid_address += size;
            auto current_mcb = reinterpret_cast<mem_control_block*>(memory_location);
            current_mcb->is_available = 0;
            current_mcb->size = size;
        }

        memory_location += sizeof(mem_control_block);
        
        return memory_location;
    }

    void mem_free(void *ptr) {
        auto mcb = reinterpret_cast<mem_control_block*>(ptr - sizeof(mem_control_block));

        mcb->is_available = 1;
    }
}
