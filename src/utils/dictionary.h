//
// Author: Lenovo
// Date: 01.01.2024
// Description: 
//

#ifndef FAV_PC_SP_23_24_BINARYDATAINIMG_DICTIONARY_H
#define FAV_PC_SP_23_24_BINARYDATAINIMG_DICTIONARY_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned int DicCodeType;
typedef unsigned char *DicValueType;

typedef struct {
    DicValueType *array;
    size_t length;
    size_t capacity;
} DicValueArray;

typedef struct {
    size_t length;
    DicCodeType *array;
} DicCodeArray;

typedef struct {
    DicCodeType code;
    unsigned char *value;
} DictionaryEntry;

typedef struct {
    DictionaryEntry *dictionary_array;
    size_t length;
    size_t capacity;
} Dictionary;


DicCodeType dictionary_get_code_to_value(const Dictionary *dictionary, DicValueType value);

bool dictionary_add_entry(Dictionary *dictionary, DictionaryEntry entry);

bool dictionary_init(Dictionary *dictionary);

void dictionary_free(Dictionary *dictionary);

DicValueType dictionary_get_value_to_code(const Dictionary *dictionary, const DicCodeType code);

bool dicvaluearray_add_element(DicValueArray *dic_value_array, const DicValueType element);

bool is_value_invalid(const const DicValueType value);

bool is_code_invalid(DicCodeType code);

#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_DICTIONARY_H
