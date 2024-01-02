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

static bool
hide_payload(const char *input_image_filepath, const char *hide_payload_filepath, const char *output_image_filepath);

static int extract_payload(const char *input_image_filepath, const char *output_payload_filepath);

//endregion


//region FUNCTIONS DEFINITIONS

/**
 * @brief main function
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[]) {
    /*ERROR wrong number of arguments -> user manual */
    //todo user manual
    if (argc != ARGUMENT_COUNT) {
        printf("ERROR: Wrong number of arguments - needed 3 arguments\n"
        );
        return 1;
    }

    bool is_success;

    //arguments to variable
    char *direction_flag = argv[2];

    //flag to hide data
    if (strcmp(direction_flag, HIDE_DATA_FLAG) == 0) {
        char *input_image_filepath = argv[1];
        char *output_image_filepath = argv[1];
        char *hide_payload_filepath = argv[3];

        // Hide the payload into the image
        is_success = hide_payload(input_image_filepath, hide_payload_filepath, output_image_filepath);
        if (is_success) {
            printf("Hiding completed.\n");
        } else {
            LOG_MESSAGE(ERROR, "Error: Unable to hide the payload into the image.");
            return 2;
        }
    //flag to extract data
    } else if (strcmp(direction_flag, EXTRACT_DATA_FLAG) == 0) {
        char *input_image_filepath = argv[1];
        char *output_payload_filepath = argv[3];

        // Extract the payload from the image
        int result = extract_payload(input_image_filepath, output_payload_filepath);
        if (result == 0) {
            printf("Extraction completed.\n");
        } else {
            LOG_MESSAGE(ERROR, "Error: Unable to extract the payload from the image.");
            return result;
        }
    } else {
        LOG_MESSAGE(ERROR, "Invalid direction flag. Use -h for hiding or -x for extraction.");
        return 4;
    }

    return 0;

}

/**
 * @brief hide payload
 * @param input_image_filepath
 * @param hide_payload_filepath
 * @return
 */
static bool
hide_payload(const char *input_image_filepath, const char *hide_payload_filepath, const char *output_image_filepath) {
    //Check if the arguments are valid
    if (!input_image_filepath || !hide_payload_filepath || !output_image_filepath) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return false;
    }

    bool is_success;

    PayloadArray hide_data;
    is_success = prepare_payload_data(hide_payload_filepath, &hide_data);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Error: Unable to prepare the payload data.");
        return false;
    }

    // Hide the payload into the image
    is_success = hide_data_lsb(input_image_filepath, hide_data, output_image_filepath);

    //Check if the hiding was successful
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Error: Unable to hide the payload into the image.");
        return false;
    }

    TRACKED_FREE(hide_data.array);

    return true;
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

    // Extract the payload from the image
    PayloadArray hidden_data;
    is_success = extract_data_lsb(input_image_filepath, &hidden_data);

    //Check if the extraction was successful
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Error: Unable to extract the payload from the image.");
        return false;//todo change
    }

    PayloadArray payload_data;
    int result = extract_payload_from_data(hidden_data, &payload_data);
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

//endregion

