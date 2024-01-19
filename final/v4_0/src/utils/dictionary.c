//
// Author: Filip Cerny
// Date: 01.01.2024
// Description: This file contains dictionary functions.
//

//Local includes
#include "dictionary.h"
#include "utils.h"


//Lib includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//region DEFINE, MACROS
/**
 * Represents the initial size of the dictionary.
 */
#define INITIAL_DICTIONARY_SIZE 256
/**
 * Represents the increment of the dictionary size.
 */
#define DIC_ARR_INC 50000
/**
 * Represents the invalid value of the dictionary.
 */
#define DIC_VALUE_INVALID NULL

/**
 * Represents the initial size of char array.
 */
#define CHARARRAY_INIT_SIZE 10000
//endregion

//region FUNCTIONS DEFINITIONS

//region PUBLIC FUNCTIONS

//region DICTIONARY FUNCTIONS
/**
 * Prints the contents of a dictionary.
 * @param dictionary  Dictionary to be printed.
 */
void dictionary_print(struct dictionary dictionary) {
    for (int i = 0; i < dictionary.length; i++) {
        printf("Code: %d, Value: %s\n", i, dictionary.array[i]);
    }
}

/**
 * Initializes a dictionary with an initial size and assigns codes and values to each entry.
 * 
 * @param dictionary Pointer to the dictionary to be initialized.
 * @return true if the initialization is successful, false otherwise.
 */
bool dictionary_init(struct dictionary *dictionary) {
    if (!dictionary) {
        LOG_MESSAGE(ERROR, "struct dictionary pointer is NULL.");
        return false;
    }

    dictionary->capacity = DIC_ARR_INC;
    (*dictionary).array = TRACKED_MALLOC(dictionary->capacity * sizeof(dic_value_type));
    if (!(*dictionary).array) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        return false;
    }
    (*dictionary).length = 0;

    for (size_t i = 0; i < INITIAL_DICTIONARY_SIZE; i++) {

        // Save i to char*
        (*dictionary).array[i] = TRACKED_MALLOC(2);
        if (!(*dictionary).array[i]) {
            LOG_MESSAGE(ERROR, "Memory allocation failed.");
            dictionary_free(dictionary);
            return false;
        }

        // Copy i to char*
        int result = snprintf((char *) (*dictionary).array[i], 2, "%c", (char) i);
        if (result < 0) {
            LOG_MESSAGE(ERROR, "Memory allocation failed.");
            dictionary_free(dictionary);
            return false;
        }

        (dictionary->length)++;

        //because ascii is 0-255
    }

    for (int i = INITIAL_DICTIONARY_SIZE; i < dictionary->capacity; ++i) {
        /*(*dictionary).array[i] = TRACKED_MALLOC(1);
        if (!(*dictionary).array[i]) {
            LOG_MESSAGE(ERROR, "Memory allocation failed.");
            dictionary_free(dictionary);
            return false;
        }*/
        (*dictionary).array[i] = NULL;
    }

    return true;
}

/**
 * Frees the memory occupied by a dictionary, including its internal array of entries.
 * 
 * @param dictionary Pointer to the dictionary to be freed.
 */
void dictionary_free(struct dictionary *dictionary) {
    if (dictionary->array == NULL) {
        return;
    }

    for (size_t i = 0; i < dictionary->length; i++) {
        if (dictionary->array[i] == NULL) {
            continue;
        }
        TRACKED_FREE(dictionary->array[i]);
    }


    TRACKED_FREE(dictionary->array);
}

/**
 * Checks if a specified code exists in the dictionary.
 * 
 * @param dictionary struct dictionary to be searched.
 * @param code Code to be checked.
 * @param ptr_return_is_in_dict Pointer to a boolean variable to store the result.
 * @return true if the operation is successful, false otherwise.
 */
bool
dictionary_is_code_in_dictionary(struct dictionary dictionary, dic_code_type code, bool *ptr_return_is_in_dict) {
    if (!dictionary.array || !ptr_return_is_in_dict) {
        LOG_MESSAGE(ERROR, "Wrong argument");
        return false;
    }

    if (code < dictionary.length) {
        //estimate
        *ptr_return_is_in_dict = true;
        return true;
    }

    *ptr_return_is_in_dict = false;
    return true;
}

/**
 * Retrieves the dictionary value corresponding to a specified code.
 * 
 * @param dictionary Pointer to the dictionary.
 * @param code Code for which the value is requested.
 * @return struct dictionary value corresponding to the code.
 */
dic_value_type dictionary_get_value_to_code(struct dictionary dictionary, dic_code_type code) {
    dic_value_type value = DIC_VALUE_INVALID;

    //Check if data is not null
    if (!dictionary.array) {
        LOG_MESSAGE(ERROR, "Data is NULL.");
        return value;
    }

    if (code < dictionary.length) {
        //estimate

        value = dictionary.array[code];
        return value;
    }

    return value;
}

/**
 * Adds a dictionary entry to the dictionary using dynamic array functions.
 * 
 * @param dictionary Pointer to the dictionary.
 * @param entry struct dictionary entry to be added.
 * @return true if the addition is successful, false otherwise.
 */
bool dictionary_add_entry(struct dictionary *dictionary, dic_value_type entry) {
    //Check if data is not null
    if (!dictionary->array) {
        LOG_MESSAGE(ERROR, "Data is NULL.");
        return false;
    }
    //Check if value is not null
    if (!entry) {
        LOG_MESSAGE(ERROR, "Value is NULL.");
        return false;
    }

    //if value doesnt fit
    if (dictionary->length >= dictionary->capacity) {
        size_t new_capacity = dictionary->capacity + DIC_ARR_INC;

        dic_value_type *temp_ptr = TRACKED_MALLOC(new_capacity * sizeof(dic_value_type));
        if (temp_ptr == NULL) {
            LOG_MESSAGE(ERROR, "Memory allocation failed.");
            dictionary_free(dictionary);
            return false; // Memory allocation failed
        }
        memcpy(temp_ptr, dictionary->array, dictionary->capacity * sizeof(dic_value_type));
        TRACKED_FREE(dictionary->array);

        dictionary->array = temp_ptr;
        dictionary->capacity = new_capacity;
        temp_ptr = NULL;
    }

    size_t str_len = strlen((char *)entry);

    dictionary->array[dictionary->length] = TRACKED_MALLOC(CALC_STR_MEM_SIZE(str_len));
    if (!dictionary->array[dictionary->length]) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        dictionary_free(dictionary);
        return false;
    }
    memcpy(dictionary->array[dictionary->length], entry, CALC_STR_MEM_SIZE(str_len));/*
    strcpy((char *) dictionary->array[dictionary->length], (char *) entry);*/

    dictionary->length++;

    return true;
}
//endregion


//region DICVALUEARRAY FUNCTIONS
/**
 * Checks if a given dictionary value is equal to the invalid value.
 *
 * @param value struct dictionary value to be checked.
 * @return true if the value is invalid, false otherwise.
 */
bool is_value_invalid(dic_value_type value) {
    return value == DIC_VALUE_INVALID;
}

/**
 * Adds an element to a dictionary value array using dynamic array functions.
 * 
 * @param dic_value_array Pointer to the dictionary value array.
 * @param element Element to be added.
 * @return true if the addition is successful, false otherwise.
 */
bool dicvaluearray_add_element(struct dicvaluearray *dic_value_array, dic_value_type element) {
    //Check if data is not null
    if (!dic_value_array->array || !element) {
        LOG_MESSAGE(ERROR, "Wrong argument");
        return false;
    }

    size_t str_len = strlen((char *)element);
    // If array is full, increase capacity
    if ((dic_value_array->length + str_len) >= dic_value_array->capacity) {
        size_t new_capacity = dic_value_array->capacity + DIC_ARR_INC;

        char *temp_ptr = TRACKED_MALLOC(new_capacity * sizeof(char));
        if (temp_ptr == NULL) {
            LOG_MESSAGE(ERROR, "Memory allocation failed.");
            dicvaluearray_free(dic_value_array);
            return false; // Memory allocation failed
        }
        memcpy(temp_ptr, dic_value_array->array, dic_value_array->length * sizeof(char));
        dicvaluearray_free(dic_value_array);

        dic_value_array->array = temp_ptr;
        dic_value_array->capacity = new_capacity;
        temp_ptr = NULL;
    }


    memcpy(dic_value_array->array + (dic_value_array->length * sizeof(char)), element, str_len * sizeof(char));
    dic_value_array->char_count += str_len;
    dic_value_array->length += str_len;
    /*

    strcpy((char *) dic_value_array->array[dic_value_array->length], (char *) element);*/

    return true;
}

/**
 * Frees the memory occupied by a dictionary value array, including its internal array.
 * 
 * @param dic_value_array Pointer to the dictionary value array to be freed.
 */
void dicvaluearray_free(struct dicvaluearray *dic_value_array) {
    if (!dic_value_array->array) {
        return;
    }

    TRACKED_FREE(dic_value_array->array);
}
//endregion

//region DICCODEARRAY FUNCTIONS
/**
 * Adds an element to a static dictionary code array.
 * 
 * @param dic_value_array Pointer to the static dictionary code array.
 * @param element Element to be added.
 * @return true if the addition is successful, false otherwise.
 */
bool diccodearray_add_element(struct staticdiccodearray *dic_value_array, dic_code_type element) {
    //Check if data is not null
    if (!dic_value_array->array) {
        LOG_MESSAGE(ERROR, "Wrong argument");
        return false;
    }

    // If array is full, increase capacity
    if (dic_value_array->length == dic_value_array->capacity) {
        LOG_MESSAGE(ERROR, "Array is full");
        return false;
    }

    dic_value_array->array[dic_value_array->length++] = element;

    return true;
}


/**
 * This function copies struct staticdiccodearray to another struct staticdiccodearray
 * @param original_array is the original array
 * @param copy_array is the array to be copied tos
 * @return  true if copy is successful, false otherwise.
 */
bool diccodearray_copy(struct staticdiccodearray original_array, struct staticdiccodearray *copy_array) {
    //Check if data is not null
    if (!original_array.array || !copy_array) {
        LOG_MESSAGE(ERROR, "Wrong argument");
        return false;
    }

    copy_array->capacity = original_array.length;
    copy_array->array = TRACKED_MALLOC(copy_array->capacity * sizeof(dic_code_type));
    if (copy_array->array == NULL) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        return false;
    }


    // Copy the compressed data from original_array to compressed_data
    copy_array->length = copy_array->capacity;
    memcpy(copy_array->array, original_array.array, copy_array->length * sizeof(dic_code_type));
    return true;
}
//endregion

//endregion

//endregion+