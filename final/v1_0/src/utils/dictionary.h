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
#include <stdint.h>

#include "utils.h"



//region STRUCTS
#define DELETED_KEY -1
/**
 * Represents the type of the dictionary value.
 */
typedef unsigned char *dic_value_type;

/**
 * Represents an array of dictionary values.
 */
struct dicvaluearray {
    dic_value_type *array;
    size_t length;
    size_t capacity;
};

/**
 * Represents the type of the dictionary code.
 */
typedef int dic_code_type;

/**
 * Represents an array of static dictionary codes.
 */
struct staticdiccodearray {
    dic_code_type *array;    // Array of static dictionary codes
    size_t length;         // Current number of elements in the array
    size_t capacity;       // Maximum capacity of the array
};

/**
 * Represents an entry in the dictionary.
 */
struct dictionaryentry {
    dic_code_type code;       // dictionary code
    unsigned char *value;   // Dictionary value
};

/**
 *  Represents a dictionary for LZW compression.
 */
struct dictionary {
    struct dictionaryentry *array; // Array of dictionary entries
    size_t length;          // Current number of entries in the dictionary
    size_t capacity;        // Maximum capacity of the dictionary
};

//endregion

//region DICTIONARY FUNCTIONS
/**
 * Initializes the dictionary by allocating memory for its internal structures.
 * 
 * @param dictionary Pointer to the dictionary to be initialized.
 * @return true if initialization is successful, false otherwise.
 */
bool dictionary_init(struct dictionary *dictionary);

/**
 * Frees the memory occupied by a dictionary, including its internal arrays.
 * 
 * @param dictionary Pointer to the dictionary to be freed.
 */
void dictionary_free(struct dictionary *dictionary);

/**
 * Adds a dictionary entry to the dictionary.
 * 
 * @param dictionary Pointer to the dictionary.
 * @param entry struct dictionary entry to be added.
 * @return true if the addition is successful, false otherwise.
 */
bool dictionary_add_entry(struct dictionary *dictionary, struct dictionaryentry entry);

/**
 * Checks if a specified value exists in the dictionary.
 * 
 * @param dictionary Pointer to the dictionary.
 * @param value Value to be checked.
 * @param ptr_return_value Pointer to a boolean variable to store the result.
 * @return true if the operation is successful, false otherwise.
 */
bool dictionary_is_value_in_dictionary(struct dictionary dictionary, dic_value_type value, bool *ptr_return_value);

/**
 * Checks if a specified code exists in the dictionary.
 * 
 * @param dictionary struct dictionary to be searched.
 * @param code Code to be checked.
 * @param ptr_return_is_in_dict Pointer to a boolean variable to store the result.
 * @return true if the operation is successful, false otherwise.
 */
bool dictionary_is_code_in_dictionary(struct dictionary dictionary, dic_code_type code, bool *ptr_return_is_in_dict);

/**
 * Retrieves the dictionary code corresponding to a specified value.
 * 
 * @param dictionary Pointer to the dictionary.
 * @param value Value for which the code is requested.
 * @param ptr_return_value Pointer to store the retrieved code.
 * @return true if the operation is successful, false otherwise.
 */
bool dictionary_get_code_to_value(struct dictionary dictionary, dic_value_type value, dic_code_type *ptr_return_value);

/**
 * Retrieves the dictionary value corresponding to a specified code.
 * 
 * @param dictionary Pointer to the dictionary.
 * @param code Code for which the value is requested.
 * @return struct dictionary value corresponding to the code.
 */
dic_value_type dictionary_get_value_to_code(struct dictionary dictionary, dic_code_type code);
//endregion


/**
 * Adds an element to a dictionary value array.
 * 
 * @param dic_value_array Pointer to the dictionary value array.
 * @param element Element to be added.
 * @return true if the addition is successful, false otherwise.
 */
bool dicvaluearray_add_element(struct dicvaluearray *dic_value_array, dic_value_type element);

/**
 * Frees the memory occupied by a dictionary value array, including its internal array.
 * 
 * @param dic_value_array Pointer to the dictionary value array to be freed.
 */
void dicvaluearray_free(struct dicvaluearray *dic_value_array);

/**
 * This function takes a struct dicvaluearray and converts it into a single string,
 * allocating memory for the resulting string and updating the pointer accordingly.
 *
 * @param array The struct dicvaluearray to be converted to a string.
 * @param ptr_return_str Pointer to the char pointer to store the resulting string.
 * @return true if conversion is successful, false otherwise.
 */
bool dicvaluearray_to_string(struct dicvaluearray array, char **ptr_return_str);

/**
 *  This function takes a struct dicvaluearray and copies it to a new struct dicvaluearray,
 * @param original_array  The struct dicvaluearray to be copied.
 * @param ptr_return_copy_array  Pointer to the struct dicvaluearray to store the copied array.
 * @return  true if copy is successful, false otherwise.
 */
bool dicvaluearray_copy(struct dicvaluearray original_array, struct dicvaluearray *ptr_return_copy_array);

/**
 * Adds an element to a static dictionary code array.
 * 
 * @param dic_value_array Pointer to the static dictionary code array.
 * @param element Element to be added.
 * @return true if the addition is successful, false otherwise.
 */
bool diccodearray_add_element(struct staticdiccodearray *dic_value_array, dic_code_type element);

/**
 * Copies a static dictionary code array to a new static dictionary code array.
 * @param original_array  The struct staticdiccodearray to be copied.
 * @param copy_array  Pointer to the struct staticdiccodearray to store the copied array.
 * @return  true if copy is successful, false otherwise.
 */
bool diccodearray_copy(struct staticdiccodearray original_array, struct staticdiccodearray *copy_array);

/**
 * Checks if a specified dictionary value is invalid (NULL).
 * 
 * @param value struct dictionary value to be checked.
 * @return true if the value is invalid, false otherwise.
 */
bool is_value_invalid(dic_value_type value);

#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_DICTIONARY_H
