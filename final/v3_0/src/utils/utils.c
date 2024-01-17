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
//endregion

//endregion