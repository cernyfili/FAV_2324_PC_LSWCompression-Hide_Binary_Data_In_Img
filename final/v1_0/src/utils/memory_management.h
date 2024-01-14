//
// Author: Filip Cerny
// Date: 02.12.2023
// Description: This file contains functions for memory management.
//

#ifndef FAV_PC_SP_23_24_BINARYDATAINIMG_MEMORY_MANAGEMENT_H
#define FAV_PC_SP_23_24_BINARYDATAINIMG_MEMORY_MANAGEMENT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//region DEFINE, MACROS
/**
 * Macro for tracking memory allocation.
 * @param size Size of the memory to be allocated.
 * @param file File name where the cleanup command is added.
 * @param line Line number where the cleanup command is added.
 * @return Pointer to the allocated memory.
 */
#define TRACKED_MALLOC(size) tracked_malloc(size, __FILE__, __LINE__)

/**
 * Macro for tracking memory deallocation.
 * @param ptr Pointer to the memory to be freed.
 * @param file File name where the cleanup command is added.
 * @param line Line number where the cleanup command is added.
 */
#define TRACKED_FREE(ptr) tracked_free((void **) &( ptr ), __FILE__, __LINE__)

/**
 * Macro for cleaning up memory.
 * @param ptr Pointer to the memory to be freed.
 * @param head Pointer to the head of the cleanup command list.
 * @param file File name where the cleanup command is added.
 * @param line Line number where the cleanup command is added.
 */
#define CLEANUP_ADD_COMMAND(head, ptr) cleanup_add_command(head, (void **) &( ptr ), __FILE__, __LINE__)
//endregion

//region STRUCTS
/**
 * Structure for storing information about allocation.
 */
struct allocationinfo {
    void *pointer;      /**< Pointer to the allocated memory. */
    size_t size;        /**< Size of the allocated memory. */
    char *file;   /**< File name where the allocation occurred. */
    int line;           /**< Line number where the allocation occurred. */
    bool is_deallocated;/**< Flag indicating whether the memory is deallocated. */
};

/**
 * Linked list structure for storing information about allocation.
 */
struct allocation {
    struct allocationinfo info;
    struct allocation *next;
};


/**
 * Structure for storing information about cleanup commands.
 */
struct cleanupcommand {
    void **ptr;            /**< Pointer to the memory to be freed during cleanup. */
    char *file;      /**< File name where the cleanup command is added. */
    int line;              /**< Line number where the cleanup command is added. */
    struct cleanupcommand *next; /**< Pointer to the next cleanup command in the list. */
};
//endregion

//region Memory management functions

/**
 * Initializes the memory management system.
 * @return  true if initialization was successful, false otherwise
 */
void memory_management_init();

/**
 * Destroys the memory management system.
 * @return  true if destruction was successful, false otherwise
 */
void memory_management_destroy();

/**
 * This function allocates memory of the specified size using malloc and tracks the allocation
 * for memory management purposes. It records the file and line information of the allocation.
 *
 * @param size Size of the memory to be allocated.
 * @param file File name where the allocation is made.
 * @param line Line number where the allocation is made.
 * @return A pointer to the allocated memory.
 */
void *tracked_malloc(size_t size, const char *file, int line);

/**
 *  This function frees the memory pointed to by the given pointer using free, sets the pointer to NULL,
 * and tracks the deallocation for memory management purposes. It records the file and line information
 * of the deallocation.
 *
 * @param ptr Pointer to the memory to be deallocated and set to NULL.
 * @param file File name where the deallocation is made.
 * @param line Line number where the deallocation is made.
 */
void tracked_free(void **ptr, const char *file, int line);

/**
 *  This function prints a report summarizing the memory allocations and deallocations,
 * helping to identify potential memory leaks.
 */
void memory_management_report();
//endregion

//region Cleanup functions

/**
 *  This function adds a cleanup command to the cleanup list, specifying a pointer to be freed
 * and recording file and line information for cleanup purposes.
 *
 * @param head Pointer to the head of the cleanup command list.
 * @param ptr Pointer to the memory to be freed during cleanup.
 * @param file File name where the cleanup command is added.
 * @param line Line number where the cleanup command is added.
 */
void cleanup_add_command(struct cleanupcommand **head, void **ptr, char *file, int line);

/**
 *  This function iterates through the cleanup command list and executes each cleanup command,
 * freeing the associated memory. After execution, the cleanup list is cleared.
 *
 * @param head Pointer to the head of the cleanup command list.
 */
void cleanup_run_commands(struct cleanupcommand **cleanup_list);
//endregion

#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_MEMORY_MANAGEMENT_H