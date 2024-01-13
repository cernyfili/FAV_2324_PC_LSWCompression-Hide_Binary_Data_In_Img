//
// Author: Filip Cerny
// Date: 01.01.2024
// Description: This file contains dictionary functions.
//

#ifndef FAV_PC_SP_23_24_BINARYDATAINIMG_DICTIONARY_H
#define FAV_PC_SP_23_24_BINARYDATAINIMG_DICTIONARY_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"



//region STRUCTS
/**
 * Represents the type of the dictionary value.
 */
typedef unsigned char *DicValueType;

/**
 * Represents an array of dictionary values.
 */
typedef struct {
    DicValueType *array;
    size_t length;
    size_t capacity;
} DicValueArray;

/**
 * Represents the type of the dictionary code.
 */
typedef u_int16_t DicCodeType;

/**
 * Represents an array of static dictionary codes.
 */
typedef struct {
    DicCodeType *array;    // Array of static dictionary codes
    size_t length;         // Current number of elements in the array
    size_t capacity;       // Maximum capacity of the array
} StaticDicCodeArray;

/**
 * Represents an entry in the dictionary.
 */
typedef struct {
    DicCodeType code;       // Dictionary code
    unsigned char *value;   // Dictionary value
} DictionaryEntry;

/**
 *  Represents a dictionary for LZW compression.
 */
typedef struct {
    DictionaryEntry *array; // Array of dictionary entries
    size_t length;          // Current number of entries in the dictionary
    size_t capacity;        // Maximum capacity of the dictionary
} Dictionary;

//endregion

//region DICTIONARY FUNCTIONS
/**
 * Initializes the dictionary by allocating memory for its internal structures.
 * 
 * @param dictionary Pointer to the dictionary to be initialized.
 * @return true if initialization is successful, false otherwise.
 */
bool dictionary_init(Dictionary *dictionary);

/**
 * Frees the memory occupied by a dictionary, including its internal arrays.
 * 
 * @param dictionary Pointer to the dictionary to be freed.
 */
void dictionary_free(Dictionary *dictionary);

/**
 * Adds a dictionary entry to the dictionary.
 * 
 * @param dictionary Pointer to the dictionary.
 * @param entry Dictionary entry to be added.
 * @return true if the addition is successful, false otherwise.
 */
bool dictionary_add_entry(Dictionary *dictionary, DictionaryEntry entry);

/**
 * Checks if a specified value exists in the dictionary.
 * 
 * @param dictionary Pointer to the dictionary.
 * @param value Value to be checked.
 * @param ptr_return_value Pointer to a boolean variable to store the result.
 * @return true if the operation is successful, false otherwise.
 */
bool dictionary_is_value_in_dictionary(Dictionary dictionary, DicValueType value, bool *ptr_return_value);

/**
 * Checks if a specified code exists in the dictionary.
 * 
 * @param dictionary Dictionary to be searched.
 * @param code Code to be checked.
 * @param ptr_return_is_in_dict Pointer to a boolean variable to store the result.
 * @return true if the operation is successful, false otherwise.
 */
bool dictionary_is_code_in_dictionary(Dictionary dictionary, DicCodeType code, bool *ptr_return_is_in_dict);

/**
 * Retrieves the dictionary code corresponding to a specified value.
 * 
 * @param dictionary Pointer to the dictionary.
 * @param value Value for which the code is requested.
 * @param ptr_return_value Pointer to store the retrieved code.
 * @return true if the operation is successful, false otherwise.
 */
bool dictionary_get_code_to_value(Dictionary dictionary, DicValueType value, DicCodeType *ptr_return_value);

/**
 * Retrieves the dictionary value corresponding to a specified code.
 * 
 * @param dictionary Pointer to the dictionary.
 * @param code Code for which the value is requested.
 * @return Dictionary value corresponding to the code.
 */
DicValueType dictionary_get_value_to_code(Dictionary dictionary, DicCodeType code);
//endregion


/**
 * Adds an element to a dictionary value array.
 * 
 * @param dic_value_array Pointer to the dictionary value array.
 * @param element Element to be added.
 * @return true if the addition is successful, false otherwise.
 */
bool dicvaluearray_add_element(DicValueArray *dic_value_array, DicValueType element);

/**
 * Frees the memory occupied by a dictionary value array, including its internal array.
 * 
 * @param dic_value_array Pointer to the dictionary value array to be freed.
 */
void dicvaluearray_free(DicValueArray *dic_value_array);

/**
 * This function takes a DicValueArray and converts it into a single string,
 * allocating memory for the resulting string and updating the pointer accordingly.
 *
 * @param array The DicValueArray to be converted to a string.
 * @param ptr_return_str Pointer to the char pointer to store the resulting string.
 * @return true if conversion is successful, false otherwise.
 */
 bool dicvaluearray_to_string(DicValueArray array, char **ptr_return_str);

/**
 *  This function takes a DicValueArray and copies it to a new DicValueArray,
 * @param original_array  The DicValueArray to be copied.
 * @param ptr_return_copy_array  Pointer to the DicValueArray to store the copied array.
 * @return  true if copy is successful, false otherwise.
 */
 bool dicvaluearray_copy(DicValueArray original_array, DicValueArray *ptr_return_copy_array);

/**
 * Adds an element to a static dictionary code array.
 * 
 * @param dic_value_array Pointer to the static dictionary code array.
 * @param element Element to be added.
 * @return true if the addition is successful, false otherwise.
 */
bool diccodearray_add_element(StaticDicCodeArray *dic_value_array, DicCodeType element);

/**
 * Copies a static dictionary code array to a new static dictionary code array.
 * @param original_array  The StaticDicCodeArray to be copied.
 * @param copy_array  Pointer to the StaticDicCodeArray to store the copied array.
 * @return  true if copy is successful, false otherwise.
 */
 bool diccodearray_copy(StaticDicCodeArray original_array, StaticDicCodeArray *copy_array);

/**
 * Checks if a specified dictionary value is invalid (NULL).
 * 
 * @param value Dictionary value to be checked.
 * @return true if the value is invalid, false otherwise.
 */
bool is_value_invalid(DicValueType value);

#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_DICTIONARY_H
