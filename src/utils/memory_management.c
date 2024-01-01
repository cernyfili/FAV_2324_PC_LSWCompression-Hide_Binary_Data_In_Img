//
// Author: Lenovo
// Date: 02.12.2023
// Description: 
//

#include "memory_management.h"
#include "logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Structure to store allocation information
typedef struct {
    void* pointer;
    size_t size;
    const char* file;
    int line;
    bool is_deallocated;
} AllocationInfo;

// Array to store allocation information
AllocationInfo allocations[1000];  // Adjust the size based on your needs
int allocationCount = 0;

// Wrapper function for malloc
void* tracked_malloc(size_t size, const char* file, int line) {
    void* ptr = malloc(size);

    if (ptr != NULL) {
        // Record allocation information
        AllocationInfo info = {ptr, size, file, line, false};
        allocations[allocationCount++] = info;
    }

    return ptr;
}

// Wrapper function for free
void tracked_free(void* ptr, const char* file, int line) {
    // Find the allocation information for the given pointer
    for (int i = 0; i < allocationCount; ++i) {
        if (allocations[i].pointer == ptr) {
            allocations[i].is_deallocated = true;
            free(ptr);
            // Record deallocation information
            log_message(INFO, allocations[i].file, allocations[i].line, "Freeing memory allocated in %s at line %d\n", allocations[i].file, allocations[i].line);
            return;
        }
    }

    // If the pointer is not found, it was not allocated by tracked_malloc
    LOG_MESSAGE(ERROR, "Attempted to free untracked memory in %s at line %d\n");
}