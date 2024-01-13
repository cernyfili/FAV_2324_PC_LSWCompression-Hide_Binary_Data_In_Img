//
// Author: Filip Cerny
// Date: 07.11.2023
// Description: File for compression and decompression of data
//

//Local includes
#include "compression.h"
#include "utils/utils.h"
#include "utils/dictionary.h"

//Lib includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//region DEFINE, MACROS
/**
 * LZW compression rate
 */
#define LZW_COMPRESSION_RATE 0.6
//endregion


//region FUNCTIONS DECLARATION
/**
 * This function takes input data and compresses it using the LZW algorithm,
 * storing the compressed result in the provided StaticDicCodeArray.
 *
 * @param data The input data to be compressed.
 * @param ptr_return_compressed_data Pointer to the StaticDicCodeArray to store the compressed data.
 * @return true if compression is successful, false otherwise.
 */
static bool lzw_compress(const char *data, StaticDicCodeArray *ptr_return_compressed_data);

/**
 * This function decompresses LZW compressed data stored in a StaticDicCodeArray,
 * producing the decompressed data in the form of a DicValueArray.
 *
 * @param compressed_data The LZW compressed data to be decompressed.
 * @param ptr_return_decompressed_data Pointer to the DicValueArray to store the decompressed data.
 * @return true if decompression is successful, false otherwise.
 */
static bool lzw_decompress(StaticDicCodeArray compressed_data, DicValueArray *ptr_return_decompressed_data);


//endregion


//region FUNCTIONS DEFINITIONS

//region PUBLIC FUNCTIONS

/**
 * This function compresses the input payload data using the LZW algorithm,
 * storing the compressed result in the provided PayloadArray.
 *
 * @param data The input payload data to be compressed.
 * @param ptr_return_compressed_data Pointer to the compressed payload data.
 * @return true if compression is successful, false otherwise.
 */
bool compress_payload(const PayloadArray data, PayloadArray *ptr_return_compressed_data) {
    //CHeck if data is not null
    if (!data.array || !ptr_return_compressed_data) {
        LOG_MESSAGE(ERROR, "Wrong argument");
        return false;
    }

    //Prepare ptr_return
    ptr_return_compressed_data->length = 0;
    ptr_return_compressed_data->capacity = 0;
    ptr_return_compressed_data->array = NULL;


    CleanupCommand *cleanup_list = NULL;

    // Convert data to char array
    char *lzw_data = TRACKED_MALLOC((STR_ADD_ONE(data.length)) * sizeof(PayloadType));
    if (!lzw_data) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        return false;
    }
    CLEANUP_ADD_COMMAND(&cleanup_list, lzw_data);
    memcpy(lzw_data, data.array, data.length * sizeof(PayloadType));
    lzw_data[data.length] = STRING_NULL_TERMINATOR;
    //endregion

    StaticDicCodeArray lzw_compressed_data;
    // Call lzw_compress function
    bool is_success = lzw_compress(lzw_data, &lzw_compressed_data);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Unable to compress payload data.");
        cleanup_run_commands(&cleanup_list);
        return false;
    }
    CLEANUP_ADD_COMMAND(&cleanup_list, lzw_compressed_data.array);

    //copy convert_compressed_data to compressed_data
    double size_diccodetype = sizeof(DicCodeType);
    double size_payloadtype = sizeof(PayloadType);
    double convert_ratio = size_diccodetype / size_payloadtype;
    ptr_return_compressed_data->capacity = (size_t) ((double) lzw_compressed_data.length * convert_ratio);
    ptr_return_compressed_data->length = ptr_return_compressed_data->capacity;

    size_t size = ptr_return_compressed_data->capacity * sizeof(PayloadType);
    ptr_return_compressed_data->array = TRACKED_MALLOC(size);
    if (!ptr_return_compressed_data->array) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        cleanup_run_commands(&cleanup_list);
        return false;
    }

    memcpy(ptr_return_compressed_data->array, lzw_compressed_data.array, ptr_return_compressed_data->length * sizeof(PayloadType));

    cleanup_run_commands(&cleanup_list);

    // Return the result of compression
    return true;
}

/**
 * This function decompresses payload data that was previously compressed using the LZW algorithm,
 * providing the decompressed data as a dynamically allocated string.
 *
 * @param compressed_data The compressed payload data to be decompressed.
 * @param ptr_return_uncompressed_data Pointer to the dynamically allocated string for the decompressed data.
 * @return true if decompression is successful, false otherwise.
 */
bool decompress_payload(const PayloadArray compressed_data, char **ptr_return_uncompressed_data) {
    //Check if data is not null
    if (!compressed_data.array || !ptr_return_uncompressed_data) {
        LOG_MESSAGE(ERROR, "Arguments are not valid");
        return false;
    }

    CleanupCommand *cleanup_list = NULL;

    // Convert PayloadArray to DicCodeArray
    StaticDicCodeArray lzw_compressed_data;
    double ratio = (double) sizeof(PayloadType) / (double) sizeof(DicCodeType);
    lzw_compressed_data.capacity = (size_t) ((double) compressed_data.length * ratio);
    lzw_compressed_data.length = lzw_compressed_data.capacity;
    lzw_compressed_data.array = TRACKED_MALLOC(lzw_compressed_data.capacity * sizeof(DicCodeType));
    if (!lzw_compressed_data.array) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        return false;
    }
    CLEANUP_ADD_COMMAND(&cleanup_list, lzw_compressed_data.array);

    memcpy(lzw_compressed_data.array, compressed_data.array, lzw_compressed_data.length * sizeof(DicCodeType));

    // Call lzw_decompress function
    DicValueArray lzw_decompressed_data;
    bool is_success = lzw_decompress(lzw_compressed_data, &lzw_decompressed_data);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Unable to decompress payload data.");
        cleanup_run_commands(&cleanup_list);
        dicvaluearray_free(&lzw_decompressed_data);
        return false;
    }

    //Convert DicValueArray to one string
    is_success = dicvaluearray_to_string(lzw_decompressed_data, ptr_return_uncompressed_data);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Unable to convert DicValueArray to string.");
        cleanup_run_commands(&cleanup_list);
        dicvaluearray_free(&lzw_decompressed_data);
        return false;
    }

    cleanup_run_commands(&cleanup_list);
    dicvaluearray_free(&lzw_decompressed_data);


    // Return the result of decompression
    return true;
}
//endregion


//region PRIVATE FUNCTIONS


/**
 * This function takes binary data and compresses it using the LZW algorithm,
 * storing the compressed result in the provided StaticDicCodeArray.
 *
 * @param data Array of unsigned char representing binary data.
 * @param ptr_return_compressed_data Pointer to the StaticDicCodeArray to store the compressed data.
 * @return true if compression is successful, false otherwise.
 */
static bool lzw_compress(const char *data, StaticDicCodeArray *ptr_return_compressed_data) {
    //Check if data is not null
    if (!data || !ptr_return_compressed_data) {
        LOG_MESSAGE(ERROR, "Wrong argument");
        return false;
    }

    //Prepare ptr_return
    ptr_return_compressed_data->length = 0;
    ptr_return_compressed_data->capacity = 0;
    ptr_return_compressed_data->array = NULL;

    bool is_success;
    CleanupCommand *cleanup_list = NULL;
    int result;
    void *temp_ptr = NULL;
    size_t data_size = strlen(data);

    //init memory for result_code
    StaticDicCodeArray result_code;//array of int
    result_code.length = 0;
    result_code.capacity = data_size;//compressed data cant be bigger than original data
    result_code.array = TRACKED_MALLOC(result_code.capacity * sizeof(DicCodeType));
    if (result_code.array == NULL) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        return false;
    }
    CLEANUP_ADD_COMMAND(&cleanup_list, result_code.array);


    //region LZW Initialize the dictionary with single-byte entries
    Dictionary dictionary;
    is_success = dictionary_init(&dictionary);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        cleanup_run_commands(&cleanup_list);
        return false;
    }

    //endregion

    DicCodeType next_dic_code = dictionary.length;//start where we ended 256

    //region LZW P_INPUT_CHAR = first input character
    DicValueType p_input_char = TRACKED_MALLOC(CALC_STR_MEM_SIZE(1));
    if (!p_input_char) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        goto error;
    }
    CLEANUP_ADD_COMMAND(&cleanup_list, p_input_char);

    result = snprintf((char *) p_input_char, STR_ADD_ONE(1), "%c", data[0]);
    if (result < 0) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        goto error;
    }
    //endregion

    //region LZW WHILE not end of input stream
    //<= data_size becouse we need to process the last char
    for (size_t i = 1; i <= data_size; ++i) {

        //region LZW C_NEXT_CHAR = next input character
        char c_next_char = data[i];
        //endregion

        bool is_last_char = false;
        //i - 1 is last character
        if (c_next_char == LZW_DECOMPRESS_LAST_CHAR) {
            is_last_char = true;
        }

        //region combined_pc = P_INPUT_CHAR + C_NEXT_CHAR
        DicValueType combined_pc;
        is_success = str_conc_strchar((char *) p_input_char, c_next_char, (char **) &combined_pc);
        if (!is_success) {
            LOG_MESSAGE(ERROR, "Memory allocation failed.");
            goto error;
        }
        CLEANUP_ADD_COMMAND(&cleanup_list, combined_pc);
        //endregion


        // Check if the combined code is present in the dictionary
        bool is_in_dictionary = false;
        is_success = dictionary_is_value_in_dictionary(dictionary, combined_pc, &is_in_dictionary);
        if (!is_success) {
            LOG_MESSAGE(ERROR, "when finding in dictionary");
            goto error;
        }

        //LZW IF P_INPUT_CHAR + C_NEXT_CHAR is in the string table
        if (!is_last_char && is_in_dictionary) {

            //region LZW P_INPUT_CHAR = P_INPUT_CHAR + C_NEXT_CHAR
            size_t new_length = strlen((char *) combined_pc);
            temp_ptr = realloc(p_input_char, CALC_STR_MEM_SIZE(new_length));
            if (temp_ptr == NULL) {
                LOG_MESSAGE(ERROR, "Memory allocation failed.");
                TRACKED_FREE(p_input_char);
                goto error;
            }
            p_input_char = temp_ptr;
            temp_ptr = NULL;

            strcpy((char *) p_input_char, (char *) combined_pc);
            //endregion
        }
        //LZW P_INPUT_CHAR + C_NEXT_CHAR not in the string table
        else {
            //region LZW save the code for INPUT_CHAR to result
            DicCodeType tmp_code;
            is_success = dictionary_get_code_to_value(dictionary, p_input_char, &tmp_code);
            if (!is_success) {
                LOG_MESSAGE(ERROR, "Code not found in dictionary.");
                goto error;
            }

            is_success = diccodearray_add_element(&result_code, tmp_code);
            if (!is_success) {
                LOG_MESSAGE(ERROR, "Memory allocation failed.");
                goto error;
            }
            //endregion

            //region LZW add P_INPUT_CHAR + C_NEXT_CHAR to the string table
            DictionaryEntry entry;
            entry.code = next_dic_code++;
            entry.value = combined_pc;

            bool is_succes = dictionary_add_entry(&dictionary, entry);
            if (!is_succes) {
                LOG_MESSAGE(ERROR, "Memory allocation failed.");
                goto error;
            }
            //endregion

            //region LZW P_INPUT_CHAR = C_NEXT_CHAR
            is_success = str_copy_char(c_next_char, (char **) &p_input_char);
            if (!is_success) {
                LOG_MESSAGE(ERROR, "Memory allocation failed.");
                goto error;
            }
            //endregion

            TRACKED_FREE(entry.value);
        }

        TRACKED_FREE(combined_pc);
    }
    //endregion

    // Create a new array to store the compressed result
    is_success = diccodearray_copy(result_code, ptr_return_compressed_data);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        goto error;
    }
    

    // Clean up the original result_code array
    cleanup_run_commands(&cleanup_list);
    dictionary_free(&dictionary);

    return true;

    error:
    cleanup_run_commands(&cleanup_list);
    dictionary_free(&dictionary);
    return false;
}


/**
 * This function decompresses LZW compressed data stored in a StaticDicCodeArray,
 * producing the decompressed data in the form of a DicValueArray.
 *
 * @param compressed_data The LZW compressed data to be decompressed.
 * @param ptr_return_decompressed_data Pointer to the DicValueArray to store the decompressed data.
 * @return true if decompression is successful, false otherwise.
 */
static bool
lzw_decompress(const StaticDicCodeArray compressed_data, DicValueArray *ptr_return_decompressed_data) {
    //Check if data is not null
    if (!compressed_data.array || !ptr_return_decompressed_data) {
        LOG_MESSAGE(ERROR, "Wrong argument");
        return false;
    }

    //Prepare ptr_return
    ptr_return_decompressed_data->length = 0;
    ptr_return_decompressed_data->capacity = 0;
    ptr_return_decompressed_data->array = NULL;

    void *temp_ptr = NULL;
    CleanupCommand *cleanup_list = NULL;

    //init memory for result_data
    size_t result_capacity_estimate = (size_t) ((double) compressed_data.length / LZW_COMPRESSION_RATE);
    DicValueArray result_data;
    result_data.capacity = result_capacity_estimate;
    result_data.length = 0;
    result_data.array = TRACKED_MALLOC(result_data.capacity * sizeof(DicValueType));
    if (!result_data.array) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        return false;
    }

    //region LZW Initialize the dictionary with single-byte entries
    Dictionary dictionary;
    bool is_succes = dictionary_init(&dictionary);
    if (!is_succes) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        goto error;
    }
    //endregion

    //region LZW OLD = first input code
    DicCodeType old_code = compressed_data.array[0];
    //endregion

    //region LZW save to result translation of OLD
    DicValueType old_value = dictionary_get_value_to_code(dictionary, old_code);
    if (is_value_invalid(old_value)) {
        LOG_MESSAGE(ERROR, "Code not found in dictionary.");
        goto error;
    }
    is_succes = dicvaluearray_add_element(&result_data, old_value);
    if (!is_succes) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        goto error;
    }
    //endregion

    char c_value = STRING_NULL_TERMINATOR;
    DicValueType s_value = NULL;

    //region LZW WHILE not end of input stream
    for (size_t i = 1; i < compressed_data.length; ++i) {

        DicCodeType new_code;

        //old_value find
        old_value = dictionary_get_value_to_code(dictionary, old_code);
        if (is_value_invalid(old_value)) {
            LOG_MESSAGE(ERROR, "Code not found in dictionary.");
            goto error;
        }

        //region LZW NEW = next input code
        new_code = compressed_data.array[i];
        //endregion

        //region LZW IF NEW is not in the string table
        bool is_in_dictionary;
        bool is_success = dictionary_is_code_in_dictionary(dictionary, new_code, &is_in_dictionary);
        if (!is_succes) {
            LOG_MESSAGE(ERROR, "Code not found in dictionary.");
            goto error;
        }

        if (!is_in_dictionary) {
            //region LZW S = translation of OLD
            size_t length = strlen((char *) old_value);
            s_value = TRACKED_MALLOC(CALC_STR_MEM_SIZE(length));
            if (s_value == NULL) {
                LOG_MESSAGE(ERROR, "Memory allocation failed.");
                goto error;
            }
            CLEANUP_ADD_COMMAND(&cleanup_list, s_value);
            strcpy((char *) s_value, (char *) old_value);
            //endregion

            //region LZW S = S + C
            if (c_value != STRING_NULL_TERMINATOR) {//is not last itaration
                char* temp_str;
                is_success = str_conc_strchar((char *) s_value, c_value, &temp_str);
                if (!is_success) {
                    LOG_MESSAGE(ERROR, "Memory allocation failed.");
                    goto error;
                }

                //s_value = temp_str
                temp_ptr = realloc(s_value, CALC_STR_MEM_SIZE(strlen(temp_str)));
                if (temp_ptr == NULL) {
                    LOG_MESSAGE(ERROR, "Memory allocation failed.");
                    TRACKED_FREE(s_value);
                    goto error;
                }
                s_value = temp_ptr;
                temp_ptr = NULL;

                strcpy((char *) s_value, temp_str);
                TRACKED_FREE(temp_str);
            }
            //endregion
        } else {
            //region LZW S = translation of NEW
            DicValueType new_value = dictionary_get_value_to_code(dictionary, new_code);
            if (is_value_invalid(new_value)) {
                LOG_MESSAGE(ERROR, "Code not found in dictionary.");
                goto error;
            }
            //s_value new size
            size_t s_value_new_length = strlen((char *) new_value);

            //s_value not allocated
            if (s_value == NULL) {
                s_value = TRACKED_MALLOC(CALC_STR_MEM_SIZE(s_value_new_length));
                if (s_value == NULL) {
                    LOG_MESSAGE(ERROR, "Memory allocation failed.");
                    goto error;
                }
                CLEANUP_ADD_COMMAND(&cleanup_list, s_value);
            } else {
                //s_value realloc
                temp_ptr = TRACKED_MALLOC(CALC_STR_MEM_SIZE(s_value_new_length));
                if (temp_ptr == NULL) {
                    LOG_MESSAGE(ERROR, "Memory allocation failed.");
                    goto error;
                }
                memcpy(temp_ptr, s_value, CALC_STR_MEM_SIZE(strlen((char *) s_value)));
                TRACKED_FREE(s_value);
                s_value = temp_ptr;
                temp_ptr = NULL;
            }

            //s_value = new_value
            strcpy((char *) s_value, (char *) new_value);
            //endregion
        }

        //region LZW save to result S
        is_succes = dicvaluearray_add_element(&result_data, s_value);
        if (!is_succes) {
            LOG_MESSAGE(ERROR, "Memory allocation failed.");
            goto error;
        }
        //endregion

        //region LZW C = first character of S
        c_value = (char) s_value[0];
        //endregion

        //region LZW OLD_value + C to the string table
        DicValueType combine_oldc;
        is_success = str_conc_strchar((char *) old_value, c_value, (char **) &combine_oldc);
        if (!is_success) {
            LOG_MESSAGE(ERROR, "Memory allocation failed.");
            goto error;
        }
        CLEANUP_ADD_COMMAND(&cleanup_list, combine_oldc);

        //save to dictionary
        DictionaryEntry entry = {dictionary.length, combine_oldc};
        is_succes = dictionary_add_entry(&dictionary, entry);
        if (!is_succes) {
            LOG_MESSAGE(ERROR, "Memory allocation failed.");
            goto error;
        }
        //endregion

        //region LZW OLD = NEW
        old_code = new_code;
        //endregion
        TRACKED_FREE(combine_oldc);
    }
    //endregion

    //Copy result_data to ptr_return_decompressed_data
    is_succes = dicvaluearray_copy(result_data, ptr_return_decompressed_data);
    if (!is_succes) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        dicvaluearray_free(ptr_return_decompressed_data);
        goto error;
    }


    dictionary_free(&dictionary);
    cleanup_run_commands(&cleanup_list);
    dicvaluearray_free(&result_data);
    return true;

    error:
    dictionary_free(&dictionary);
    dicvaluearray_free(&result_data);
    cleanup_run_commands(&cleanup_list);
    return false;
}

//endregion

//endregion