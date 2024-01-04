//
// Author: Filip Cerny
// Date: 07.11.2023
// Description:
//

//Local includes
#include "compression.h"
#include "utils/utils.h"
#include "utils/dictionary.h"

//Lib includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

//region DEFINE, MACROS
#define LZW_COMPRESSION_RATE 0.6
#define STRING_NULL_TERMINATOR '\0'
//endregion


//region STRUCTS


//endregion





//region FUNCTIONS DECLARATION
static bool lzw_compress(const char *data, DicCodeArray *compressed_data);

static bool lzw_decompress(DicCodeArray compressed_data, DicValueArray *ptr_decompressed_data);

//endregion


//region FUNCTIONS DEFINITIONS

//region PUBLIC FUNCTIONS
bool compress_payload(const PayloadArray data, PayloadArray *ptr_compressed_data){
    void* temp_ptr;

    // Convert data to char array
    char* lzw_data = (char*)data.array;
    //region Add null terminator
    size_t new_length = data.length;//char is size of one
    temp_ptr = realloc(lzw_data, CALC_STR_MEM_SIZE(new_length));
    if (temp_ptr == NULL) {
        LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
        TRACKED_FREE(lzw_data);
        return false;
    }
    lzw_data = temp_ptr;
    temp_ptr = NULL;
    lzw_data[data.length] = STRING_NULL_TERMINATOR;
    //endregion

    DicCodeArray lzw_compressed_data;

    // Call lzw_compress function
    bool is_success = lzw_compress(lzw_data, &lzw_compressed_data);

    if (!is_success) {
        LOG_MESSAGE(ERROR, "Error: Unable to compress payload data.");
        return false;
    }

    //copy convert_compressed_data to compressed_data
    ptr_compressed_data->length = (size_t)((double)lzw_compressed_data.length * (sizeof(DicCodeType) / (double) sizeof(PayloadType)));

    size_t size = ptr_compressed_data->length * sizeof(PayloadType);
    ptr_compressed_data->array = TRACKED_MALLOC(size);
    if (!ptr_compressed_data->array) {
        LOG_MESSAGE(ERROR, "Error: Memory allocation failed.");
        return false;
    }

    memcpy(ptr_compressed_data->array, lzw_compressed_data.array, size);

    TRACKED_FREE(lzw_compressed_data.array);


    // Return the result of compression
    return true;
}

bool decompress_payload(const PayloadArray compressed_data, PayloadArray *ptr_uncompressed_data){

    // Convert compressed_data to DicCodeArray
    DicCodeArray lzw_compressed_data;
    lzw_compressed_data.length = (size_t)((double)compressed_data.length * (sizeof(PayloadType) / (double) sizeof(DicCodeType)));
    lzw_compressed_data.array = (DicCodeType *) compressed_data.array;

    // Call lzw_decompress function
    DicValueArray lzw_decompressed_data;
    bool is_success = lzw_decompress(lzw_compressed_data, &lzw_decompressed_data);

    if (!is_success) {
        LOG_MESSAGE(ERROR, "Error: Unable to decompress payload data.");
        return false;
    }

    // Convert lzw_decompressed_data to PayloadArray
    ptr_uncompressed_data->length =(size_t) ((double)lzw_decompressed_data.length * (sizeof(DicValueType) / (double)sizeof(PayloadType)));
    ptr_uncompressed_data->array = TRACKED_MALLOC(ptr_uncompressed_data->length * sizeof(PayloadType));
    if (!ptr_uncompressed_data->array) {
        LOG_MESSAGE(ERROR, "Error: Memory allocation failed.");
        return false;
    }

    memcpy(ptr_uncompressed_data->array, lzw_decompressed_data.array, ptr_uncompressed_data->length * sizeof(PayloadType));

    TRACKED_FREE(lzw_decompressed_data.array);

    // Return the result of decompression
    return true;
}
//endregion


//region PRIVATE FUNCTIONS

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
static bool lzw_compress(const char *data, DicCodeArray *compressed_data) {
    if (data == NULL) {
        LOG_MESSAGE(ERROR, "Error: Data is NULL.\n");
        return false;
    }

    size_t data_size = strlen(data);
    DicCodeType *result_code;//array of int
    size_t result_length = 0;
    size_t result_capacity = data_size;//compressed data cant be bigger than original data

    int result;
    void *temp_ptr = NULL;


    result_code = TRACKED_MALLOC(result_capacity * sizeof(DicCodeType));
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
    DicCodeType next_dic_code = dictionary.length;//start where we ended 256

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
        if (is_code_invalid(code)) {

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
            if (is_code_invalid(tmp_code)) {
                LOG_MESSAGE(ERROR, "Error: Code not found in dictionary.\n");
                return false;
            }
            result_code[result_length++] = tmp_code;

            //LZW add P_INPUT_CHAR + C_NEXT_CHAR to the string table
            DictionaryEntry entry = {next_dic_code++, combined_pc};
            bool is_succes = dictionary_add_entry(&dictionary, entry);
            if (!is_succes) {
                LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
                return false;
            }

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
    compressed_data->array = TRACKED_MALLOC(compressed_data->length * sizeof(DicCodeType));
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

static bool lzw_decompress(const DicCodeArray compressed_data, DicValueArray *ptr_decompressed_data) {
    //Check if data is not null
    if (!compressed_data.array || !ptr_decompressed_data->array) {
        LOG_MESSAGE(ERROR, "Error: Data is NULL.\n");
        return false;
    }

    //Check if data is not empty
    if (compressed_data.array[0] == STRING_NULL_TERMINATOR) {
        LOG_MESSAGE(ERROR, "Error: Data is empty.\n");
        return false;
    }

    int result;
    void *temp_ptr = NULL;

    //estimate of decompressed data size
    size_t result_capacity = (size_t) ((double) compressed_data.length / LZW_COMPRESSION_RATE);

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
    DicCodeType old_code = compressed_data.array[0];
    //endregion

    //region LZW save to result translation of OLD
    DicValueType old_value = dictionary_get_value_to_code(&dictionary, old_code);
    if (is_value_invalid(old_value)) {
        LOG_MESSAGE(ERROR, "Error: Code not found in dictionary.\n");
        return false;
    }
    is_succes = dicvaluearray_add_element(&result_data, old_value);
    if (!is_succes) {
        LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
        return false;
    }

    //endregion
    char c_value = STRING_NULL_TERMINATOR;
    DicValueType s_value = NULL;

    //LZW WHILE not end of input stream

    for (size_t i = 1; i < compressed_data.length; ++i) {

        DicCodeType new_code;

        //old_value find
        old_value = dictionary_get_value_to_code(&dictionary, old_code);
        if (is_value_invalid(old_value)) {
            LOG_MESSAGE(ERROR, "Error: Code not found in dictionary.\n");
            return false;
        }

        //region LZW NEW = next input code
        new_code = compressed_data.array[i];
        //endregion

        //region LZW IF NEW is not in the string table
        DicValueType new_value = dictionary_get_value_to_code(&dictionary, new_code);
        if (is_value_invalid(new_value)) {

            //region LZW S = translation of OLD
            size_t length = strlen((char *) old_value);
            s_value = TRACKED_MALLOC(CALC_STR_MEM_SIZE(length));
            if (s_value == NULL) {
                LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
                TRACKED_FREE(s_value);
                s_value = NULL;
                return false;
            }
            strcpy((char *)s_value, (char *)old_value);
            //endregion

            //region LZW S = S + C
            if (c_value != STRING_NULL_TERMINATOR) {
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
            }
            //endregion
        } else {
            //region LZW S = translation of NEW
            //s_value new size
            size_t new_length = strlen((char *)new_value);

            if(s_value == NULL){
                s_value = TRACKED_MALLOC(CALC_STR_MEM_SIZE(new_length));
                if (s_value == NULL) {
                    LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
                    TRACKED_FREE(s_value);
                    return false;
                }
            } else {
                temp_ptr = realloc(s_value, CALC_STR_MEM_SIZE(new_length));
                if (temp_ptr == NULL) {
                    LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
                    TRACKED_FREE(s_value);
                    s_value = NULL;
                    return false;
                }
                s_value = temp_ptr;
                temp_ptr = NULL;
            }

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

        //region LZW save to result S
        is_succes = dicvaluearray_add_element(&result_data, s_value);
        if (!is_succes) {
            LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
            return false;
        }
        //endregion

        //region LZW C = first character of S
        c_value = (char) s_value[0];
        //endregion

        //region LZW OLD_value + C to the string table
        size_t combine_oldc_length = strlen((char *) old_value) + 1;

        //var new size
        DicValueType combine_oldc = TRACKED_MALLOC(CALC_STR_MEM_SIZE(combine_oldc_length));
        if (combine_oldc == NULL) {
            LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
            TRACKED_FREE(combine_oldc);
            combine_oldc = NULL;
            return false;
        }

        //var = old + c
        result = snprintf((char *) combine_oldc, CALC_STR_SIZE(combine_oldc_length), "%s%c", old_value, c_value);
        if (result < 0) {
            LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
            TRACKED_FREE(combine_oldc);
            combine_oldc = NULL;
            return false;
        }

        //save to dictionary
        DictionaryEntry entry = {dictionary.length, combine_oldc};
        is_succes = dictionary_add_entry(&dictionary, entry);
        if (!is_succes) {
            LOG_MESSAGE(ERROR, "Error: Memory allocation failed.\n");
            return false;
        }

        //endregion

        //region LZW OLD = NEW
        //endregion

        //region LZW OLD = NEW
        old_code = new_code;
        //endregion

    }

    //endregion

    return true;

}
//endregion

//endregion