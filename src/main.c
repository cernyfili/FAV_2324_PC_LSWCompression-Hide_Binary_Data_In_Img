//
// Author: Lenovo
// Date: 07.11.2023
// Description: Main file for project
//

#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#define ARGUMENT_NUM 4

//todo write

/**
 * @brief main function
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[]) {
    /*ERROR wrong number of arguments -> user manual */
    if (argc != ARGUMENT_NUM) {
        printf("ERROR: Wrong number of arguments - needed 3 arguments\n"
        );
        return 1;
    }

    //arguments to variable
    char *image_filepathpath = argv[1];
    char *direction_flag = argv[2];
    char *payload_filepathpath = argv[3];

    if (strcmp(direction_flag, "-h") == 0) {
        // Hide the payload into the image
        if (hide_payload(image_filepath, payload_filepath) == 0) {
            printf("Hiding completed.\n");
        } else {
            fprintf(stderr, "Error while hiding the payload.\n");
            return 2;
        }
    } else if (strcmp(direction_flag, "-x") == 0) {
        // Extract the payload from the image
        if (extract_payload(image_filepath, payload_filepath) == 0) {
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

    return 0;

}

