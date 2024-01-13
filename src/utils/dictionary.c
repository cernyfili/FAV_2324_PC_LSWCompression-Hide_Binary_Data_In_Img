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
#define DIC_ARR_INC 1000

/**
 * Represents the invalid value of the dictionary.
 */
#define DIC_VALUE_INVALID NULL
//endregion

//region FUNCTIONS DEFINITIONS

//region PUBLIC FUNCTIONS
//region DICTIONARY FUNCTIONS
/**
 * Initializes a dictionary with an initial size and assigns codes and values to each entry.
 * 
 * @param dictionary Pointer to the dictionary to be initialized.
 * @return true if the initialization is successful, false otherwise.
 */
bool dictionary_init(Dictionary *dictionary) {
    if (!dictionary) {
        LOG_MESSAGE(ERROR, "Dictionary pointer is NULL.");
        return false;
    }

    dictionary->capacity = INITIAL_DICTIONARY_SIZE;
    (*dictionary).array = TRACKED_MALLOC(dictionary->capacity * sizeof(DictionaryEntry));
    if (!(*dictionary).array) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        return false;
    }
    (*dictionary).length = 0;

    for (size_t i = 0; i < dictionary->capacity; i++) {
        (*dictionary).array[i].code = (DicCodeType) i;

        // Save i to char*
        (*dictionary).array[i].value = TRACKED_MALLOC(2);
        if (!(*dictionary).array[i].value) {
            LOG_MESSAGE(ERROR, "Memory allocation failed.");
            dictionary_free(dictionary);
            return false;
        }

        // Copy i to char*
        int result = snprintf((char *) (*dictionary).array[i].value, 2, "%c", (char) i);
        if (result < 0) {
            LOG_MESSAGE(ERROR, "Memory allocation failed.");
            dictionary_free(dictionary);
            return false;
        }

        (dictionary->length)++;

        //because ascii is 0-255
    }

    return true;
}

/**
 * Frees the memory occupied by a dictionary, including its internal array of entries.
 * 
 * @param dictionary Pointer to the dictionary to be freed.
 */
void dictionary_free(Dictionary *dictionary) {
    if (dictionary->array == NULL) {
        return;
    }

    for (size_t i = 0; i < dictionary->length; ++i) {
        if (dictionary->array[i].value == NULL) {
            continue;
        }
        TRACKED_FREE(dictionary->array[i].value);
    }


    TRACKED_FREE(dictionary->array);
}

/**
 * Retrieves the dictionary code corresponding to a specified value.
 * 
 * @param dictionary Pointer to the dictionary.
 * @param value Value for which the code is requested.
 * @param ptr_return_value Pointer to store the retrieved code.
 * @return true if the operation is successful, false otherwise.
 */
bool dictionary_get_code_to_value(Dictionary dictionary, DicValueType value, DicCodeType *ptr_return_value) {

    //Check if data is not null
    if (!dictionary.array || !value || !ptr_return_value) {
        LOG_MESSAGE(ERROR, "Wrong argument");
        return false;
    }

    for (int i = 0; i < dictionary.length; ++i) {
        DicValueType current_value = dictionary.array[i].value;
        if (strcmp((char *) current_value, (char *) value) == 0) {
            *ptr_return_value = dictionary.array[i].code;
            return true;
        }
    }

    return false;
}

/**
 * Checks if a specified value exists in the dictionary.
 * 
 * @param dictionary Pointer to the dictionary.
 * @param value Value to be checked.
 * @param ptr_return_value Pointer to a boolean variable to store the result.
 * @return true if the operation is successful, false otherwise.
 */
bool dictionary_is_value_in_dictionary(Dictionary dictionary, DicValueType value, bool *ptr_return_value) {
    //Check if data is not null
    if (!dictionary.array || !value || !ptr_return_value) {
        LOG_MESSAGE(ERROR, "Wrong argument");
        return false;
    }

    for (int i = 0; i < dictionary.length; ++i) {
        DicValueType current_value = dictionary.array[i].value;
        if (strcmp((char *) current_value, (char *) value) == 0) {
            *ptr_return_value = true;
            return true;
        }
    }
    *ptr_return_value = false;

    return true;
}

/**
 * Checks if a specified code exists in the dictionary.
 * 
 * @param dictionary Dictionary to be searched.
 * @param code Code to be checked.
 * @param ptr_return_is_in_dict Pointer to a boolean variable to store the result.
 * @return true if the operation is successful, false otherwise.
 */
bool
dictionary_is_code_in_dictionary(Dictionary dictionary, DicCodeType code, bool *ptr_return_is_in_dict) {
    if (!dictionary.array || !ptr_return_is_in_dict) {
        LOG_MESSAGE(ERROR, "Wrong argument");
        return false;
    }

    for (int i = 0; i < dictionary.length; ++i) {
        if (dictionary.array[i].code == code) {
            *ptr_return_is_in_dict = true;
            return true;
        }
    }
    *ptr_return_is_in_dict = false;
    return true;
}

/**
 * Retrieves the dictionary value corresponding to a specified code.
 * 
 * @param dictionary Pointer to the dictionary.
 * @param code Code for which the value is requested.
 * @return Dictionary value corresponding to the code.
 */
DicValueType dictionary_get_value_to_code(Dictionary dictionary, DicCodeType code) {
    DicValueType value = DIC_VALUE_INVALID;

    //Check if data is not null
    if (!dictionary.array) {
        LOG_MESSAGE(ERROR, "Data is NULL.");
        return value;
    }

    if (code < dictionary.length) {
        //estimate
        DicCodeType current_code = dictionary.array[code].code;
        if (current_code == code) {
            value = dictionary.array[code].value;
            return value;
        }
    }

    for (int i = 0; i < dictionary.length; ++i) {
        DicCodeType current_value = dictionary.array[i].code;
        if (current_value == code) {
            value = dictionary.array[i].value;
            return value;
        }
    }

    return value;
}

/**
 * Adds a dictionary entry to the dictionary using dynamic array functions.
 * 
 * @param dictionary Pointer to the dictionary.
 * @param entry Dictionary entry to be added.
 * @return true if the addition is successful, false otherwise.
 */
bool dictionary_add_entry(Dictionary *dictionary, DictionaryEntry entry) {
    //Check if data is not null
    if (!dictionary->array) {
        LOG_MESSAGE(ERROR, "Data is NULL.");
        return false;
    }
    //Check if value is not null
    if (!entry.value) {
        LOG_MESSAGE(ERROR, "Value is NULL.");
        return false;
    }

    //if value doesnt fit
    if (dictionary->length == dictionary->capacity) {
        size_t new_capacity = dictionary->capacity + DIC_ARR_INC;

        DictionaryEntry *temp_ptr = TRACKED_MALLOC(new_capacity * sizeof(DictionaryEntry));
        if (temp_ptr == NULL) {
            LOG_MESSAGE(ERROR, "Memory allocation failed.");
            dictionary_free(dictionary);
            return false; // Memory allocation failed
        }
        memcpy(temp_ptr, dictionary->array, dictionary->capacity * sizeof(DictionaryEntry));
        TRACKED_FREE(dictionary->array);

        dictionary->array = temp_ptr;
        dictionary->capacity = new_capacity;
        temp_ptr = NULL;
    }

    dictionary->array[dictionary->length].code = entry.code;

    dictionary->array[dictionary->length].value = TRACKED_MALLOC(CALC_STR_MEM_SIZE(strlen((char *) entry.value)));
    if (!dictionary->array[dictionary->length].value) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        dictionary_free(dictionary);
        return false;
    }
    strcpy((char *) dictionary->array[dictionary->length].value, (char *) entry.value);

    dictionary->length++;

    return true;
    /*return dynamicarray_add_element((DynamicArray *) dictionary, (void *) &entry, sizeof(DictionaryEntry));*/
}
//endregion


/**
 * Adds an element to a dictionary value array using dynamic array functions.
 * 
 * @param dic_value_array Pointer to the dictionary value array.
 * @param element Element to be added.
 * @return true if the addition is successful, false otherwise.
 */
bool dicvaluearray_add_element(DicValueArray *dic_value_array, DicValueType element) {
    //Check if data is not null
    if (!dic_value_array->array || !element) {
        LOG_MESSAGE(ERROR, "Wrong argument");
        return false;
    }

    // If array is full, increase capacity
    if (dic_value_array->length == dic_value_array->capacity) {
        size_t new_capacity = dic_value_array->capacity + DIC_ARR_INC;

        DicValueType *temp_ptr = realloc(dic_value_array->array, new_capacity * sizeof(DicValueType));
        if (temp_ptr == NULL) {
            LOG_MESSAGE(ERROR, "Memory allocation failed.");
            TRACKED_FREE(temp_ptr);
            dicvaluearray_free(dic_value_array);

            return false; // Memory allocation failed
        }

        dic_value_array->array = temp_ptr;
        dic_value_array->capacity = new_capacity;
        temp_ptr = NULL;
    }

    dic_value_array->array[dic_value_array->length] = TRACKED_MALLOC(CALC_STR_MEM_SIZE(strlen((char *) element)));
    if (!dic_value_array->array[dic_value_array->length]) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        dicvaluearray_free(dic_value_array);
        return false;
    }
    strcpy((char *) dic_value_array->array[dic_value_array->length], (char *) element);


    (dic_value_array->length)++;
    return true;
}

/**
 * Frees the memory occupied by a dictionary value array, including its internal array.
 * 
 * @param dic_value_array Pointer to the dictionary value array to be freed.
 */
void dicvaluearray_free(DicValueArray *dic_value_array) {
    if (!dic_value_array->array) {
        return;
    }

    for (size_t i = 0; i < dic_value_array->length; ++i) {
        TRACKED_FREE(dic_value_array->array[i]);
    }

    TRACKED_FREE(dic_value_array->array);
}


/**
 * This function takes a DicValueArray and converts it into a single string,
 * allocating memory for the resulting string and updating the pointer accordingly.
 *
 * @param array The DicValueArray to be converted to a string.
 * @param ptr_return_str Pointer to the char pointer to store the resulting string.
 * @return true if conversion is successful, false otherwise.
 */
bool dicvaluearray_to_string(DicValueArray array, char **ptr_return_str) {
    //Check if data is not null
    if (!array.array || !ptr_return_str) {
        LOG_MESSAGE(ERROR, "Arguments not valid.");
        return false;
    }

    size_t final_str_len = 0;
    for (int i = 0; i < array.length; ++i) {
        final_str_len += strlen((char *) array.array[i]);
    }

    *ptr_return_str = TRACKED_MALLOC(CALC_STR_MEM_SIZE(final_str_len));
    if (!*ptr_return_str) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        return false;
    }
    strcpy(*ptr_return_str, (char *) array.array[0]);
    for (int i = 1; i < array.length; ++i) {
        strcat(*ptr_return_str, (char *) array.array[i]);
    }

    return true;
}

/**
 * This function takes a DicValueArray and copies it to a new DicValueArray,
 * allocating memory for the resulting array and updating the pointer accordingly.
 *
 * @param original_array The DicValueArray to be copied.
 * @param ptr_return_copy_array Pointer to the DicValueArray to store the copied array.
 * @return true if copy is successful, false otherwise.
 */
bool dicvaluearray_copy(DicValueArray original_array, DicValueArray *ptr_return_copy_array) {
    //Check if data is not null
    if (!original_array.array || !ptr_return_copy_array) {
        LOG_MESSAGE(ERROR, "Wrong argument");
        return false;
    }

    //Prepare ptr_return
    ptr_return_copy_array->length = 0;
    ptr_return_copy_array->capacity = 0;
    ptr_return_copy_array->array = NULL;

    (ptr_return_copy_array)->capacity = (original_array).length;
    (ptr_return_copy_array)->array = TRACKED_MALLOC((ptr_return_copy_array)->capacity * sizeof(DicValueType));
    if (!(ptr_return_copy_array)->array) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        return false;
    }
    (ptr_return_copy_array)->length = (ptr_return_copy_array)->capacity;

    //Copy contents of ( original_array) to ( copy_array)
    for (int i = 0; i < (ptr_return_copy_array)->length; ++i) {
        DicValueType value = (original_array).array[i];
        (ptr_return_copy_array)->array[i] = TRACKED_MALLOC(CALC_STR_MEM_SIZE(strlen((char *) value)));
        if (!(ptr_return_copy_array)->array[i]) {
            LOG_MESSAGE(ERROR, "Memory allocation failed.");
            dicvaluearray_free(ptr_return_copy_array);
            return false;
        }
        memcpy((ptr_return_copy_array)->array[i], value, CALC_STR_MEM_SIZE(strlen((char *) value)));
    }

    return true;
}

/**
 * Adds an element to a static dictionary code array.
 * 
 * @param dic_value_array Pointer to the static dictionary code array.
 * @param element Element to be added.
 * @return true if the addition is successful, false otherwise.
 */
bool diccodearray_add_element(StaticDicCodeArray *dic_value_array, DicCodeType element) {
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
 * This function copies StaticDicCodeArray to another StaticDicCodeArray
 * @param original_array is the original array
 * @param copy_array is the array to be copied tos
 * @return  true if copy is successful, false otherwise.
 */
bool diccodearray_copy(StaticDicCodeArray original_array, StaticDicCodeArray *copy_array) {
    //Check if data is not null
    if (!original_array.array || !copy_array) {
        LOG_MESSAGE(ERROR, "Wrong argument");
        return false;
    }

    copy_array->capacity = original_array.length;
    copy_array->array = TRACKED_MALLOC(copy_array->capacity * sizeof(DicCodeType));
    if (copy_array->array == NULL) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        return false;
    }


    // Copy the compressed data from original_array to compressed_data
    copy_array->length = copy_array->capacity;
    memcpy(copy_array->array, original_array.array, copy_array->length * sizeof(DicCodeType));
    return true;
}

/**
 * Checks if a given dictionary value is equal to the invalid value.
 * 
 * @param value Dictionary value to be checked.
 * @return true if the value is invalid, false otherwise.
 */
bool is_value_invalid(DicValueType value) {
    return value == DIC_VALUE_INVALID;
}

//endregion

//endregion