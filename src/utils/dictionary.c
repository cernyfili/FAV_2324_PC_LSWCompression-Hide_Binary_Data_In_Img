//
// Author: Lenovo
// Date: 01.01.2024
// Description: 
//

//Local includes
#include "dictionary.h"
#include "utils.h"


//Lib includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

//region DEFINE, MACROS
#define INITIAL_DICTIONARY_SIZE 256
#define DIC_ARR_INC 1000

#define DIC_CODE_INVALID (-1)

#define DIC_VALUE_INVALID NULL
//endregion

//region STRUCTS

//endregion

//region FUNCTIONS DECLARATION

//endregion

//region FUNCTIONS DEFINITIONS

//region PUBLIC FUNCTIONS
void dictionary_free(Dictionary *dictionary) {
    for (size_t i = 0; i < dictionary->length; ++i) {
        TRACKED_FREE(dictionary->dictionary_array[i].value);
    }

    TRACKED_FREE(dictionary->dictionary_array);
}

DicCodeType dictionary_get_code_to_value(const Dictionary *dictionary, const DicValueType value) {
    DicCodeType code = DIC_CODE_INVALID;

    //Check if data is not null
    if (!dictionary || !value) {
        LOG_MESSAGE(ERROR, "Error: Data is NULL.\n");
        return code;
    }

    for (int i = 0; i < dictionary->length; ++i) {
        DicValueType current_value = dictionary->dictionary_array[i].value;
        if (strcmp((char *) current_value, (char *) value) == 0) {
            code = dictionary->dictionary_array[i].code;
            return code;
        }
    }

    return code;
}

DicValueType dictionary_get_value_to_code(const Dictionary *dictionary, const DicCodeType code) {
    DicValueType value = DIC_VALUE_INVALID;

    //Check if data is not null
    if (!dictionary || !code) {
        LOG_MESSAGE(ERROR, "Error: Data is NULL.\n");
        return value;
    }

    //estimate
    DicCodeType current_code = dictionary->dictionary_array[code].code;
    if (current_code == code) {
        value = dictionary->dictionary_array[code].value;
        return value;
    }


    for (int i = 0; i < dictionary->length; ++i) {
        DicCodeType current_value = dictionary->dictionary_array[i].code;
        if (current_value == code) {
            value = dictionary->dictionary_array[i].value;
            return value;
        }
    }

    return value;
}

bool dictionary_add_entry(Dictionary *dictionary, const DictionaryEntry entry) {
    //if value doesnt fit
    if (dictionary->length == dictionary->capacity) {
        size_t new_capacity = dictionary->capacity + DIC_ARR_INC;

        DictionaryEntry *temp_ptr = realloc(dictionary->dictionary_array, new_capacity * sizeof(DictionaryEntry));
        if (temp_ptr == NULL) {
            LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
            TRACKED_FREE(temp_ptr);
            dictionary->dictionary_array = NULL;

            return false; // Memory allocation failed
        }

        dictionary->dictionary_array = temp_ptr;
        dictionary->capacity = new_capacity;
        temp_ptr = NULL;
    }

    dictionary->dictionary_array[dictionary->length++] = entry;

    return true;
}

bool dicvaluearray_add_element(DicValueArray *dic_value_array, const DicValueType element) {
    // If array is full, increase capacity
    if (dic_value_array->length == dic_value_array->capacity) {
        size_t new_capacity = dic_value_array->capacity + DIC_ARR_INC;

        DicValueType *temp_ptr = realloc(dic_value_array->array, new_capacity * sizeof(DicValueType));
        if (temp_ptr == NULL) {
            LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
            TRACKED_FREE(temp_ptr);
            dic_value_array->array = NULL;

            return false; // Memory allocation failed
        }

        dic_value_array->array = temp_ptr;
        dic_value_array->capacity = new_capacity;
        temp_ptr = NULL;
    }

    dic_value_array->array[dic_value_array->length++] = element;

    return true;
}

bool dictionary_init(Dictionary *dictionary) {
    (*dictionary).dictionary_array = (DictionaryEntry *) TRACKED_MALLOC(
            INITIAL_DICTIONARY_SIZE * sizeof(DictionaryEntry));
    (*dictionary).length = 0;
    (*dictionary).capacity = INITIAL_DICTIONARY_SIZE;

    for (size_t i = 0; i < INITIAL_DICTIONARY_SIZE; ++i) {
        (*dictionary).dictionary_array[i].code = (DicCodeType) i;

        // Save i to char*
        (*dictionary).dictionary_array[i].value = TRACKED_MALLOC(CALC_STR_MEM_SIZE(1));
        if (!(*dictionary).dictionary_array[i].value) {
            LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
            dictionary_free(dictionary);
            return false;
        }

        // Copy i to char*
        int result = snprintf((char *) (*dictionary).dictionary_array[i].value, CALC_STR_SIZE(1), "%c", (char) i);
        if (result < 0) {
            LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
            dictionary_free(dictionary);
            return false;
        }

        //because ascii is 0-255
    }

    return true;
}


bool is_value_invalid(DicValueType value){
    return value == DIC_VALUE_INVALID;
}

bool is_code_invalid(DicCodeType code){
    return code == DIC_CODE_INVALID;
}
//endregion

//region PRIVATE FUNCTIONS

//endregion

//endregion