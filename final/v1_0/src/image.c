//
// Author: Filip Cerny
// Date: 07.11.2023
// Description: File contains functions for hiding and extracting data in/from image.
//

//Local includes
#include "image.h"
#include "utils/utils.h"

//Lib includes
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <png.h>

//region DEFINE, MACROS
/**
 * The size of the PNG signature in bytes.
 */
#define PNG_SIGNATURE_SIZE 8

/**
 * The BMP signature.
 */
#define BMP_SIGNATURE 0x4D42

/**
 * The number of color components per pixel in a BMP image.
 */
#define PIXEL_COLOR_NUM 3

/**
 * The number of bits per pixel in a BMP image for 24-bit RGB color depth.
 */
#define RGB_BIT_SIZE 24


/**
 * The number of bytes in a pixel
 */
#define NUM_COLOR_PIXEL 3

/**
 * BMP signature offset in bytes
 */
#define BMP_SIGNATURE_OFFSET_BYTES 0

/**
 * BMP data offset in bytes
 */
#define BMP_DATAOFFSET_OFFSET_BYTES 10

/**
 * BMP height offset in bytes
 */
#define BMP_HEIGHT_OFFSET_BYTES 22

/**
 * BMP bitcount offset in bytes
 */
#define BMP_BITCOUNT_OFFSET_BYTES 28

/**
 * BMP payloadsize offset in bytes
 */
#define PAYLOAD_SIZE_OFFSET 4

/**
 * BMP width offset in bytes
 */
#define BMP_WIDTH_OFFSET_BYTES 18


//endregion


//region STRUCTURES

//BMP METADATA
typedef uint16_t bmp_signatue_type;
typedef uint32_t bmp_dataoffset_type;
typedef uint32_t bmp_width_type;
typedef uint32_t bmp_height_type;
typedef uint16_t bmp_bitcount_type;

/**
 * BMP metadata
 */
struct bmpmetadata {
    bmp_signatue_type signature;
    bmp_dataoffset_type dataoffset;
    bmp_width_type width;
    bmp_height_type height;
    bmp_bitcount_type bitcount;
};
//endregion

//region FUNCTIONS DECLARATION

/**
 * Hides binary data in the Least Significant Bit (LSB) of each pixel in a PNG image.
 *
 * @param input_filepath - The path to the input PNG file.
 * @param hide_data - The binary data to hide. Each bit of the data will be embedded in the LSB of the image.
 * @param output_filepath - The path to the output PNG file.
 * @return
 * 0 succesfull
 * 1 cannot open file
 * 2 if input file is not BMP; not 24 bit RGB
 * 3 if payload is too big for image
 * 6 other error
 */
static int hide_data_lsb_png(const char *input_filepath, struct payloadarray hide_data,
                             char *output_filepath);

/**
 * Hides binary data in the least significant bit (LSB) of each RGB value in a BMP file.
 *
 * @param input_filepath - Path to the input BMP file.
 * @param payload_data - Pointer to an array of binary data to be hidden.
 * @param output_filepath - Path to the output BMP file.
 * @return 0 if the hiding process is successful, non-zero otherwise.
 */
static int hide_data_lsb_bmp(const char *input_filepath, struct payloadarray payload_data,
                             char *output_filepath);

/**
 * Extracts binary data from the least significant bit (LSB) of each RGB value in a BMP file.
 * @param input_image_filepath  The path to the input BMP file.
 * @param ptr_return_hidden_data  The path to the output BMP file.
 * @return  0 if the hiding process is successful, non-zero otherwise.
 */
static int extract_data_lsb_png(const char *input_image_filepath, struct payloadarray *ptr_return_hidden_data);


/**
 * Extracts binary data from the least significant bit (LSB) of each RGB value in a BMP file.
 * @param input_image_filepath  The path to the input BMP file.
 * @param ptr_return_hidden_data  The path to the output BMP file.
 * @return  0 if the hiding process is successful, non-zero otherwise.
 */
static int extract_data_lsb_bmp(const char *input_image_filepath, struct payloadarray *ptr_return_hidden_data);

/**
 * Checks if the input file is a PNG file.
 * @param input_file  The input file.
 * @return true if the input file is a PNG file, false otherwise.
 */
static bool is_png_file(FILE *input_file);

/**
 *  Sets the last bit of the value to the change_value.
 * @param change_value  The value to be set as the last bit.
 * @param value  The value to be changed.
 * @return  true if the change was successful, false otherwise.
 */
static bool set_last_bit(bool change_value, uint8_t *value);

/**
 * Checks if the input file is a BMP file.
 * @param input_file  The input file.
 * @return true if the input file is a BMP file, false otherwise.
 */
static bool get_last_bit(colorvalue_type value);

/**
 * Checks if the input file is a BMP file.
 * @param input_file  The input file.
 * @return true if the input file is a BMP file, false otherwise.
 */
static bool read_bmp_metadata(FILE *input_file, struct bmpmetadata *ptr_return_bmpmetadata);

/**
 * Checks if the input file is a BMP file.
 * @param input_file  The input file.
 * @return true if the input file is a BMP file, false otherwise.
 */
static bool payload_get_payloadsize(struct payloadarray array, size_t *ptr_return_size);


/**
 * Frees the memory allocated by libpng.
 * @param png_read_ptr  The PNG read structure.
 * @param png_read_info_ptr  The PNG info structure.
 * @param row_pointers_size  The number of rows in the PNG image.
 * @param row_pointers  The PNG image rows.
 */
static void free_png_read_values(png_structp *png_read_ptr, png_infop *png_read_info_ptr, png_uint_32 row_pointers_size,
                                 png_bytepp row_pointers);

/**
 * Frees the memory allocated by libpng.
 * @param png_read_ptr  The PNG read structure.
 * @param png_read_info_ptr  The PNG info structure.
 * @param row_pointers_size  The number of rows in the PNG image.
 * @param row_pointers  The PNG image rows.
 * @param png_write_ptr  The PNG write structure.
 * @param png_write_end_info_ptr  The PNG end info structure.
 */
static void
free_png_read_write_values(png_structp *png_read_ptr, png_infop *png_read_info_ptr, png_uint_32 row_pointers_size,
                           png_bytepp row_pointers, png_structp *png_write_ptr, png_infop *png_write_end_info_ptr);
//endregion

//region FUNCTIONS DEFINITIONS


//region PUBLIC FUNCTIONS
/**
 * Checks if the input file is a PNG file.
 * @param input_filepath The input file.
 * @param hide_data Pointer to an array of binary data to be hidden.
 * @param output_filepath The path to the output BMP file.
 * @return
 * 0 succesfull
 * 1 cannot open file
 * 2 if input file is not BMP or PNG
 * 3 if payload is too big for image
 * 6 other error
 */
int hide_data_lsb(const char *input_filepath, const struct payloadarray hide_data,
                  char *output_filepath) {
    //Check if the arguments are valid
    if (!input_filepath || !hide_data.array || !output_filepath) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return 6;
    }

    //Check if files exists
    if (!file_exists(input_filepath)) {
        LOG_MESSAGE(ERROR, "Input or output file does not exist.");
        return 1;
    }

    if (strcmp(input_filepath, output_filepath) == 0) {
        LOG_MESSAGE(ERROR, "Input and output filepaths are the same.");
        return 6;
    }

    // Check if the input file has .bmp or .png extension
    char *extension = strrchr(input_filepath, '.');
    if (!extension) {
        LOG_MESSAGE(ERROR, "Input file has no extension.");


        return 2;
    }

    // Check if the input file is a BMP file
    if (strcmp(extension, ".bmp") == 0) {
        int result = hide_data_lsb_bmp(input_filepath, hide_data, output_filepath);

        //Check if the hiding process was successful
        if (result != 0) {
            LOG_MESSAGE(ERROR, "Error while hiding the payload.");
            return result;
        }

    } else if (strcmp(extension, ".png") == 0) {
        int result = hide_data_lsb_png(input_filepath, hide_data, output_filepath);

        //Check if the hiding process was successful
        if (result != 0) {
            LOG_MESSAGE(ERROR, "Error while hiding the payload.");
            return result;
        }
    } else {
        LOG_MESSAGE(ERROR, "Input file is not a BMP or PNG file.");


        return 2;
    }

    return 0;
}

/**
 * Extracts binary data from the least significant bit (LSB) of each RGB value in a BMP file.
 * @param input_image_filepath The path to the input BMP file.
 * @param ptr_return_hidden_data The path to the output BMP file.
 * @return
 * 0 succesfull
 * 1 cannot open file
 * 2 if input file is not BMP or PNG; not 24 bit RGB
 * 6 other error
 */
int extract_data_lsb(const char *input_image_filepath, struct payloadarray *ptr_return_hidden_data) {
    //Check if the arguments are valid
    if (!input_image_filepath || !ptr_return_hidden_data) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return 6;
    }
    //Check if files exists
    if (!file_exists(input_image_filepath)) {
        LOG_MESSAGE(ERROR, "Input file does not exist.");
        return 1;
    }

    int result;

    // Check if the input file has .bmp or .png extension
    char *extension = strrchr(input_image_filepath, '.');
    if (!extension) {
        LOG_MESSAGE(ERROR, "Input file has no extension.");
        return 2;
    }

    // Check if the input file is a BMP file
    if (strcmp(extension, ".bmp") == 0) {
        result = extract_data_lsb_bmp(input_image_filepath, ptr_return_hidden_data);

        //Check if the extraction process was successful
        if (result != 0) {
            LOG_MESSAGE(ERROR, "Error while extracting the payload.");
            return result;
        }

        return 0;

    }

    // Check if the input file is a PNG file
    if (strcmp(extension, ".png") == 0) {
        result = extract_data_lsb_png(input_image_filepath, ptr_return_hidden_data);

        //Check if the extraction process was successful
        if (result != 0) {
            LOG_MESSAGE(ERROR, "Error while extracting the payload.");
            return result;
        }

        return 0;
    }


    LOG_MESSAGE(ERROR, "Input file is not a BMP or PNG file.");
    return 2;
}
//endregion


//region PRIVATE FUNCTIONS
/**
 * Hides binary data in the Least Significant Bit (LSB) of each pixel in a PNG image.
 *
 * @param input_filepath - The path to the input PNG file.
 * @param hide_data - The binary data to hide. Each bit of the data will be embedded in the LSB of the image.
 * @param output_filepath - The path to the output PNG file.
 * @return
 * 0 succesfull
 * 1 cannot open file
 * 2 if input file is not PNG; not 24 bit RGB
 * 3 if payload is too big for image
 * 6 other error
 */
static int hide_data_lsb_png(const char *input_filepath, struct payloadarray hide_data,
                             char *output_filepath) {

    //Check if the arguments are valid
    if (!input_filepath || !hide_data.array || !output_filepath) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return 6;
    }

    //Check if file exists
    if (!file_exists(input_filepath)) {
        LOG_MESSAGE(ERROR, "Input file does not exist.");
        return 1;
    }

    //Check if files are same
    if (strcmp(input_filepath, output_filepath) == 0) {
        LOG_MESSAGE(ERROR, "Input and output filepaths are the same.");
        return 6;
    }

    FILE *input_file = fopen(input_filepath, "rb");
    FILE *output_file = fopen(output_filepath, "wb");

    // Check if the file was opened successfully
    if (!input_file || !output_file) {
        LOG_MESSAGE(ERROR, "Unable to open file.");
        fclose(input_file);
        fclose(output_file);
        return 1;
    }

    // Check if the input file is a PNG file
    bool is_success = is_png_file(input_file);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Input file is not a PNG file.");
        fclose(input_file);
        fclose(output_file);
        return 2;
    }

    // Create PNG read and write structures
    png_structp png_read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_structp png_write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_read_ptr || !png_write_ptr) {
        LOG_MESSAGE(ERROR, "Failed to create PNG read or write structure.");
        fclose(input_file);
        fclose(output_file);
        return 6;
    }

    // Create PNG info structures
    png_infop png_read_info_ptr = png_create_info_struct(png_read_ptr);
    png_infop png_write_end_info_ptr = png_create_info_struct(png_write_ptr);
    if (!png_read_info_ptr || !png_write_end_info_ptr) {
        LOG_MESSAGE(ERROR, "Failed to create PNG info structure.");
        png_destroy_read_struct(&png_read_ptr, NULL, NULL);
        png_destroy_write_struct(&png_write_ptr, NULL);
        fclose(input_file);
        fclose(output_file);
        return 6;
    }

    // Set up error handling
    if (setjmp(png_jmpbuf(png_read_ptr)) || setjmp(png_jmpbuf(png_write_ptr))) {
        LOG_MESSAGE(ERROR, "Error occurred during PNG read or write operation.");
        png_destroy_read_struct(&png_read_ptr, &png_read_info_ptr, NULL);
        png_destroy_write_struct(&png_write_ptr, &png_write_end_info_ptr);
        fclose(input_file);
        fclose(output_file);
        return 2;
    }

    // Initialize PNG IO
    png_init_io(png_read_ptr, input_file);
    png_init_io(png_write_ptr, output_file);

    //set signature bytes
    png_set_sig_bytes(png_read_ptr, PNG_SIGNATURE_SIZE);


    // Read PNG header
    png_read_info(png_read_ptr, png_read_info_ptr);

    //Check if the input file is 24-bit RGB PNG file
    int bit_depth = png_get_bit_depth(png_read_ptr, png_read_info_ptr);
    if (bit_depth * PIXEL_COLOR_NUM != RGB_BIT_SIZE) {
        LOG_MESSAGE(ERROR, "Input file is not a 24-bit RGB PNG image.");
        png_destroy_read_struct(&png_read_ptr, &png_read_info_ptr, NULL);
        png_destroy_write_struct(&png_write_ptr, &png_write_end_info_ptr);
        fclose(input_file);
        fclose(output_file);
        return 2;
    }

    //Get color type
    int color_type = png_get_color_type(png_read_ptr, png_read_info_ptr);

    //Check if payload fits to image
    png_uint_32 height = png_get_image_height(png_read_ptr, png_read_info_ptr);
    png_uint_32 width = png_get_image_width(png_read_ptr, png_read_info_ptr);
    double pixel_count = width * (double) height;
    int bits_in_payload = (int) ((double) hide_data.length * BITS_IN_BYTE);
    if (pixel_count < bits_in_payload) {
        LOG_MESSAGE(ERROR, "Payload is too big for image.");
        png_destroy_read_struct(&png_read_ptr, &png_read_info_ptr, NULL);
        png_destroy_write_struct(&png_write_ptr, &png_write_end_info_ptr);
        fclose(input_file);
        fclose(output_file);
        return 3;
    }

    // Write PNG header
    if (setjmp(png_jmpbuf(png_write_ptr))) {
        LOG_MESSAGE(ERROR, "Error occurred during PNG read or write operation.");
        png_destroy_read_struct(&png_read_ptr, &png_read_info_ptr, NULL);
        png_destroy_write_struct(&png_write_ptr, &png_write_end_info_ptr);
        fclose(input_file);
        fclose(output_file);
        return 6;
    }

    png_set_IHDR(png_write_ptr, png_write_end_info_ptr, width, height,
                 bit_depth, color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_write_ptr, png_write_end_info_ptr);

    // Read PNG image row_array

    // Allocate memory for pixel row_array
    png_bytepp row_pointers = (png_bytepp) png_malloc(png_read_ptr, height * sizeof(png_bytep));

    // Error reading PNG image
    if (setjmp(png_jmpbuf(png_read_ptr))) {
        LOG_MESSAGE(ERROR, "Error occurred during PNG read or write operation.");
        png_destroy_read_struct(&png_read_ptr, &png_read_info_ptr, NULL);
        png_destroy_write_struct(&png_write_ptr, &png_write_end_info_ptr);
        fclose(input_file);
        fclose(output_file);
        return 6;
    }
    for (int y = 0; y < height; y++) {
        row_pointers[y] = (png_bytep) png_malloc(png_read_ptr, png_get_rowbytes(png_read_ptr, png_read_info_ptr));
        /*png_read_row(png_read_ptr, row_pointers[y], NULL);*/
    }
    png_read_image(png_read_ptr, row_pointers);

    //vars for bitshifts
    size_t payload_array_index = 0;
    size_t payload_element_bitshift = 0;

    // Hide binary row_array in the LSB of each pixel
    for (int y = 0; y < height; y++) {
        //end when all payload data is hidden
        if (payload_array_index >= hide_data.length) {
            break;
        }

        size_t row_bytes = png_get_rowbytes(png_read_ptr, png_read_info_ptr);
        for (int x = 0; x < row_bytes; x += 3) {
            //end when all payload data is hidden
            if (payload_array_index >= hide_data.length) {
                break;
            }

            colorvalue_type *ptr_blue_value = &row_pointers[y][x + 2];

            //change last bit of b_value to ptr_hidden_data
            bool next_payload_bit;
            is_success = payloadarray_get_next_bit(hide_data, &payload_array_index,
                                                   &payload_element_bitshift, &next_payload_bit);
            if (!is_success) {
                LOG_MESSAGE(ERROR, "Cannot get next bit");
                free_png_read_write_values(&png_read_ptr, &png_read_info_ptr, height, row_pointers,
                                           &png_write_ptr, &png_write_end_info_ptr);
                fclose(input_file);
                fclose(output_file);
                return 6;
            }

            is_success = set_last_bit(next_payload_bit, ptr_blue_value);
            if (!is_success) {
                LOG_MESSAGE(ERROR, "Cannot change last bit");
                free_png_read_write_values(&png_read_ptr, &png_read_info_ptr, height, row_pointers,
                                           &png_write_ptr, &png_write_end_info_ptr);
                fclose(input_file);
                fclose(output_file);
                return 6;
            }
        }
    }

    // Write modified PNG image row_array with modified blue values
    //Check if write error
    if (setjmp(png_jmpbuf(png_write_ptr))) {
        LOG_MESSAGE(ERROR, "Error occurred during PNG read or write operation.");
        free_png_read_write_values(&png_read_ptr, &png_read_info_ptr, height, row_pointers,
                                   &png_write_ptr, &png_write_end_info_ptr);
        fclose(input_file);
        fclose(output_file);
        return 6;
    }
    png_uint_32 height_output = png_get_image_height(png_write_ptr, png_write_end_info_ptr);
    for (int y = 0; y < height_output; y++) {
        png_write_row(png_write_ptr, row_pointers[y]);
    }
    //Check if write error
    if (setjmp(png_jmpbuf(png_write_ptr))) {
        LOG_MESSAGE(ERROR, "Error occurred during PNG read or write operation.");
        free_png_read_write_values(&png_read_ptr, &png_read_info_ptr, height, row_pointers,
                                   &png_write_ptr, &png_write_end_info_ptr);
        fclose(input_file);
        fclose(output_file);
        return 6;
    }

    // Write end of PNG
    png_write_end(png_write_ptr, png_write_end_info_ptr);


    // Clean up
    free_png_read_write_values(&png_read_ptr, &png_read_info_ptr, height, row_pointers,
                               &png_write_ptr, &png_write_end_info_ptr);

    // Close the files
    fclose(input_file);
    fclose(output_file);

    return 0;
}

/**
 * Hides binary data in the least significant bit (LSB) of each RGB value in a BMP file.
 *
 * @param input_filepath - Path to the input BMP file.
 * @param payload_data - Pointer to an array of binary data to be hidden.
 * @param output_filepath - Path to the output BMP file.
 * @return 0 if the hiding process is successful, non-zero otherwise.
 * 1 cannot open file
 * 2 if input file is not BMP; not 24 bit RGB
 * 3 if payload is too big for image
 * 6 other error
 */
static int hide_data_lsb_bmp(const char *input_filepath, struct payloadarray payload_data,
                             char *output_filepath) {

    //Check if the arguments are valid
    if (!input_filepath || !payload_data.array ||
        !output_filepath) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return 6;
    }

    //Check if file exists
    if (!file_exists(input_filepath)) {
        LOG_MESSAGE(ERROR, "Input file does not exist.");
        return 1;
    }

    //Check if files are same
    if (strcmp(input_filepath, output_filepath) == 0) {
        LOG_MESSAGE(ERROR, "Input and output filepaths are the same.");
        return 6;
    }

    struct cleanupcommand *cleanup_list = NULL;
    FILE *input_file = fopen(input_filepath, "rb");
    FILE *output_file = fopen(output_filepath, "wb");

    // Check if the file was opened successfully
    if (!input_file || !output_file) {
        LOG_MESSAGE(ERROR, "Unable to open file.");
        fclose(input_file);
        fclose(output_file);
        return 1;
    }

    //read BMP metadata
    struct bmpmetadata bmp_metadata;
    bool is_success = read_bmp_metadata(input_file, &bmp_metadata);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Cannot read BMP metadata.");
        fclose(input_file);
        fclose(output_file);
        return 6;
    }

    // Check if the input file is a BMP file
    if (bmp_metadata.signature != BMP_SIGNATURE) {
        LOG_MESSAGE(ERROR, "Input file is not a BMP file.");
        fclose(input_file);
        fclose(output_file);
        return 2;
    }

    // Check if it's a 24-bit uncompressed BMP file
    if (bmp_metadata.bitcount != RGB_BIT_SIZE) {
        LOG_MESSAGE(ERROR, "Input file is not 24 bit RGB BMP file or there is compression.");
        fclose(input_file);
        fclose(output_file);
        return 2;
    }


    //Check if payload fits to image
    double pixel_count = bmp_metadata.width * (double) bmp_metadata.height;
    int bits_in_payload = (int) ((double) payload_data.length * BITS_IN_BYTE);
    if (pixel_count < bits_in_payload) {
        LOG_MESSAGE(ERROR, "Payload is too big for image.");
        fclose(input_file);
        fclose(output_file);
        return 3;
    }

    //Copy the whole input file to output file
    is_success = copy_files(input_file, output_file);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Cannot copy files.");
        fclose(input_file);
        fclose(output_file);
        return 6;
    }

    // Move to the start of pixel data - after only editing the color value
    fseek(input_file, bmp_metadata.dataoffset, SEEK_SET);
    fseek(output_file, bmp_metadata.dataoffset, SEEK_SET);

    //create array for one row
    int bytes_per_row = (int) ((double) bmp_metadata.width * (double) NUM_COLOR_PIXEL);
    colorvalue_type *row_array = TRACKED_MALLOC(bytes_per_row * sizeof(colorvalue_type));
    if (!row_array) {
        LOG_MESSAGE(ERROR, "Cannot allocate memory for row array");
        fclose(input_file);
        fclose(output_file);
        return 6;
    }
    CLEANUP_ADD_COMMAND(&cleanup_list, row_array);

    //vars for bitshifts
    size_t payload_array_index = 0;
    size_t payload_element_bitshift = 0;

    for (int y = 0; y < bmp_metadata.height; y++) {
        //end when all payload data is hidden
        if (payload_array_index >= payload_data.length) {
            break;
        }
        for (int x = 0; x < bmp_metadata.width; x++) {
            //end when all payload data is hidden
            if (payload_array_index >= payload_data.length) {
                break;
            }
            colorvalue_type blue_value;
            fread(&blue_value, sizeof(colorvalue_type), 1, input_file);
            colorvalue_type green_value;
            fread(&green_value, sizeof(colorvalue_type), 1, input_file);
            colorvalue_type red_value;
            fread(&red_value, sizeof(colorvalue_type), 1, input_file);

            //change last bit of b_value to ptr_hidden_data
            bool next_payload_bit;
            is_success = payloadarray_get_next_bit(payload_data, &payload_array_index,
                                                   &payload_element_bitshift, &next_payload_bit);
            if (!is_success) {
                LOG_MESSAGE(ERROR, "Cannot get next bit");
                fclose(input_file);
                fclose(output_file);
                cleanup_run_commands(&cleanup_list);
                return 6;
            }

            is_success = set_last_bit(next_payload_bit, &blue_value);
            if (!is_success) {
                LOG_MESSAGE(ERROR, "Cannot change last bit");
                fclose(input_file);
                fclose(output_file);
                cleanup_run_commands(&cleanup_list);
                return 6;
            }

            //write the changed values to output file
            fwrite(&blue_value, sizeof(colorvalue_type), 1, output_file);
            fwrite(&green_value, sizeof(colorvalue_type), 1, output_file);
            fwrite(&red_value, sizeof(colorvalue_type), 1, output_file);
        }
    }
    //endregion

    fclose(input_file);
    fclose(output_file);
    cleanup_run_commands(&cleanup_list);
    return 0;
}

/**
 * Extracts binary data from the least significant bit (LSB) of each RGB value in a BMP file.
 * @param input_image_filepath  The path to the input PNG file.
 * @param ptr_return_hidden_data  The path to the output file.
 * @return
 * 0 succesfull
 * 1 cannot open file
 * 2 if input file is not BMP or PNG; not 24 bit RGB
 * 6 other error
 */
static int extract_data_lsb_png(const char *input_image_filepath, struct payloadarray *ptr_return_hidden_data) {
    //Check if the arguments are valid
    if (!input_image_filepath || !ptr_return_hidden_data) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return 6;
    }
    //Check if files exists
    if (!file_exists(input_image_filepath)) {
        LOG_MESSAGE(ERROR, "Input file does not exist.");
        return 1;
    }

    FILE *input_file = fopen(input_image_filepath, "rb");

    // Check if the file was opened successfully
    if (!input_file) {
        // Unable to open file
        LOG_MESSAGE(ERROR, "Unable to open file.");
        TRACKED_FREE(ptr_return_hidden_data->array);
        fclose(input_file);
        return 1;
    }

    // Check if the input file is a PNG file
    bool is_success = is_png_file(input_file);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Input file is not a PNG file.");
        TRACKED_FREE(ptr_return_hidden_data->array);
        fclose(input_file);
        return 2;
    }


    // Create PNG read structures
    png_structp png_read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_read_ptr) {
        LOG_MESSAGE(ERROR, "Failed to create PNG read or write structure.");
        TRACKED_FREE(ptr_return_hidden_data->array);
        fclose(input_file);
        return 6;
    }

    // Create PNG info structures
    png_infop png_read_info_ptr = png_create_info_struct(png_read_ptr);

    if (!png_read_info_ptr) {
        LOG_MESSAGE(ERROR, "Failed to create PNG info structure.");
        png_destroy_read_struct(&png_read_ptr, NULL, NULL);
        TRACKED_FREE(ptr_return_hidden_data->array);
        fclose(input_file);
        return 6;
    }

    // Set up error handling
    if (setjmp(png_jmpbuf(png_read_ptr))) {
        LOG_MESSAGE(ERROR, "Error occurred during PNG read or write operation.");
        png_destroy_read_struct(&png_read_ptr, &png_read_info_ptr, NULL);
        TRACKED_FREE(ptr_return_hidden_data->array);
        fclose(input_file);
        return 2;
    }

    // Initialize PNG IO
    png_init_io(png_read_ptr, input_file);

    //set signature bytes
    png_set_sig_bytes(png_read_ptr, PNG_SIGNATURE_SIZE);

    // Read PNG header
    png_read_info(png_read_ptr, png_read_info_ptr);

    //Check if the input file is 24-bit RGB PNG file
    int bit_depth = png_get_bit_depth(png_read_ptr, png_read_info_ptr);
    if (bit_depth * PIXEL_COLOR_NUM != RGB_BIT_SIZE) {
        LOG_MESSAGE(ERROR, "Input file is not a 24-bit RGB PNG image.");
        png_destroy_read_struct(&png_read_ptr, &png_read_info_ptr, NULL);

        fclose(input_file);
        return 2;
    }

    // Get image dimensions
    png_uint_32 height = png_get_image_height(png_read_ptr, png_read_info_ptr);

    // Read PNG image row_array
    // Allocate memory for pixel row_array
    png_bytepp row_pointers = (png_bytepp) png_malloc(png_read_ptr, height * sizeof(png_bytep));

    // Error reading PNG image
    if (setjmp(png_jmpbuf(png_read_ptr))) {
        LOG_MESSAGE(ERROR, "Error occurred during PNG read or write operation.");
        png_destroy_read_struct(&png_read_ptr, &png_read_info_ptr, NULL);
        fclose(input_file);
        return 6;
    }
    for (int y = 0; y < height; y++) {
        row_pointers[y] = (png_bytep) png_malloc(png_read_ptr, png_get_rowbytes(png_read_ptr, png_read_info_ptr));
        png_read_row(png_read_ptr, row_pointers[y], NULL);
    }


    //vars for bitshifts
    size_t payload_array_index = 0;
    size_t payload_element_bitshift = 0;

    size_t payload_size_bytes = UINT_MAX;

    // Extract binary row_array in the LSB of each pixel
    for (int y = 0; y < height; y++) {
        //end when all payload data is hidden
        if (ptr_return_hidden_data->length >= payload_size_bytes) {
            break;
        }

        size_t row_bytes = png_get_rowbytes(png_read_ptr, png_read_info_ptr);
        for (int x = 0; x < row_bytes; x += 3) {
            //end when all payload data is hidden
            if (ptr_return_hidden_data->length >= payload_size_bytes) {
                break;
            }
            colorvalue_type blue_value = row_pointers[y][x + 2];

            // Extract the LSB of the pixel value
            bool lsb_bit = get_last_bit(blue_value);

            // Write the LSB to the dynamic binary array
            is_success = payload_array_add_bit(ptr_return_hidden_data, lsb_bit, &payload_array_index,
                                               &payload_element_bitshift);
            if (!is_success) {
                LOG_MESSAGE(ERROR, "Cannot add bit to dynamic binary array.");
                TRACKED_FREE(ptr_return_hidden_data->array);
                free_png_read_values(&png_read_ptr, &png_read_info_ptr, height, row_pointers);
                fclose(input_file);
                return 6;
            }

            //READ payload size from file
            if (ptr_return_hidden_data->length == PAYLOAD_SIZE_OFFSET) {

                is_success = payload_get_payloadsize(*ptr_return_hidden_data, &payload_size_bytes);
                if (!is_success) {
                    LOG_MESSAGE(ERROR, "Cannot get payload size");
                    TRACKED_FREE(ptr_return_hidden_data->array);
                    free_png_read_values(&png_read_ptr, &png_read_info_ptr, height, row_pointers);
                    fclose(input_file);
                    return 6;
                }
            }
        }
    }

    free_png_read_values(&png_read_ptr, &png_read_info_ptr, height, row_pointers);
    fclose(input_file);
    return 0;
}

/**
 * Extracts binary data from the least significant bit (LSB) of each RGB value in a BMP file.
 * @param input_image_filepath  The path to the input BMP file.
 * @param binary_data  Pointer to an array of binary data to be hidden.
 * @param ptr_return_hidden_data  The path to the output BMP file.
 * @return
 * 0 succesfull
 * 1 cannot open file
 * 2 if input file is not BMP or PNG; not 24 bit RGB
 * 6 other error
 */
static int extract_data_lsb_bmp(const char *input_image_filepath, struct payloadarray *ptr_return_hidden_data) {

    //Check if the arguments are valid
    if (!input_image_filepath || !ptr_return_hidden_data) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return 6;
    }

    //Check if file exists
    if (!file_exists(input_image_filepath)) {
        LOG_MESSAGE(ERROR, "Input file does not exist.");
        return 1;
    }

    FILE *input_file = fopen(input_image_filepath, "rb");

    // Check if the file was opened successfully
    if (!input_file) {
        LOG_MESSAGE(ERROR, "Unable to open file.");
        fclose(input_file);
        return 1;
    }

    //read BMP metadata
    struct bmpmetadata bmp_metadata;
    bool is_success = read_bmp_metadata(input_file, &bmp_metadata);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Cannot read BMP metadata.");
        fclose(input_file);
        return 6;
    }

    // Check if the input file is a BMP file
    if (bmp_metadata.signature != BMP_SIGNATURE) {
        LOG_MESSAGE(ERROR, "Input file is not a BMP file.");
        fclose(input_file);
        return 2;
    }

    // Check if it's a 24-bit uncompressed BMP file
    if (bmp_metadata.bitcount != RGB_BIT_SIZE) {
        LOG_MESSAGE(ERROR, "Input file is not 24 bit RGB BMP file or there is compression.");
        fclose(input_file);
        return 2;
    }

    // Move to the start of pixel data - after only editing the color value
    fseek(input_file, bmp_metadata.dataoffset, SEEK_SET);

    //vars for bitshifts
    size_t payload_array_index = 0;
    size_t payload_element_bitshift = 0;

    size_t payload_size_bytes = UINT_MAX;

    for (int y = 0; y < bmp_metadata.height; y++) {
        //end when all payload data is hidden
        if (ptr_return_hidden_data->length >= payload_size_bytes) {
            break;
        }

        for (int i = 0; i < bmp_metadata.width; i++) {
            //end when all payload data is hidden
            if (ptr_return_hidden_data->length >= payload_size_bytes) {
                break;
            }
            colorvalue_type blue_value;
            fread(&blue_value, sizeof(colorvalue_type), 1, input_file);
            colorvalue_type green_value;
            fread(&green_value, sizeof(colorvalue_type), 1, input_file);
            colorvalue_type red_value;
            fread(&red_value, sizeof(colorvalue_type), 1, input_file);

            //save lsb to payload_array
            bool lsb_bit = get_last_bit(blue_value);
            is_success = payload_array_add_bit(ptr_return_hidden_data, lsb_bit, &payload_array_index,
                                               &payload_element_bitshift);
            if (!is_success) {
                LOG_MESSAGE(ERROR, "Cannot add bit to payload array");
                TRACKED_FREE(ptr_return_hidden_data->array);
                fclose(input_file);
                return 6;
            }

            //READ payload size from file
            if (ptr_return_hidden_data->length == PAYLOAD_SIZE_OFFSET) {
                is_success = payload_get_payloadsize(*ptr_return_hidden_data, &payload_size_bytes);
                if (!is_success) {
                    LOG_MESSAGE(ERROR, "Cannot get payload size");
                    TRACKED_FREE(ptr_return_hidden_data->array);
                    fclose(input_file);
                    return 6;
                }
            }
        }
    }

    fclose(input_file);
    return 0;
}




//region UTILS FUNCTIONS

/**
 * Frees the memory allocated by libpng.
 * @param png_read_ptr  The PNG read structure.
 * @param png_read_info_ptr  The PNG info structure.
 * @param row_pointers_size  The number of rows in the PNG image.
 * @param row_pointers  The PNG image rows.
 */
static void free_png_read_values(png_structp *png_read_ptr, png_infop *png_read_info_ptr, png_uint_32 row_pointers_size,
                                 png_bytepp row_pointers) {
    if (!png_read_ptr || !png_read_info_ptr || !row_pointers) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return;
    }

    for (int y = 0; y < row_pointers_size; y++) {
        png_free((*png_read_ptr), row_pointers[y]);
    }
    png_free((*png_read_ptr), row_pointers);
    png_destroy_read_struct(png_read_ptr, png_read_info_ptr, NULL);
}

/**
 * Frees the memory allocated by libpng.
 * @param png_read_ptr  The PNG read structure.
 * @param png_read_info_ptr  The PNG info structure.
 * @param row_pointers_size  The number of rows in the PNG image.
 * @param row_pointers  The PNG image rows.
 * @param png_write_ptr  The PNG write structure.
 * @param png_write_end_info_ptr  The PNG end info structure.
 */
static void
free_png_read_write_values(png_structp *png_read_ptr, png_infop *png_read_info_ptr, png_uint_32 row_pointers_size,
                           png_bytepp row_pointers, png_structp *png_write_ptr, png_infop *png_write_end_info_ptr) {
    if (!png_read_ptr || !png_read_info_ptr || !row_pointers || !png_write_ptr || !png_write_end_info_ptr) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return;
    }

    free_png_read_values(png_read_ptr, png_read_info_ptr, row_pointers_size, row_pointers);
    png_destroy_write_struct(png_write_ptr, png_write_end_info_ptr);
}

/**
 * Function copies payload size from payload array to ptr_return_size
 * @param array  payload array
 * @param ptr_return_size  pointer to size_t where the payload size will be copied
 * @return  true if succesfull, false otherwise
 */
static bool payload_get_payloadsize(struct payloadarray array, size_t *ptr_return_size) {
    if (!array.array || !ptr_return_size) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return false;
    }
    //Prepare ptr_return
    *ptr_return_size = 0;

    size_t size = PAYLOAD_SIZE_OFFSET;

    if (array.length < size) {
        LOG_MESSAGE(ERROR, "Payload is too small.");
        return false;
    }

    memcpy(ptr_return_size, array.array, PAYLOAD_SIZE_OFFSET);

    return true;
}

/**
 * Function reads BMP metadata from input file and saves it to metadata
 * @param input_file  input file
 * @param ptr_return_bmpmetadata  pointer to struct bmpmetadata where the metadata will be saved
 * @return  true if succesfull, false otherwise
 */
static bool read_bmp_metadata(FILE *input_file, struct bmpmetadata *ptr_return_bmpmetadata) {
    // Check if the file was opened successfully
    if (!input_file || !ptr_return_bmpmetadata) {
        LOG_MESSAGE(ERROR, "Unable to open file.");
        fclose(input_file);
        return false;
    }

    int bmp_signature_offset_bytes = BMP_SIGNATURE_OFFSET_BYTES;
    int bmp_dataoffset_offset_bytes = BMP_DATAOFFSET_OFFSET_BYTES;
    int bmp_width_offset_bytes = BMP_WIDTH_OFFSET_BYTES;
    int bmp_height_offset_bytes = BMP_HEIGHT_OFFSET_BYTES;
    int bmp_bitcount_offset_bytes = BMP_BITCOUNT_OFFSET_BYTES;

    fseek(input_file, bmp_signature_offset_bytes, SEEK_SET);
    fread(&(ptr_return_bmpmetadata->signature), sizeof(bmp_signatue_type), 1, input_file);

    fseek(input_file, bmp_dataoffset_offset_bytes, SEEK_SET);
    fread(&(ptr_return_bmpmetadata->dataoffset), sizeof(bmp_dataoffset_type), 1, input_file);

    fseek(input_file, bmp_width_offset_bytes, SEEK_SET);
    fread(&(ptr_return_bmpmetadata->width), sizeof(bmp_width_type), 1, input_file);

    fseek(input_file, bmp_height_offset_bytes, SEEK_SET);
    fread(&(ptr_return_bmpmetadata->height), sizeof(bmp_height_type), 1, input_file);

    fseek(input_file, bmp_bitcount_offset_bytes, SEEK_SET);
    fread(&(ptr_return_bmpmetadata->bitcount), sizeof(bmp_bitcount_type), 1, input_file);

    return true; // You might want to add error handling and return appropriate codes
}

/**
 * Function sets the last bit of value to change_value
 * @param change_value  value to be set to last bit
 * @param value  pointer to value where the last bit will be changed
 * @return  true if succesfull, false otherwise
 */
static bool set_last_bit(bool change_value, uint8_t *value) {
    if (value == NULL) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return false;
    }

    if (change_value == 1) {
        *value = *value | 0x01;
        return true;
    }

    //change to 0
    if (change_value == 0) {
        *value = *value & 0xFE;
        return true;
    }

    return false;
}

/**
 * This function extracts and returns the last bit of the provided ColorValueType.
 *
 * @param value The ColorValueType from which the last bit will be extracted.
 * @return The extracted last bit.
 */
static bool get_last_bit(colorvalue_type value) { return value & 0x01; }

/**
 * This function reads the signature of the input file and compares it with the PNG signature
 * to determine if the file is a PNG file.
 *
 * @param input_file A pointer to the FILE structure representing the input file.
 * @return true if the file is a PNG file, false otherwise.
 */
static bool is_png_file(FILE *input_file) {
    //Check if the arguments are valid
    if (!input_file) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return false;
    }

    // Check if the input file is a PNG file
    unsigned char signature[PNG_SIGNATURE_SIZE];
    if (fread(signature, 1, PNG_SIGNATURE_SIZE, input_file) != PNG_SIGNATURE_SIZE) {
        fclose(input_file);
        return false; // Couldn't read enough bytes
    }

    bool is_success = !png_sig_cmp(signature, 0, PNG_SIGNATURE_SIZE);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Input file is not a PNG file.");
        fclose(input_file);
        return false;
    }

    return true;
}
//endregion
//endregion

//endregion+