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

//region PRIVATE VARIABLES
/**
 * Structure for storing information about allocation.
 * defined in main
 * global variable is used here because it helps with code readability
 * (in funtions which use memory management functions) because otherwise
 * we would have to pass it to every function
 */
static struct allocation *allocations_head;

static bool is_initialized = false;
//endregion

//region FUNCTION DECLARATIONS
/**
 *  This function frees the memory pointed to by the given pointer using free and tracks the deallocation
 * for memory management purposes. It records the file and line information of the deallocation.
 *
 * @param allocation_list Pointer to the head of the allocation list.
 * @param ptr Pointer to the memory to be deallocated.
 * @param file File name where the deallocation is made.
 * @param line Line number where the deallocation is made.
 */
static void tracked_free_static(struct allocation **allocation_list, void **ptr, const char *file, int line);

/**
 * Allocates memory of the specified size using malloc and tracks the allocation
 * by recording information such as file name, line number, and size.
 *
 * @param allocation_list Pointer to the head of the allocation list.
 * @param size Size of the memory to be allocated.
 * @param file File name where the allocation is made.
 * @param line Line number where the allocation is made.
 * @return A pointer to the allocated memory.
 */
static void *tracked_malloc_static(struct allocation **allocation_list, size_t size, const char *file, int line);

/**
 * Function for getting singletion head of the allocation list.
 * @return  Pointer to the head of the allocation list.
 */
static struct allocation **get_allocations_head();


/**
 * Frees the memory of allocation info linked list
 * pointed to by the given pointer using free and tracks the deallocation
 * @param allocation_list Pointer to the head of the allocation list.
 */
static void allocation_list_free(struct allocation **allocation_list);

/**
 * Prints a report summarizing the memory allocations and deallocations,
 * helping to identify potential memory leaks.
 * @param allocation_list Pointer to the head of the allocation list.
 */
static void memory_management_report_static(struct allocation **allocation_list);
//endregion

//region FUNCTION DEFINITIONS

//region PRIVATE FUNCTIONS
/**
 * Frees the memory pointed to by the given pointer using free and tracks the deallocation
 * by recording information such as file name and line number. Also marks the allocation as deallocated.
 *
 * @param allocation_list Pointer to the head of the allocation list.
 * @param ptr Pointer to the memory to be deallocated.
 * @param file File name where the deallocation is made.
 * @param line Line number where the deallocation is made.
 */
static void tracked_free_static(struct allocation **allocation_list, void **ptr, const char *file, int line) {
    if (!is_initialized) {
        printf("MEMORY MANAGEMENT(tracked_free_static): Memory management is not initialized%s:%d\n", file, line);
        return;
    }
    if (allocation_list == NULL) {
        printf("MEMORY MANAGEMENT(tracked_free_static): Wrong argument at %s:%d\n", file, line);
        return;
    }
    if ((*allocation_list) == NULL) {
        printf("MEMORY MANAGEMENT(tracked_free_static): Allocation list is empty%s:%d\n", file, line);
        return;
    }
    if (ptr == NULL) {
        printf("MEMORY MANAGEMENT(tracked_free_static): Wrong argument%s:%d\n", file, line);
        return;
    }
    if (*ptr == NULL) {
        printf("MEMORY MANAGEMENT(tracked_free_static): Wrong argument%s:%d\n", file, line);
        return;
    }
        struct allocation *current = *allocation_list;
        do {
            if (current->info.pointer == (*ptr) && current->info.is_deallocated == false) {
                current->info.is_deallocated = true;
                free(*ptr);
                return;
            }
            current = current->next;
        } while (current != NULL);

        *ptr = NULL;


/*
    // If the pointer is not found, it was not allocated by tracked_malloc_static
    printf("MEMORY MANAGEMENT: Attempted to free untracked memory in %s:%d\n", file, line);*/
}

/**
 * Allocates memory of the specified size using malloc and tracks the allocation
 * by recording information such as file name, line number, and size.
 *
 * @param allocation_list Pointer to the head of the allocation list.
 * @param size Size of the memory to be allocated.
 * @param file File name where the allocation is made.
 * @param line Line number where the allocation is made.
 * @return A pointer to the allocated memory.
 */
static void *tracked_malloc_static(struct allocation **allocation_list, size_t size, const char *file, int line) {
    if (!is_initialized) {
        printf("MEMORY MANAGEMENT: Memory management is not initialized\n");;
        return NULL;
    }

    if (allocation_list == NULL) {
        printf("MEMORY MANAGMENT: Wrong argument\n");
        return NULL;
    }

    void *ptr = malloc(size);

    if (ptr != NULL) {
        // Record allocation information
        struct allocationinfo new_info;
        new_info.pointer = ptr;
        new_info.size = size;
        new_info.file = malloc(CALC_STR_MEM_SIZE(CALC_STR_REAL_LEN(file)));
        if (!new_info.file) {
            printf("MEMORY MANAGEMENT: Unable to allocate memory for allocation info.\n");;

            return NULL;
        }
        strcpy((char *) new_info.file, file);
        new_info.line = line;
        new_info.is_deallocated = false;

        if (*allocation_list == NULL) {
            *allocation_list = malloc(sizeof(struct allocation));
            if (!(*allocation_list)) {
                printf("MEMORY MANAGEMENT: Unable to allocate memory for allocation info.\n");;
                return NULL;
            }
            (*allocation_list)->info = new_info;
            (*allocation_list)->next = NULL;

        } else {
            // Add allocationinfo at the end of list
            struct allocation *current = *allocation_list;
            while (current->next != NULL) {
                current = current->next;
            }
            current->next = malloc(sizeof(struct allocation));
            if (!current->next) {
                printf("MEMORY MANAGEMENT: Unable to allocate memory for allocation info.\n");;
                return NULL;
            }
            current->next->info = new_info;
            current->next->next = NULL;
        }
    }

    return ptr;
}

/**
 * Frees the memory of allocation info linked list
 * pointed to by the given pointer using free and tracks the deallocation
 * @param allocation_list Pointer to the head of the allocation list.
 */
static void allocation_list_free(struct allocation **allocation_list) {
    if (!is_initialized) {
        printf("MEMORY MANAGEMENT: Memory management is not initialized\n");;
        return;
    }

    if (allocation_list == NULL) {
        printf("MEMORY MANAGEMENT: Wrong argument\n");;
        return;
    }

    if ((*allocation_list) == NULL) {
        printf("MEMORY MANAGEMENT: Allocation list is empty\n");;
        return;
    }

    struct allocation *current = *allocation_list;
    do {
        free(current->info.file);

        struct allocation *next = current->next;
        free(current);
        current = next;
    } while (current != NULL);
    *allocation_list = NULL;
}

/**
 * Function for getting singletion head of the allocation list.
 * @return  Pointer to the head of the allocation list.
 */
static struct allocation **get_allocations_head() {
    return &allocations_head;
}

/**
 * Prints a report summarizing the memory allocations and deallocations,
 * helping to identify potential memory leaks.
 * @param allocation_list Pointer to the head of the allocation list.
 */
static void memory_management_report_static(struct allocation **allocation_list) {
    if (!is_initialized) {
        printf("MEMORY MANAGEMENT: Memory management is not initialized\n");;
        return;
    }

    if (allocation_list == NULL) {
        printf("MEMORY MANAGEMENT: Wrong argument\n");;
        return;
    }

    if ((*allocation_list) == NULL) {
        printf("MEMORY MANAGEMENT: Allocation list is empty\n");
        return;
    }

    while ((*allocation_list)->next != NULL) {
        if ((*allocation_list)->info.is_deallocated == false) {
            log_message(WARNING, (*allocation_list)->info.file, (*allocation_list)->info.line, "Memory leak detected");
        }
        (*allocation_list) = (*allocation_list)->next;
    }
}

/**
 * Iterates through the cleanup command list and executes each cleanup command,
 * freeing the associated memory. After execution, the cleanup list is cleared.
 *
 * @param head Pointer to the head of the cleanup command list.
 * @param allocation_list Pointer to the head of the allocation list.
 */
static void cleanup_run_commands_static(struct cleanupcommand **cleanup_list, struct allocation **allocation_list) {
    if (!is_initialized) {
        printf("MEMORY MANAGEMENT: Memory management is not initialized\n");;
        return;
    }

    struct cleanupcommand *current = *cleanup_list;
    while (current != NULL) {
        // Run the cleanup command
        tracked_free_static(allocation_list, current->ptr, current->file, current->line);

        struct cleanupcommand *next = current->next;
        free(current->file);
        free(current);

        // Move to the next command
        current = next;
    }
    free(current);
    cleanup_list = NULL;
}
//endregion

/**
 * Initializes the memory management system.
 * @return  true if initialization was successful, false otherwise
 */
void memory_management_init() {
    if (is_initialized) {
        printf("MEMORY MANAGEMENT: Memory management is already initialized\n");;
        return;
    }

    allocations_head = NULL;
    is_initialized = true;
}

/**
 * Destroys the memory management system.
 * @return  true if destruction was successful, false otherwise
 */
void memory_management_destroy() {
    if (!is_initialized) {
        printf("MEMORY MANAGEMENT: Memory management is not initialized\n");;
        return;
    }

    allocation_list_free(&allocations_head);
    is_initialized = false;
}

/**
 * Allocates memory of the specified size using malloc and tracks the allocation
 * by recording information such as file name, line number, and size.
 *
 * @param size Size of the memory to be allocated.
 * @param file File name where the allocation is made.
 * @param line Line number where the allocation is made.
 * @return A pointer to the allocated memory.
 */
void *tracked_malloc(size_t size, const char *file, int line) {
    if (!is_initialized) {
        printf("MEMORY MANAGEMENT: Memory management is not initialized\n");;
        return NULL;
    }
    return tracked_malloc_static(get_allocations_head(), size, file, line);
}

/**
 * Frees the memory pointed to by the given pointer using free, sets the pointer to NULL,
 * and tracks the deallocation by recording information such as file name and line number.
 *
 * @param ptr Pointer to the memory to be deallocated and set to NULL.
 * @param file File name where the deallocation is made.
 * @param line Line number where the deallocation is made.
 */
void tracked_free(void **ptr, const char *file, int line) {
    if (!is_initialized) {
        printf("MEMORY MANAGEMENT: Memory management is not initialized\n");;
        return;
    }

    tracked_free_static(get_allocations_head(), ptr, file, line);
}

/**
 * Prints a report summarizing the memory allocations and deallocations,
 * helping to identify potential memory leaks.
 */
void memory_management_report() {
    if (!is_initialized) {
        printf("MEMORY MANAGEMENT: Memory management is not initialized\n");;
        return;
    }
    memory_management_report_static(get_allocations_head());
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
void cleanup_add_command(struct cleanupcommand **head, void **ptr, char *file, int line) {
    if (!is_initialized) {
        printf("MEMORY MANAGEMENT: Memory management is not initialized\n");;
        return;
    }
    if (!ptr) {
        printf("MEMORY MANAGEMENT: Wrong argument\n");;
        return;
    }

    if (*head == NULL) {
        *head = malloc(sizeof(struct cleanupcommand));
        if (!(*head)) {
            printf("MEMORY MANAGEMENT: Unable to allocate memory for allocation info.\n");;
            return;
        }
        (*head)->ptr = ptr;
        (*head)->file = malloc(CALC_STR_MEM_SIZE(CALC_STR_REAL_LEN(file)));
        if (!(*head)->file) {
            printf("MEMORY MANAGEMENT: Unable to allocate memory for allocation info.\n");;
            return;
        }
        strcpy((*head)->file, file);
        (*head)->line = line;
        (*head)->next = NULL;
    } else {
        // Add allocationinfo at the end of list
        struct cleanupcommand *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = malloc(sizeof(struct cleanupcommand));
        if (!current->next) {
            printf("MEMORY MANAGEMENT: Unable to allocate memory for allocation info.\n");;
            return;
        }
        current->next->ptr = ptr;
        current->next->file = malloc(CALC_STR_MEM_SIZE(CALC_STR_REAL_LEN(file)));
        if (!current->next->file) {
            printf("MEMORY MANAGEMENT: Unable to allocate memory for allocation info.\n");;
            return;
        }
        strcpy(current->next->file, file);
        current->next->line = line;
        current->next->next = NULL;
    }
/*

    struct cleanupcommand *new_command = malloc(sizeof(struct cleanupcommand));
    if (!new_command) {
        printf("MEMORY MANAGEMENT: Unable to allocate memory for cleanup command.\n");;
        return;
    }

    new_command->ptr = ptr;

    new_command->file = malloc(CALC_STR_MEM_SIZE(CALC_STR_REAL_LEN(file)));
    if (!new_command->file) {
        printf("MEMORY MANAGEMENT: Unable to allocate memory for cleanup command.\n");;
        return;
    }
    strcpy(new_command->file, file);

    new_command->line = line;
    new_command->next = *head;
    *head = new_command;*/
}

/**
 * Iterates through the cleanup command list and executes each cleanup command,
 * freeing the associated memory. After execution, the cleanup list is cleared.
 *
 * @param head Pointer to the head of the cleanup command list.
 */
void cleanup_run_commands(struct cleanupcommand **cleanup_list) {
    if (!is_initialized) {
        printf("MEMORY MANAGEMENT: Memory management is not initialized\n");;
        return;
    }
    cleanup_run_commands_static(cleanup_list, get_allocations_head());
}
//endregion
//endregion