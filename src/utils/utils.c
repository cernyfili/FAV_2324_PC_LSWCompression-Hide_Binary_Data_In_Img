//
// Author: Filip Cerny
// Date: 06.01.2024
// Description: This file contains functions for copying files and checking if file exists. and is imports headers
//


//Local includes
#include "utils.h"

//Lib includes
#include <sys/stat.h>

//region DEFINE, MACROS

/**
 * Buffer size for copying files
 */
#define BUFFER_SIZE 4096
//endregion

//region FUNCTIONS DEFINITIONS

//region PUBLIC FUNCTIONS
/**
 * Copy file from input_file to output_file
 * @param input_file  is file from which we copy
 * @param output_file  is file to which we copy
 * @return  true if copy was successful, false otherwise
 */
bool copy_files(FILE *input_file, FILE *output_file) {
    // Check if the file was opened successfully
    if (!input_file || !output_file) {
        // Unable to open file
        LOG_MESSAGE(ERROR, "Unable to open file.");

        return false;
    }
    long input_pos = ftell(input_file);
    long output_pos = ftell(output_file);

    // Copy the contents from the input file to the output file
    fseek(input_file, 0, SEEK_SET);
    fseek(output_file, 0, SEEK_SET);

    char buffer[BUFFER_SIZE];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), input_file)) > 0) {
        fwrite(buffer, 1, bytes_read, output_file);
    }

    fseek(input_file, input_pos, SEEK_SET);
    fseek(output_file, output_pos, SEEK_SET);

    return true; // Return success
}

/**
 * Check if file exists
 * @param filename  is name of file
 * @return  true if file exists, false otherwise
 */
bool file_exists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

/**
 * Copy char to string
 * @param copy_value  is char which we copy
 * @param ptr_return_str  is pointer to string where we copy
 * @return  true if copy was successful, false otherwise
 */
bool str_copy_char(char copy_value, char ** ptr_return_str){
    if (!ptr_return_str) {
        LOG_MESSAGE(ERROR, "Wrong argument");
        return false;
    }

    size_t new_length = sizeof(copy_value);//char is size of one
    char * temp_ptr = TRACKED_MALLOC(CALC_STR_MEM_SIZE(new_length));
    if (temp_ptr == NULL) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        return false;
    }
    memcpy(temp_ptr, *ptr_return_str, CALC_STR_MEM_SIZE(strlen((char *) *ptr_return_str)));
    TRACKED_FREE(*ptr_return_str);
    *ptr_return_str = temp_ptr;
    temp_ptr = NULL;

    int result = snprintf((char *) *ptr_return_str, STR_ADD_ONE(new_length), "%c", copy_value);
    if (result < 0) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        TRACKED_FREE(*ptr_return_str);
        return false;
    }

    return true;
}

/**
 * Concatenate string and char
 * @param str_value  is string which we concatenate
 * @param char_value  is char which we concatenate
 * @param ptr_return_str  is pointer to string where we concatenate
 * @return  true if concatenate was successful, false otherwise
 */
bool str_conc_strchar(char* str_value, char char_value, char** ptr_return_str){
    size_t length = strlen(str_value) + sizeof(char);
    (*ptr_return_str) = TRACKED_MALLOC(CALC_STR_MEM_SIZE(length));
    if (!(*ptr_return_str)) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        return false;
    }

    int result = snprintf((char *) (*ptr_return_str), STR_ADD_ONE(length), "%s%c", str_value, char_value);
    if (result < 0) {
        LOG_MESSAGE(ERROR, "Memory allocation failed.");
        TRACKED_FREE(*ptr_return_str);
        return false;
    }

    return true;
}
//endregion

//endregion