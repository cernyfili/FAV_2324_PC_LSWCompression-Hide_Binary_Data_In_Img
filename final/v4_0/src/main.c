//
// Author: Filip Cerny
// Date: 07.11.2023
// Description: Main file for project
//

//Lib includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

//Local includes
#include "payload.h"
#include "utils/utils.h"
#include "image.h"
#include "utils/dictionary.h"

//region DEFINE, MACROS

/**
 * Number of program arguments
 */
#define ARGUMENT_COUNT 4

/**
 * Flag for hiding data
 */
#define HIDE_DATA_FLAG "-h"

/**
 * Flag for extracting data
 */
#define EXTRACT_DATA_FLAG "-x"

/**
 * Tutorial string
 */
#define TUTORIAL_STR "You can use this program for hiding data in image or extract hidden data from image.\n"\
                     "Hiding: stegim.exe <image_to_hide_to_filepath> -h <hide_payload_filepath>\n"\
                     "Extraction: stegim.exe <input_image_to_extract_from_filepath> -x <output_payload_filepath>\n"
/**
 * Temporary filename
 */
#define TEMP_FILENAME "temp"

/**
 * Best LZW compression rate
 */
#define BEST_LZW_COMPRESSION_RATE 5.0
//endregion

//region FUNCTIONS DECLARATION

#define ARRAY_INIT_SIZE 1000

/**
 * Function to hide payload
 * @param input_image_filepath is path to input image
 * @param hide_payload_filepath  is path to file with payload
 * @return
 * 0 succesfull
 * 1 cannot open file
 * 2 if input file is not BMP or PNG
 * 3 if payload is too big for image
 * 6 other error
 */
static int
hide_payload(const char *input_image_filepath, const char *hide_payload_filepath);

/**
 * Function to extract payload
 * @param input_image_filepath is path to input image
 * @param output_payload_filepath is path to output file with payload
 * @return
 * 0 succesfull
 * 1 cannot open file
 * 2 if input file is not BMP or PNG; not 24 bit RGB
 * 6 other error
 */
static int extract_payload(const char *input_image_filepath, const char *output_payload_filepath);

/**
 * cleanup function
 */
void cleanup();

/**
 * Print error message
 * @param result is result of program
 */
static void print_error_message(int result);

/**
 * Check if payload fits into image
 * @param image_filepath is path to input image
 * @param payload_filepath is path to file with payload
 * @return
 * 0 succesfull
 * 1 cannot open file
 * 2 if input file is not BMP or PNG
 * 3 if payload is too big for image
 * 6 other error
 */
static int check_if_payload_fits(const char *image_filepath, const char *payload_filepath);

//endregion

//region FUNCTIONS DEFINITIONS

/**
 * @brief main function
 * @param argc is number of arguments
 * @param argv is array of arguments
 * @return
 * 0 succesfull
 * 1 wrong number of arguments
 * 2 if input file is not BMP or PNG
 * 3 if payload is too big for image
 * 4 if image doesn't have payload inside
 * 5 if hidden file in image was corrupted
 * 6 other error
 */
int main(int argc, char *argv[]) {
    atexit(cleanup);

    memory_management_init();

    if (argc != ARGUMENT_COUNT) {
        LOG_MESSAGE(ERROR, "Wrong number of arguments - needed 3 arguments");
        printf("Wrong number of arguments - needed 3 arguments\n");
        printf(TUTORIAL_STR);

        exit(1);
    }
    
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
            LOG_MESSAGE(ERROR, "Unable to hide the payload into the image.");
            printf("Unable to hide the payload into the image.\n");
            print_error_message(result);
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
            LOG_MESSAGE(ERROR, "Unable to extract the payload from the image.");
            printf("Unable to extract the payload from the image.\n");
            print_error_message(result);
            exit(result);
        }
    } else {
        LOG_MESSAGE(ERROR, "Invalid direction flag. Use -h for hiding or -x for extraction.");
        printf("Invalid direction flag. Use -h for hiding or -x for extraction.\n");
        printf(TUTORIAL_STR);
        exit(1);
    }

    exit(0);
}

/**
 * Print error message
 * @param result is result of program
 */
static void print_error_message(int result) {
    switch (result) {
        case 1:
            printf("Files doesn't exist or cannot be open.\n");
            break;
        case 2:
            printf("Input file is not BMP or PNG 24 bit color.\n");
            break;
        case 3:
            printf("Payload is too big for image.\n");
            break;
        case 4:
            printf("Image doesn't have payload inside.\n");
            break;
        case 5:
            printf("Hidden file in image was corrupted.\n");
            break;
        case 6:
            printf("Other error.\n");
            break;
        default:
            printf("Unknown error.\n");
            break;
    }

}

/**
 * Check if payload fits into image
 * @param image_filepath is path to input image
 * @param payload_filepath is path to file with payload
 * @return
 * 0 succesfull
 * 1 cannot open file
 * 2 if input file is not BMP or PNG
 * 3 if payload is too big for image
 * 6 other error
 */
static int check_if_payload_fits(const char *image_filepath, const char *payload_filepath) {
    //Check if the arguments are valid
    if (!image_filepath || !payload_filepath) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return 6;
    }
    //Check if the input image file exists
    if (!file_exists(image_filepath) || !file_exists(payload_filepath)) {
        LOG_MESSAGE(ERROR, "Input image file does not exist.");
        return 1;
    }

    bool is_success;
    int result;

    //Get the size of the payload
    size_t payload_size_bytes;
    is_success = get_file_size(payload_filepath, &payload_size_bytes);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Unable to get the size of the payload.");
        return 6;
    }
    size_t payload_size_bits = (size_t)(((double)payload_size_bytes * BITS_IN_BYTE)/BEST_LZW_COMPRESSION_RATE);

    //Get the size of the image
    size_t image_size;
    result = get_image_pixelscount(image_filepath, &image_size);
    if (result != 0) {
        LOG_MESSAGE(ERROR, "Unable to get the size of the image.");
        return result;
    }

    //Check if the payload fits into the image
    if (payload_size_bits >= image_size) {
        LOG_MESSAGE(ERROR, "The payload is too big for the image.");
        return 3;
    }

    return 0;
}

/**
 *  Function to hide payload
 * @param input_image_filepath is path to input image
 * @param hide_payload_filepath is path to file with payload
 * @return
 * 0 succesfull
 * 1 cannot open file
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
    //Check if the input image file exists
    if (!file_exists(input_image_filepath) || !file_exists(hide_payload_filepath)) {
        LOG_MESSAGE(ERROR, "Input image file does not exist.");
        return 1;
    }

    struct cleanupcommand *cleanup_list = NULL;
    bool is_success;
    int result;
    
    result = check_if_payload_fits(input_image_filepath, hide_payload_filepath);
    if (result != 0) {
        LOG_MESSAGE(ERROR, "Unable to check if the payload fits into the image.");
        return result;
    }

    LOG_MESSAGE(INFO, "MAIN: Before prepare_payload_data");

    struct binarydataarray hide_data;
    is_success = prepare_payload_data(hide_payload_filepath, &hide_data);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Unable to prepare the payload data.");
        cleanup_run_commands(&cleanup_list);
        return 6;
    }
    CLEANUP_ADD_COMMAND(&cleanup_list, hide_data.array);

    char *temp_output_image_filepath = TEMP_FILENAME;

    // Hide the payload into the image
    result = hide_data_lsb(input_image_filepath, hide_data, temp_output_image_filepath);
    //Check if the hiding was successful
    if (result != 0) {
        LOG_MESSAGE(ERROR, "Unable to hide the payload into the image.");
        cleanup_run_commands(&cleanup_list);
        return result;
    }

    //Copy the temp file to the original file
    FILE *temp_file = fopen(temp_output_image_filepath, "rb");
    FILE *original_file = fopen(input_image_filepath, "wb");

    is_success = copy_files(temp_file, original_file);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Unable to copy the temp file to the original file.");
        cleanup_run_commands(&cleanup_list);
        fclose(temp_file);
        fclose(original_file);
        return 6;
    }


    //delete temp file
    fclose(temp_file);

    result = remove(temp_output_image_filepath);
    if (result != 0) {
        LOG_MESSAGE(ERROR, "Unable to delete the temp file.");
        cleanup_run_commands(&cleanup_list);
        fclose(original_file);
        return 6;
    }

    fclose(original_file);
    cleanup_run_commands(&cleanup_list);
    return 0;
}

/**
 * Function to extract payload
 * @param input_image_filepath is path to input image
 * @param output_payload_filepath is path to output file with payload
 * @return
 * 0 succesfull
 * 1 cannot open file
 * 2 if input file is not BMP or PNG; not 24 bit RGB
 * 6 other error
 */
static int extract_payload(const char *input_image_filepath, const char *output_payload_filepath) {
    //Check if the arguments are valid
    if (!input_image_filepath || !output_payload_filepath) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return 6;
    }
    //Check if the file exists
    if (!file_exists(input_image_filepath)) {
        LOG_MESSAGE(ERROR, "Input image file does not exist.");
        return 1;
    }

    int result;
    struct cleanupcommand *cleanup_list = NULL;

    // Extract the payload from the image
    struct binarydataarray hidden_data;

    //Prepare ptr_hidden_data
    bool is_success = binarydataarray_initialize(&hidden_data);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Failed to allocate memory for payload array.");
        return false;
    }

    result = extract_data_lsb(input_image_filepath, &hidden_data);
    //Check if the extraction was successful
    if (result != 0) {
        LOG_MESSAGE(ERROR, "Unable to extract the payload from the image.");
        TRACKED_FREE(hidden_data.array);
        cleanup_run_commands(&cleanup_list);
        return result;
    }


    struct dicvaluearray payload_data;
    payload_data.length = 0;
    payload_data.capacity = 0;
    payload_data.array = NULL;
    result = extract_payload_from_data(&hidden_data, &payload_data);
    if (result != 0) {
        LOG_MESSAGE(ERROR, "Unable to get the payload from the array.");
        cleanup_run_commands(&cleanup_list);
        return result;
    }

    TRACKED_FREE(hidden_data.array);


    //save payload to file
    FILE *output_file = fopen(output_payload_filepath, "wb");
    if (!output_file) {
        LOG_MESSAGE(ERROR, "Unable to open or read file %s.", output_payload_filepath);
        cleanup_run_commands(&cleanup_list);
        dicvaluearray_free(&payload_data);
        return 1;
    }



    // Write the payload to the file
    size_t bytes_written = fwrite(payload_data.array, sizeof(char), payload_data.length, output_file);
    if (bytes_written != payload_data.length * sizeof(char)) {
        LOG_MESSAGE(ERROR, "Unable to write the payload to the file.");
        cleanup_run_commands(&cleanup_list);
        dicvaluearray_free(&payload_data);
        fclose(output_file);
        return 6;
    }


    fclose(output_file);
    dicvaluearray_free(&payload_data);
    cleanup_run_commands(&cleanup_list);

    return 0;
}

/**
 * cleanup function
 */
void cleanup() {
    memory_management_report();

    memory_management_destroy();
}

//endregion

