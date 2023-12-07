//
// Author: Lenovo
// Date: 02.12.2023
// Description: 
//

#ifndef FAV_PC_SP_23_24_BINARYDATAINIMG_MEMORY_MANAGEMENT_H
#define FAV_PC_SP_23_24_BINARYDATAINIMG_MEMORY_MANAGEMENT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define TRACKED_MALLOC(size) tracked_malloc(size, __FILE__, __LINE__)
#define TRACKED_FREE(ptr) tracked_free(ptr, __FILE__, __LINE__)

void* tracked_malloc(size_t size, const char* file, int line);
void tracked_free(void* ptr, const char* file, int line);

#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_MEMORY_MANAGEMENT_H