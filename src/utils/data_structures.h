//
// Author: Lenovo
// Date: 07.12.2023
// Description: 
//

#ifndef FAV_PC_SP_23_24_BINARYDATAINIMG_DATA_STRUCTURES_H
#define FAV_PC_SP_23_24_BINARYDATAINIMG_DATA_STRUCTURES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define CALC_STR_MEM_SIZE(n) ((n) * sizeof(char) + 1)
#define CALC_STR_SIZE(n) ((n) + 1)

typedef unsigned char PayloadType;
typedef struct {
    PayloadType *array;
    size_t length;
    size_t capacity;
} PayloadArray;


#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_DATA_STRUCTURES_H
