//
// Author: Filip Cerny
// Date: 02.12.2023
// Description: This file contains functions for memory management.
//

//local includes
#include "memory_management.h"
#include "logger.h"
#include "utils.h"

//lib includes
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define ALLOCATION_LIMIT 1000

/**
 * Structure for storing information about allocation.
 */
typedef struct {
    void* pointer;      /**< Pointer to the allocated memory. */
    size_t size;        /**< Size of the allocated memory. */
    const char* file;   /**< File name where the allocation occurred. */
    int line;           /**< Line number where the allocation occurred. */
    bool is_deallocated;/**< Flag indicating whether the memory is deallocated. */
} AllocationInfo;

// Array to store allocation information
AllocationInfo allocations[ALLOCATION_LIMIT];  // Adjust the size based on your needs
int allocations_count = 0;

/**
 * Array of AllocationInfo structures.
 */


/**
 * Allocates memory of the specified size using malloc and tracks the allocation
 * by recording information such as file name, line number, and size.
 *
 * @param size Size of the memory to be allocated.
 * @param file File name where the allocation is made.
 * @param line Line number where the allocation is made.
 * @return A pointer to the allocated memory.
 */
void* tracked_malloc(size_t size, const char* file, int line) {
    if(allocations_count >= ALLOCATION_LIMIT){
        printf("Allocation limit reached\n");
        exit(7);
    }

    void* ptr = malloc(size);

    if (ptr != NULL) {
        // Record allocation information
        AllocationInfo info = {ptr, size, file, line, false};
        allocations[allocations_count++] = info;
    }

    return ptr;
}

/**
 * Frees the memory pointed to by the given pointer using free and tracks the deallocation
 * by recording information such as file name and line number. Also marks the allocation as deallocated.
 *
 * @param ptr Pointer to the memory to be deallocated.
 * @param file File name where the deallocation is made.
 * @param line Line number where the deallocation is made.
 */
void tracked_free(void* ptr, const char* file, int line) {
    // Find the allocation information for the given pointer
    for (int i = 0; i < allocations_count; ++i) {
        if (allocations[i].pointer == ptr && allocations[i].is_deallocated == false) {
            allocations[i].is_deallocated = true;
            free(ptr);
            return;
        }
    }

    // If the pointer is not found, it was not allocated by tracked_malloc
    printf("Attempted to free untracked memory in %s:%d\n", file, line);
    exit(7);
}

/**
 * Frees the memory pointed to by the given pointer using free, sets the pointer to NULL,
 * and tracks the deallocation by recording information such as file name and line number.
 *
 * @param ptr Pointer to the memory to be deallocated and set to NULL.
 * @param file File name where the deallocation is made.
 * @param line Line number where the deallocation is made.
 */
void tracked_free_null(void** ptr, const char* file, int line) {
    if (*ptr != NULL) {
        tracked_free(*ptr, file, line);
        *ptr = NULL;
    }
}

/**
 * Prints a report summarizing the memory allocations and deallocations,
 * helping to identify potential memory leaks.
 */
void memory_management_report(){
    for (int i = 0; i < allocations_count; ++i) {
        if (allocations[i].is_deallocated == false) {
            log_message(WARNING, allocations[i].file, allocations[i].line, "Memory leak detected");
        }
    }
}

//region CLEANUP FUNCTIONS

/**
 * Adds a cleanup command to the list, specifying a pointer to be freed
 * and recording file and line information for cleanup purposes.
 *
 * @param head Pointer to the head of the cleanup command list.
 * @param ptr Pointer to the memory to be freed during cleanup.
 * @param file File name where the cleanup command is added.
 * @param line Line number where the cleanup command is added.
 */
void cleanup_add_command(CleanupCommand** head, void** ptr, char *file, int line){
    CleanupCommand* new_command = malloc(sizeof(CleanupCommand));
    if (!new_command) {
        LOG_MESSAGE(ERROR, "Unable to allocate memory for cleanup command.");
        exit(7);
    }

    new_command->ptr = ptr;

    new_command->file = malloc(CALC_STR_MEM_SIZE(CALC_STR_REAL_LEN(file)));
    if (!new_command->file) {
        LOG_MESSAGE(ERROR, "Unable to allocate memory for cleanup command.");
        exit(7);
    }
    strcpy(new_command->file, file);

    new_command->line = line;
    new_command->next = *head;
    *head = new_command;
}

/**
 * Iterates through the cleanup command list and executes each cleanup command,
 * freeing the associated memory. After execution, the cleanup list is cleared.
 *
 * @param head Pointer to the head of the cleanup command list.
 */
void cleanup_run_commands(CleanupCommand** head) {
    CleanupCommand* current = *head;
    while (current) {
        if (!((current->ptr))) {
            continue;
        }
        // Run the cleanup command
        tracked_free_null(current->ptr, current->file, current->line);
        CleanupCommand* next = current->next;
        free(current->file);
        free(current);

        // Move to the next command
        current = next;
    }
    free(current);
    head = NULL;
}
//endregion