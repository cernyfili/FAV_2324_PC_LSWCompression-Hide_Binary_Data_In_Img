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
#include "payload_prepare.h"
#include "utils/utils.h"
#include "image.h"

#define ARGUMENT_COUNT 4

//todo write

//region FUNCTIONS DECLARATION

#define HIDE_DATA_FLAG "-h"

#define EXTRACT_DATA_FLAG "-x"

static bool
hide_payload(const char *input_image_filepath, const char *hide_payload_filepath, const char *output_image_filepath);

static bool extract_payload(const char *input_image_filepath, const char *output_payload_filepath);

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
    if (argc != ARGUMENT_COUNT) {
        printf("ERROR: Wrong number of arguments - needed 3 arguments\n"
        );
        return 1;
    }

    //arguments to variable
    char *direction_flag = argv[2];

    if (strcmp(direction_flag, HIDE_DATA_FLAG) == 0) {
        char *input_image_filepath = argv[1];
        char *output_image_filepath = argv[1];
        char *hide_payload_filepath = argv[3];
        
        // Hide the payload into the image
        if (hide_payload(input_image_filepath, hide_payload_filepath, output_image_filepath) == 0) {
            printf("Hiding completed.\n");
        } else {
            fprintf(stderr, "Error while hiding the payload.\n");
            return 2;
        }
    } else if (strcmp(direction_flag, EXTRACT_DATA_FLAG) == 0) {
        char *input_image_filepath = argv[1];
        char *output_payload_filepath = argv[3];
        
        // Extract the payload from the image
        if (extract_payload(input_image_filepath, output_payload_filepath) == 0) {
            printf("Extraction completed.\n");
        } else {
            fprintf(stderr, "Error while extracting the payload.\n");
            return 3;
        }
    } else {
        fprintf(stderr, "Invalid direction flag. Use -h for hiding or -x for extraction.\n");
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
static bool hide_payload(const char *input_image_filepath, const char *hide_payload_filepath, const char *output_image_filepath) {
    //Check if the arguments are valid
    if (!input_image_filepath || !hide_payload_filepath || !output_image_filepath) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return false;
    }

    bool **hide_data = prepare_payload_data(hide_payload_filepath);
    if (!hide_data) {
        LOG_MESSAGE(ERROR, "Error: Unable to prepare the payload data.");
        return false;
    }

    // Hide the payload into the image
    bool is_success = hide_data_lsb(input_image_filepath, &hide_data, output_image_filepath);

    //Check if the hiding was successful
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Error: Unable to hide the payload into the image.");
        return false;
    }

    return true;
}

/**
 * @brief extract payload
 * @param input_image_filepath
 * @param output_payload_filepath
 * @return
 */
static bool extract_payload(const char *input_image_filepath, const char *output_payload_filepath) {
    //Check if the arguments are valid
    if (!input_image_filepath || !output_payload_filepath) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return false;
    }

    // Extract the payload from the image
    bool *hide_data = NULL;
    bool is_success = extract_data_lsb(input_image_filepath, &hide_data);

    //Check if the extraction was successful
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Error: Unable to extract the payload from the image.");
        return false;
    }

    bool is_success = get_payload_from_hide_data(hide_data);//todo change to something
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Error: Unable to get the payload from the array.");
        return false;
    }

    //save payload to file
    FILE *file = fopen(output_payload_filepath, "wb");
    if (!file) {
        LOG_MESSAGE(ERROR, "Error: Unable to open or read file %s.", output_payload_filepath);
        return false;
    }


    return true;
}

//endregion

