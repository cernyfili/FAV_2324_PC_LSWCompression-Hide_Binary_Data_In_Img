//
// Author: Lenovo
// Date: 07.11.2023
// Description: Main file for project
//

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <string.h>
#include <stdbool.h>
#include "payload.h"
#include "utils/utils.h"
#include "image.h"

#define ARGUMENT_COUNT 4

#define HIDE_DATA_FLAG "-h"

#define EXTRACT_DATA_FLAG "-x"

//todo write

//region FUNCTIONS DECLARATION

#define BUFFER_SIZE 1024

static int
hide_payload(const char *input_image_filepath, const char *hide_payload_filepath);

static int extract_payload(const char *input_image_filepath, const char *output_payload_filepath);

void clenup();

//endregion


//region FUNCTIONS DEFINITIONS

/**
 * @brief main function
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[]) {
    atexit(clenup);

    init_logger();

    /*ERROR wrong number of arguments -> user manual */
    //todo user manual
    if (argc != ARGUMENT_COUNT) {
        LOG_MESSAGE(ERROR, "Wrong number of arguments - needed 3 arguments");
        exit(1);
    }

    bool is_success;
    int result;

    //arguments to variable
    char *direction_flag = argv[2];

    //flag to hide data
    if (strcmp(direction_flag, HIDE_DATA_FLAG) == 0) {
        char *input_image_filepath = argv[1];
        char *hide_payload_filepath = argv[3];

        // Hide the payload into the image
        result = hide_payload(input_image_filepath, hide_payload_filepath);
        if (result == 0) {
            printf("Hiding completed.\n");
        } else {
            LOG_MESSAGE(ERROR, "Error: Unable to hide the payload into the image.");
            exit(result);
        }
    //flag to extract data
    } else if (strcmp(direction_flag, EXTRACT_DATA_FLAG) == 0) {
        char *input_image_filepath = argv[1];
        char *output_payload_filepath = argv[3];

        // Extract the payload from the image
        result = extract_payload(input_image_filepath, output_payload_filepath);
        if (result == 0) {
            printf("Extraction completed.\n");
        } else {
            LOG_MESSAGE(ERROR, "Error: Unable to extract the payload from the image.");
            exit(result);
        }
    } else {
        LOG_MESSAGE(ERROR, "Invalid direction flag. Use -h for hiding or -x for extraction.");
        exit(4);
    }

    exit(0);
}

/**
 * @brief hide payload
 * @param input_image_filepath
 * @param hide_payload_filepath
 * @return
 * 0 succesfull
 * 2 if input file is not BMP or PNG
 * 3 if payload is too big for image
 * 6 other error
 */
static int
hide_payload(const char *input_image_filepath, const char *hide_payload_filepath) {
    //Check if the arguments are valid
    if (!input_image_filepath || !hide_payload_filepath) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return 6;
    }

    //output_image_filepath = "temp_" + input_image_filepath
    char *ptr_filename_start = strrchr(input_image_filepath, '/') + 1;
    if (!ptr_filename_start) {
        ptr_filename_start = strrchr(input_image_filepath, '\\');
        if (!ptr_filename_start) {
            LOG_MESSAGE(ERROR, "Error: Unable to get the filename from the input image filepath.");
            return 6;
        }
    }
    char *prefix_str = "temp_";
    char *temp_output_image_filepath = TRACKED_MALLOC(CALC_STR_MEM_SIZE((strlen(prefix_str) + strlen(input_image_filepath))));
    if (!temp_output_image_filepath) {
        LOG_MESSAGE(ERROR, "Error: Unable to allocate memory.");
        return 6;
    }
    memcpy(temp_output_image_filepath, input_image_filepath, strlen(input_image_filepath));
    size_t shift = ptr_filename_start - input_image_filepath;
    memcpy(temp_output_image_filepath + shift, prefix_str, strlen(prefix_str));
    memcpy(temp_output_image_filepath + shift + strlen(prefix_str), ptr_filename_start, strlen(ptr_filename_start));


    bool is_success;
    int result;

    PayloadArray hide_data;
    is_success = prepare_payload_data(hide_payload_filepath, &hide_data);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Error: Unable to prepare the payload data.");
        return 6;
    }

    // Hide the payload into the image
    result = hide_data_lsb(input_image_filepath, hide_data, temp_output_image_filepath);

    //Check if the hiding was successful
    if (result != 0) {
        LOG_MESSAGE(ERROR, "Error: Unable to hide the payload into the image.");
        return result;
    }

    //Copy the temp file to the original file
    FILE *temp_file = fopen(temp_output_image_filepath, "rb");
    FILE *original_file = fopen(input_image_filepath, "wb");

    if (!temp_file || !original_file) {
        LOG_MESSAGE(ERROR, "Error: Unable to open files for copying.");
        TRACKED_FREE(temp_output_image_filepath);
        TRACKED_FREE(hide_data.array);
        return 6;
    }

    char buffer[BUFFER_SIZE];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), temp_file)) > 0) {
        fwrite(buffer, 1, bytesRead, original_file);
    }

    //delete temp file
    is_success = remove(temp_output_image_filepath);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Error: Unable to delete the temp file.");
        TRACKED_FREE(temp_output_image_filepath);
        TRACKED_FREE(hide_data.array);
        return 6;
    }

    fclose(temp_file);
    fclose(original_file);


    TRACKED_FREE(temp_output_image_filepath);
    TRACKED_FREE(hide_data.array);

    return 0;
}

/**
 * @brief extract payload
 * @param input_image_filepath
 * @param output_payload_filepath
 * @return
 */
static int extract_payload(const char *input_image_filepath, const char *output_payload_filepath) {
    //Check if the arguments are valid
    if (!input_image_filepath || !output_payload_filepath) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return false;//todo change
    }

    bool is_success;
    int result;

    // Extract the payload from the image
    PayloadArray hidden_data;
    result = extract_data_lsb(input_image_filepath, &hidden_data);

    //Check if the extraction was successful
    if (result != 0) {
        LOG_MESSAGE(ERROR, "Error: Unable to extract the payload from the image.");
        return result;
    }

    PayloadArray payload_data;
    result = extract_payload_from_data(hidden_data, &payload_data);
    if (result != 0) {
        LOG_MESSAGE(ERROR, "Error: Unable to get the payload from the array.");
        return result;
    }

    //save payload to file
    FILE *file = fopen(output_payload_filepath, "wb");
    if (!file) {
        LOG_MESSAGE(ERROR, "Error: Unable to open or read file %s.", output_payload_filepath);
        return false;//todo change
    }

    // Write the payload to the file
    size_t bytes_written = fwrite(payload_data.array, sizeof(PayloadType), payload_data.length, file);
    if (bytes_written != payload_data.length) {
        LOG_MESSAGE(ERROR, "Error: Unable to write payload to file.");
        fclose(file);
        return false;//todo change
    }

    fclose(file);


    return true;
}

void clenup() {
    memory_management_report();
}

//endregion

