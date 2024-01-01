//
// Author: Filip Cerny
// Date: 07.11.2023
// Description:
//

//Local includes
#include "compression.h"
#include "utils/utils.h"
#include "utils/stack.h"

//Lib includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define INITIAL_DICTIONARY_SIZE 256

#define CALC_STR_MEM_SIZE(n) ((n) * sizeof(char) + 1)
#define CALC_STR_SIZE(n) ((n) + 1)

#define DIC_ARR_INC 1000


typedef struct {
    DicCodeType code;
    unsigned char *value;
} DictionaryEntry;

typedef struct {
    size_t length;
    DicCodeType *array;
} DicCodeArray;

typedef struct {
    DicValueType *array;
    size_t length;
    size_t capacity;
} DicValueArray;

typedef struct {
    DictionaryEntry *dictionary_array;
    size_t length;
    size_t capacity;
} Dictionary;


DicCodeType dictionary_get_code_to_value(const Dictionary *dictionary, const DicValueType value);

bool dictionary_add_entry(Dictionary *dictionary, const DictionaryEntry entry);

bool dictionary_init(Dictionary *dictionary);


void dictionary_free(Dictionary *dictionary);

/**
 * @brief Function to compress binary data using LZW algorithm
 * bereme char array jako string s \0
 * plytvani pameti na array
 * dalo by se zlepist lepsi datovou strukturou
 * nebo efektivnejsim vyhledavacim algoritmem ve slovniku
 * result size of original data
 * dicitonary increment fixed size
 * @param data array of unsigned char
 * @param size
 * @return
 */
bool lzw_compress(const char *data, DicCodeArray *compressed_data) {
    if (!data) {
        LOG_MESSAGE(ERROR, "Error: Data is NULL.\n");
        return false;
    }

    size_t data_size = strlen(data);
    DicCodeType *result_code;//array of int
    size_t result_length = 0;
    size_t result_capacity = data_size;//compressed data cant be bigger than original data

    int result;
    void *temp_ptr = NULL;


    result_code = (DicCodeType *) TRACKED_MALLOC(result_capacity * sizeof(DicCodeType));
    if (result_code == NULL) {
        LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
        TRACKED_FREE(result_code);
        return false;
    }

    //region LZW Initialize the dictionary with single-byte entries
    Dictionary dictionary;
    dictionary_init(&dictionary);
    //endregion

    // Start compression
    DicCodeType next_dic_code = INITIAL_DICTIONARY_SIZE;//start where we ended 256

    //LZW P_INPUT_CHAR = first input character
    DicValueType p_input_char = TRACKED_MALLOC(CALC_STR_MEM_SIZE(1));
    if (!p_input_char) {
        LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
        dictionary_free(&dictionary);
        return false;
    }
    result = snprintf((char *) p_input_char, CALC_STR_SIZE(1), "%c", data[0]);
    if (result < 0) {
        LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
        TRACKED_FREE(p_input_char);
        return false;
    }

    //LZW WHILE not end of input stream
    for (size_t i = 1; i < data_size; ++i) {
        //C_NEXT_CHAR = next input character
        char c_next_char = data[i];

        //todo make is in dictionary function

        // P_INPUT_CHAR + C_NEXT_CHAR
        size_t length = strlen((char *) p_input_char) + 1;
        DicValueType combined_pc = TRACKED_MALLOC(CALC_STR_MEM_SIZE(length));
        if (!combined_pc) {
            LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
            TRACKED_FREE(combined_pc);
            return false;
        }
        result = snprintf((char *) combined_pc, CALC_STR_SIZE(length), "%s%c", p_input_char, c_next_char);
        if (result < 0) {
            LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
            TRACKED_FREE(combined_pc);
            return false;
        }

        // Check if the combined code is present in the dictionary
        DicCodeType code = dictionary_get_code_to_value(&dictionary, combined_pc);

        //LZW IF P_INPUT_CHAR + C_NEXT_CHAR is in the string table
        if (code == DIC_CODE_INVALID) {

            //LZW P_INPUT_CHAR = P_INPUT_CHAR + C_NEXT_CHAR
            size_t new_length = strlen((char *) combined_pc);
            temp_ptr = realloc(p_input_char, CALC_STR_MEM_SIZE(new_length));
            if (temp_ptr == NULL) {
                LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
                TRACKED_FREE(p_input_char);
                p_input_char = NULL;
                return false;
            }
            p_input_char = temp_ptr;
            temp_ptr = NULL;

            strcpy((char *) p_input_char, (char *) combined_pc);

        } else {
            //LZW save the code for INPUT_CHAR to result
            // shouldnt happen
            if (result_length == (result_capacity - 1) || result_length > UINT_MAX) {
                LOG_MESSAGE(ERROR, "Error: Data to compress are too big.\n");
            }

            DicCodeType tmp_code = dictionary_get_code_to_value(&dictionary, p_input_char);
            if (tmp_code == DIC_CODE_INVALID) {
                LOG_MESSAGE(ERROR, "Error: Code not found in dictionary.\n");
                return false;
            }
            result_code[result_length++] = tmp_code;

            //LZW add P_INPUT_CHAR + C_NEXT_CHAR to the string table
            DictionaryEntry entry = {next_dic_code++, combined_pc};
            bool is_succes = dictionary_add_entry(&dictionary, entry);

            //LZW P_INPUT_CHAR = C_NEXT_CHAR
            //p_input_char new size
            size_t new_length = 1;//char is size of one
            temp_ptr = realloc(p_input_char, CALC_STR_MEM_SIZE(new_length));
            if (temp_ptr == NULL) {
                LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
                TRACKED_FREE(p_input_char);
                p_input_char = NULL;
                return false;
            }
            p_input_char = temp_ptr;
            temp_ptr = NULL;

            //p_input_char = c_next_char
            result = snprintf((char *) p_input_char, CALC_STR_SIZE(new_length), "%c", c_next_char);
            if (result < 0) {
                LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
                TRACKED_FREE(p_input_char);
                p_input_char = NULL;
                return false;
            }
        }
    }

    if (result_length == (result_capacity - 1) || result_length > UINT_MAX) {
        LOG_MESSAGE(ERROR, "Error: Data to compress are too big.\n");
    }
    result_code[result_length++] = STRING_NULL_TERMINATOR;

    // Create a new array to store the compressed result
    compressed_data->length = result_length;
    compressed_data->array = (DicCodeType *) TRACKED_MALLOC(compressed_data->length * sizeof(DicCodeType));
    if (compressed_data->array == NULL) {
        LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
        TRACKED_FREE(result_code);
        result_code = NULL;
        return false;
    }

    // Copy the compressed data from result_code to compressed_data
    memcpy(compressed_data->array, result_code, compressed_data->length * sizeof(DicCodeType));

    // Clean up the original result_code array
    TRACKED_FREE(result_code);
    result_code = NULL;

    return true;
}

//region Dictionary functions
//todo Dictionary to separete file
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
//endregion

//NOT DONE___________________________________________________________________________________________________________

bool lzw_decompress(const DicCodeArray *compressed_data, DicValueArray *decompressed_data) {
    //Check if data is not null
    if (!compressed_data || !decompressed_data) {
        LOG_MESSAGE(ERROR, "Error: Data is NULL.\n");
        return false;
    }

    //Check if data is not empty
    if (compressed_data->array[0] == STRING_NULL_TERMINATOR) {
        LOG_MESSAGE(ERROR, "Error: Data is empty.\n");
        return false;
    }

    int result;
    void *temp_ptr = NULL;

    //estimate of decompressed data size
    size_t result_capacity = (size_t) ((double) compressed_data->length / LZW_COMPRESSION_RATE);

    DicValueArray result_data;
    result_data.capacity = result_capacity;
    result_data.length = 0;
    result_data.array = TRACKED_MALLOC(CALC_STR_MEM_SIZE(result_data.capacity));

    //region LZW Initialize the dictionary with single-byte entries
    Dictionary dictionary;
    bool is_succes = dictionary_init(&dictionary);
    if (!is_succes) {
        LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
        return false;
    }
    //endregion

    //region LZW OLD = first input code
    DicCodeType old_code = compressed_data->array[0];
    //endregion

    //region LZW save to result translation of OLD
    DicValueType old_value = dictionary_get_value_to_code(&dictionary, old_code);
    if (old_value == DIC_VALUE_INVALID) {
        LOG_MESSAGE(ERROR, "Error: Code not found in dictionary.\n");
        return false;
    }
    is_succes = dicvaluearray_add_element(&result_data, old_value);
    if (!is_succes) {
        LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
        return false;
    }

    //endregion

    //LZW WHILE not end of input stream

    for (size_t i = 1; i < compressed_data->length; ++i) {

        char c_value;
        DicValueType s_value;
        DicCodeType new_code;

        //region LZW NEW = next input code
        new_code = compressed_data->array[i];
        //endregion

        //region LZW IF NEW is not in the string table
        DicValueType new_value = dictionary_get_value_to_code(&dictionary, new_code);
        if (new_value == DIC_VALUE_INVALID) {

            //region LZW S = translation of OLD
            s_value = dictionary_get_value_to_code(&dictionary, old_code);
            if (s_value == DIC_VALUE_INVALID) {
                LOG_MESSAGE(ERROR, "Error: Code not found in dictionary.\n");
                return false;
            }
            //endregion

            //region LZW S = S + C
            size_t combine_sc_length = strlen((char *) s_value) + 1;

            //s_value new size
            temp_ptr = realloc(s_value, CALC_STR_MEM_SIZE(combine_sc_length));
            if (temp_ptr == NULL) {
                LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
                TRACKED_FREE(s_value);
                s_value = NULL;
                return false;
            }
            s_value = temp_ptr;
            temp_ptr = NULL;

            //s_value = s + c
            result = snprintf((char *) s_value, CALC_STR_SIZE(combine_sc_length), "%s%c", s_value, c_value);
            if (result < 0) {
                LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
                TRACKED_FREE(s_value);
                s_value = NULL;
                return false;
            }
            //endregion
        } else {
            //region LZW S = translation of NEW
            //s_value new size
            size_t new_length = strlen((char *)new_value);
            temp_ptr = realloc(s_value, CALC_STR_MEM_SIZE(new_length));
            if (temp_ptr == NULL) {
                LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
                TRACKED_FREE(s_value);
                s_value = NULL;
                return false;
            }
            s_value = temp_ptr;
            temp_ptr = NULL;

            //s_value = new_value
            result = snprintf((char *) s_value, CALC_STR_SIZE(new_length), "%s", new_value);
            if (result < 0) {
                LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
                TRACKED_FREE(s_value);
                s_value = NULL;
                return false;
            }
            //endregion
        }

        //________________________________________________________--------------

        //region LZW add OLD + first character of NEW to the string table
        DicValueType combined_value = TRACKED_MALLOC(CALC_STR_MEM_SIZE(2));
        if (!combined_value) {
            LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
            return false;
        }
        result = snprintf((char *) combined_value, CALC_STR_SIZE(2), "%s%c", old_value, new_value[0]);
        if (result < 0) {
            LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
            TRACKED_FREE(combined_value);
            return false;
        }

        DictionaryEntry entry = {dictionary.length, combined_value};
        is_succes = dictionary_add_entry(&dictionary, entry);
        if (!is_succes) {
            LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
            return false;
        }
        //endregion

        //region LZW OLD = NEW
        old_code = new_code;
        old_value = new_value;
        //endregion

    }

    //endregion


}