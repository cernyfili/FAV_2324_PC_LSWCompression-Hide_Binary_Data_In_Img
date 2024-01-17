//
// Author: Filip Cerny
// Date: 07.11.2023
// Description: File for compression and decompression of data
//

//Local includes
#include "compression.h"
#include "utils/utils.h"
#include "utils/dictionary.h"
#include "utils/trie.h"

//Lib includes
#include <stdio.h>
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
 * storing the compressed result in the provided struct staticdiccodearray.
 *
 * @param data The input data to be compressed.
 * @param ptr_return_compressed_data Pointer to the struct staticdiccodearray to store the compressed data.
 * @return true if compression is successful, false otherwise.
 */
static bool lzw_compress(const char *data, struct staticdiccodearray *ptr_return_compressed_data);

/**
 * This function decompresses LZW compressed data stored in a struct staticdiccodearray,
 * producing the decompressed data in the form of a struct dicvaluearray.
 *
 * @param compressed_data The LZW compressed data to be decompressed.
 * @param ptr_return_decompressed_data Pointer to the struct dicvaluearray to store the decompressed data.
 * @return true if decompression is successful, false otherwise.
 */
static bool
lzw_decompress(struct staticdiccodearray compressed_data, struct dicvaluearray *ptr_return_decompressed_data);


//endregion


//region FUNCTIONS DEFINITIONS

//region PUBLIC FUNCTIONS

/**
 * This function compresses the input payload data using the LZW algorithm,
 * storing the compressed result in the provided struct payloadarray.
 *
 * @param data The input payload data to be compressed.
 * @param ptr_return_compressed_data Pointer to the compressed payload data.
 * @return true if compression is successful, false otherwise.
 */
bool compress_payload(const struct binarydataarray data, struct binarydataarray *ptr_return_compressed_data) {

    //CHeck if data is not null
    if (!data.array || !ptr_return_compressed_data) {
        LOG_MESSAGE(ERROR, "Wrong argument");
        return false;
    }

    //Prepare ptr_return
    ptr_return_compressed_data->length = 0;
    ptr_return_compressed_data->capacity = 0;
    ptr_return_compressed_data->array = NULL;


    struct cleanupcommand *cleanup_list = NULL;

    // Convert data to char array
    char *lzw_data = TRACKED_MALLOC((STR_ADD_ONE(data.length)) * sizeof(binarydata_type));
    if (!lzw_data) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        return false;
    }
    CLEANUP_ADD_COMMAND(&cleanup_list, lzw_data);
    memcpy(lzw_data, data.array, data.length * sizeof(binarydata_type));
    lzw_data[data.length] = STRING_NULL_TERMINATOR;
    //endregion

    struct staticdiccodearray lzw_compressed_data;
    // Call lzw_compress function
    bool is_success = lzw_compress(lzw_data, &lzw_compressed_data);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Unable to compress payload data.");
        cleanup_run_commands(&cleanup_list);
        return false;
    }
    CLEANUP_ADD_COMMAND(&cleanup_list, lzw_compressed_data.array);

    //copy convert_compressed_data to compressed_data
    double size_diccodetype = sizeof(dic_code_type);
    double size_payloadtype = sizeof(binarydata_type);
    double convert_ratio = size_diccodetype / size_payloadtype;
    ptr_return_compressed_data->capacity = (size_t) ((double) lzw_compressed_data.length * convert_ratio);
    ptr_return_compressed_data->length = ptr_return_compressed_data->capacity;

    size_t size = ptr_return_compressed_data->capacity * sizeof(binarydata_type);
    ptr_return_compressed_data->array = TRACKED_MALLOC(size);
    if (!ptr_return_compressed_data->array) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        cleanup_run_commands(&cleanup_list);
        return false;
    }

    memcpy(ptr_return_compressed_data->array, lzw_compressed_data.array,
           ptr_return_compressed_data->length * sizeof(binarydata_type));

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
bool decompress_payload(const struct binarydataarray compressed_data, char **ptr_return_uncompressed_data) {
    //Check if data is not null
    if (!compressed_data.array || !ptr_return_uncompressed_data) {
        LOG_MESSAGE(ERROR, "Arguments are not valid");
        return false;
    }

    struct cleanupcommand *cleanup_list = NULL;

    // Convert struct payloadarray to DicCodeArray
    struct staticdiccodearray lzw_compressed_data;
    double ratio = (double) sizeof(binarydata_type) / (double) sizeof(dic_code_type);
    lzw_compressed_data.capacity = (size_t) ((double) compressed_data.length * ratio);
    lzw_compressed_data.length = lzw_compressed_data.capacity;
    lzw_compressed_data.array = TRACKED_MALLOC(lzw_compressed_data.capacity * sizeof(dic_code_type));
    if (!lzw_compressed_data.array) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        cleanup_run_commands(&cleanup_list);
        return false;
    }
    CLEANUP_ADD_COMMAND(&cleanup_list, lzw_compressed_data.array);

    memcpy(lzw_compressed_data.array, compressed_data.array, lzw_compressed_data.length * sizeof(dic_code_type));

    // Call lzw_decompress function
    struct dicvaluearray lzw_decompressed_data;
    bool is_success = lzw_decompress(lzw_compressed_data, &lzw_decompressed_data);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Unable to decompress payload data.");
        cleanup_run_commands(&cleanup_list);
        dicvaluearray_free(&lzw_decompressed_data);
        return false;
    }

    //Convert struct dicvaluearray to one string
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
 * storing the compressed result in the provided struct staticdiccodearray.
 *
 * @param data Array of unsigned char representing binary data.
 * @param ptr_return_compressed_data Pointer to the struct staticdiccodearray to store the compressed data.
 * @return true if compression is successful, false otherwise.
 */
static bool lzw_compress(const char *data, struct staticdiccodearray *ptr_return_compressed_data) {
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
    struct cleanupcommand *cleanup_list = NULL;
    size_t data_size = strlen(data);

    LOG_MESSAGE(INFO, "lzw_compress: Uncompressed data size: %zu", data_size);

    //init memory for result_code
    struct staticdiccodearray result_code;//array of int
    result_code.length = 0;
    result_code.capacity = data_size;//compressed data cant be bigger than original data
    result_code.array = TRACKED_MALLOC(result_code.capacity * sizeof(dic_code_type));
    if (result_code.array == NULL) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        return false;
    }
    CLEANUP_ADD_COMMAND(&cleanup_list, result_code.array);


    //region LZW Initialize the dictionary with single-byte entries
    struct trienode *ptr_dictionary = trie_create_node();
    if (!ptr_dictionary) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        cleanup_run_commands(&cleanup_list);
        return false;
    }

    int current_code = 0;
    for (int i = 0; i < CHILDREN_SIZE; i++)
    {
        char current_value = (char) i;
        current_code = (dic_code_type) i;
        is_success = trie_insert_char(ptr_dictionary, current_value, current_code);
        if (!is_success) {
            LOG_MESSAGE(ERROR, "Memory allocation failed.");
            cleanup_run_commands(&cleanup_list);
            trie_free(ptr_dictionary);
            return false;
        }
    }
    current_code++;//next code

    //endregion

    //region LZW P_INPUT_CHAR = first input character
    char first_char = data[0];

    struct trienode *ptr_p_trie;
    is_success = trie_search_char(ptr_dictionary, first_char, &ptr_p_trie);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        cleanup_run_commands(&cleanup_list);
        trie_free(ptr_dictionary);
        return false;
    }
    //endregion

    //region LZW WHILE not end of input stream
    //<= data_size becouse we need to process the last char
    for (size_t i = 1; i <= data_size; i++) {
        if (i > data_size) {
            LOG_MESSAGE(ERROR, "lzw_compress: i > data_size");
        }
        /*LOG_MESSAGE(INFO, "lzw_compress: Processing character %zu from %zu", i, data_size);*/

        //region LZW C_NEXT_CHAR = next input character
        char c_next_char = data[i];
        //endregion

        bool is_last_char = false;
        //i - 1 is last character
        if (c_next_char == LZW_DECOMPRESS_LAST_CHAR) {
            is_last_char = true;
        }

        // Check if the combined code is present in the dictionary
        struct trienode *ptr_combined_pc_trie;
        bool is_in_dictionary = trie_search_char(ptr_p_trie, c_next_char, &ptr_combined_pc_trie);

        //LZW IF P_INPUT_CHAR + C_NEXT_CHAR is in the string table
        if (!is_last_char && is_in_dictionary) {
            //region LZW P_INPUT_CHAR = P_INPUT_CHAR + C_NEXT_CHAR
            ptr_p_trie = ptr_combined_pc_trie;
            //endregion
        }
            //LZW P_INPUT_CHAR + C_NEXT_CHAR not in the string table
        else {
            //region LZW save the code for P_INPUT_CHAR to result
            is_success = diccodearray_add_element(&result_code, ptr_p_trie->value);
            if (!is_success) {
                LOG_MESSAGE(ERROR, "Memory allocation failed.");
                cleanup_run_commands(&cleanup_list);
                trie_free(ptr_dictionary);
                return false;
            }
            //endregion

            //region LZW add P_INPUT_CHAR + C_NEXT_CHAR to the string table
            is_success = trie_insert_char(ptr_p_trie, c_next_char, current_code++);
            if (!is_success) {
                LOG_MESSAGE(ERROR, "Memory allocation failed.");
                cleanup_run_commands(&cleanup_list);
                trie_free(ptr_dictionary);
                return false;
            }
            //endregion

            //region LZW P_INPUT_CHAR = C_NEXT_CHAR
            is_success = trie_search_char(ptr_dictionary, c_next_char, &ptr_p_trie);
            if (!is_success) {
                LOG_MESSAGE(ERROR, "Memory allocation failed.");
                cleanup_run_commands(&cleanup_list);
                trie_free(ptr_dictionary);
                return false;
            }

            //endregion
        }
    }
    //endregion

    // Create a new array to store the compressed result
    is_success = diccodearray_copy(result_code, ptr_return_compressed_data);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        cleanup_run_commands(&cleanup_list);
        trie_free(ptr_dictionary);
        return false;
    }

    // Clean up the original result_code array
    cleanup_run_commands(&cleanup_list);
    trie_free(ptr_dictionary);

    return true;
}


/**
 * This function decompresses LZW compressed data stored in a struct staticdiccodearray,
 * producing the decompressed data in the form of a struct dicvaluearray.
 *
 * @param compressed_data The LZW compressed data to be decompressed.
 * @param ptr_return_decompressed_data Pointer to the struct dicvaluearray to store the decompressed data.
 * @return true if decompression is successful, false otherwise.
 */
static bool
lzw_decompress(const struct staticdiccodearray compressed_data, struct dicvaluearray *ptr_return_decompressed_data) {
    //Check if data is not null
    if (!compressed_data.array || !ptr_return_decompressed_data) {
        LOG_MESSAGE(ERROR, "Wrong argument");
        return false;
    }

    //Prepare ptr_return
    ptr_return_decompressed_data->length = 0;
    ptr_return_decompressed_data->capacity = 0;
    ptr_return_decompressed_data->array = NULL;

    //init memory for result_data
    size_t result_capacity_estimate = (size_t) ((double) compressed_data.length / LZW_COMPRESSION_RATE);

    (*ptr_return_decompressed_data).capacity = result_capacity_estimate;
    (*ptr_return_decompressed_data).length = 0;
    (*ptr_return_decompressed_data).array = TRACKED_MALLOC((*ptr_return_decompressed_data).capacity * sizeof(dic_value_type));
    if (!(*ptr_return_decompressed_data).array) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        return false;
    }

    //region LZW Initialize the dictionary with single-byte entries
    struct dictionary dictionary;
    bool is_success = dictionary_init(&dictionary);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        dictionary_free(&dictionary);
        dicvaluearray_free(ptr_return_decompressed_data);
        return false;
    }
    //endregion

    //region LZW OLD = first input code
    dic_code_type old_code = compressed_data.array[0];
    //endregion

    //region LZW save to result translation of OLD
    dic_value_type old_value = dictionary_get_value_to_code(dictionary, old_code);
    if (is_value_invalid(old_value)) {
        LOG_MESSAGE(ERROR, "Code not found in dictionary.");
        dictionary_free(&dictionary);
        dicvaluearray_free(ptr_return_decompressed_data);
        return false;
    }
    is_success = dicvaluearray_add_element(ptr_return_decompressed_data, old_value);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        dictionary_free(&dictionary);
        dicvaluearray_free(ptr_return_decompressed_data);
        return false;
    }
    //endregion

    char c_value = STRING_NULL_TERMINATOR;

    size_t s_value_len = 0;
    //max length set becouse of speed (should be dynamic becouse we don't know final size of word)
    size_t s_value_capacity = MAX_VALUE_LENGTH;
    char s_value[MAX_VALUE_LENGTH];

    //region LZW WHILE not end of input stream
    for (size_t i = 1; i < compressed_data.length; i++) {
        /*LOG_MESSAGE(INFO, "lzw_decompress: Processing character %zu from %zu", i, compressed_data.length);*/
        /*printf("%zu from %zu\n", i, compressed_data.length);*/

        dic_code_type new_code;

        //old_value find
        old_value = dictionary_get_value_to_code(dictionary, old_code);
        if (is_value_invalid(old_value)) {
            LOG_MESSAGE(ERROR, "Code not found in dictionary.");
            dictionary_free(&dictionary);
            dicvaluearray_free(ptr_return_decompressed_data);
            return false;
        }
        size_t old_value_len = strlen((char *) old_value);

        //region LZW NEW = next input code
        new_code = compressed_data.array[i];
        //endregion

        //region LZW IF NEW is not in the string table
        bool is_in_dictionary;
        is_success = dictionary_is_code_in_dictionary(dictionary, new_code, &is_in_dictionary);
        if (!is_success) {
            LOG_MESSAGE(ERROR, "Code not found in dictionary.");
            dictionary_free(&dictionary);
            dicvaluearray_free(ptr_return_decompressed_data);
            return false;
        }

        if (!is_in_dictionary) {
            //region LZW S = translation of OLD
            size_t length = STR_ADD_ONE(old_value_len);
            if (length >= s_value_capacity) {
                LOG_MESSAGE(ERROR, "Value is too long.");
                dictionary_free(&dictionary);
                dicvaluearray_free(ptr_return_decompressed_data);
                return false;
            }
            memcpy(s_value, old_value, length * sizeof(char));
            s_value_len = length;
            //endregion

            //region LZW S = S + C
            if (c_value != STRING_NULL_TERMINATOR) {//is not last iteration
                s_value[s_value_len - 1] = c_value;
                if (s_value_len >= s_value_capacity) {
                    LOG_MESSAGE(ERROR, "Value is too long.");
                    dictionary_free(&dictionary);
                    dicvaluearray_free(ptr_return_decompressed_data);
                    return false;
                }
                s_value[s_value_len] = STRING_NULL_TERMINATOR;
                s_value_len++;
            }

            //endregion
        } else {
            //region LZW S = translation of NEW
            dic_value_type new_value = dictionary_get_value_to_code(dictionary, new_code);
            if (is_value_invalid(new_value)) {
                LOG_MESSAGE(ERROR, "Code not found in dictionary.");
                dictionary_free(&dictionary);
                dicvaluearray_free(ptr_return_decompressed_data);
                return false;
            }
            size_t new_value_len = strlen((char *) new_value);
            size_t length = STR_ADD_ONE(new_value_len);
            if (length >= s_value_capacity) {
                LOG_MESSAGE(ERROR, "Value is too long.");
                dictionary_free(&dictionary);
                dicvaluearray_free(ptr_return_decompressed_data);
                return false;
            }
            memcpy(s_value, new_value, length * sizeof(char));
            s_value_len = length;

            //endregion
        }

        //region LZW save to result S
        is_success = dicvaluearray_add_element(ptr_return_decompressed_data, (dic_value_type) s_value);
        if (!is_success) {
            LOG_MESSAGE(ERROR, "Memory allocation failed.");
            dictionary_free(&dictionary);
            dicvaluearray_free(ptr_return_decompressed_data);
            return false;
        }
        //endregion

        //region LZW C = first character of S
        c_value = s_value[0];
        //endregion

        //region LZW OLD_value + C to the string table
        char combined_value[MAX_VALUE_LENGTH];
        size_t combined_value_len = 0;
        size_t combined_value_capacity = MAX_VALUE_LENGTH;
        size_t length = STR_ADD_ONE(old_value_len + sizeof(char));
        if (length >= combined_value_capacity) {
            LOG_MESSAGE(ERROR, "Value is too long.");
            dictionary_free(&dictionary);
            dicvaluearray_free(ptr_return_decompressed_data);
            return false;
        }
        memcpy(combined_value, old_value, old_value_len * sizeof(char));
        combined_value_len = old_value_len;
        combined_value[combined_value_len] = c_value;
        combined_value_len++;
        combined_value[combined_value_len] = STRING_NULL_TERMINATOR;


        //save to dictionary
        dic_value_type entry = (dic_value_type)combined_value;
        is_success = dictionary_add_entry(&dictionary, entry);
        if (!is_success) {
            LOG_MESSAGE(ERROR, "Memory allocation failed.");
            dictionary_free(&dictionary);
            dicvaluearray_free(ptr_return_decompressed_data);
            return false;
        }
        //endregion

        //region LZW OLD = NEW
        old_code = new_code;
        //endregion
    }
    printf("compression.c after compression");
    //endregion

    //Copy result_data to ptr_return_decompressed_data
    /*is_success = dicvaluearray_copy(result_data, ptr_return_decompressed_data);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        dicvaluearray_free(ptr_return_decompressed_data);
        dictionary_free(&dictionary);
        dicvaluearray_free(ptr_return_decompressed_data);
        return false;
    }*/

    dictionary_free(&dictionary);
    return true;
}

//endregion

//endregion