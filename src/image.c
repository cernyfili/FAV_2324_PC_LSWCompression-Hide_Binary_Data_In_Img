//
// Author: Filip Cerny
// Date: 07.11.2023
// Description: 
//

//Local includes
#include "image.h"
#include "utils/utils.h"
#include "utils/dictionary.h"

//Lib includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <png.h>


/**
 * The size of the PNG signature in bytes.
 */
#define PNG_SIGNATURE_SIZE 8

/**
 * The size of the BMP signature in bytes.
 */
#define BMP_SIGNATURE_SIZE 2

/**
 * The BMP signature.
 */
#define BMP_SIGNATURE 0x4D42

/**
 * The size of the BMP header in bytes.
 */
#define BMP_HEADER_SIZE 54

/**
 * The number of color components per pixel in a BMP image.
 */
#define PIXEL_COLOR_NUM 3

/**
 * The number of bits per pixel in a BMP image for 24-bit RGB color depth.
 */
#define RGB_BIT_SIZE 24

#define INITIAL_PAYLOADARRAY_SIZE 1000

//region STRUCTURES
typedef struct {
    uint16_t signature;
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t dataOffset;
} BMPHeader;

typedef struct  {
    uint32_t headerSize;
    int32_t width;
    int32_t height;
    uint16_t colorPlanes;
    uint16_t bitsPerPixel;
    uint32_t compressionMethod;
    uint32_t imageSize;
    int32_t horizontalResolution;
    int32_t verticalResolution;
    uint32_t numColorsInPalette;
    uint32_t numImportantColors;
} DIBHeader;
//endregion

//region FUNCTIONS DECLARATION

/**
 * Hides binary data in the Least Significant Bit (LSB) of each pixel in a PNG image.
 *
 * @param input_filepath - The path to the input PNG file.
 * @param hide_data - The binary data to hide. Each bit of the data will be embedded in the LSB of the image.
 * @param output_filepath - The path to the output PNG file.
 * @return true if the data hiding is successful, false otherwise.
 */
static int hide_data_lsb_png(const char *input_filepath, const PayloadArray hide_data,
                             const char *output_filepath);

/**
 * Hides binary data in the least significant bit (LSB) of each RGB value in a BMP file.
 *
 * @param input_filepath - Path to the input BMP file.
 * @param payload_data - Pointer to an array of binary data to be hidden.
 * @param output_filepath - Path to the output BMP file.
 * @return 0 if the hiding process is successful, non-zero otherwise.
 */
static int hide_data_lsb_bmp(const char *input_filepath, PayloadArray payload_data,
                             const char *output_filepath);

/**
 * Extracts binary data from the least significant bit (LSB) of each RGB value in a BMP file.
 * @param input_image_filepath  The path to the input BMP file.
 * @param ptr_hidden_data  The path to the output BMP file.
 * @return  0 if the hiding process is successful, non-zero otherwise.
 */
static int extract_data_lsb_png(const char *input_image_filepath, PayloadArray *ptr_hidden_data);


/**
 * Extracts binary data from the least significant bit (LSB) of each RGB value in a BMP file.
 * @param input_image_filepath  The path to the input BMP file.
 * @param ptr_hidden_data  The path to the output BMP file.
 * @return  0 if the hiding process is successful, non-zero otherwise.
 */
static int extract_data_lsb_bmp(const char *input_image_filepath, PayloadArray *ptr_hidden_data);

static bool is_png_file(FILE *input_file);

static bool payloadarray_add_element(PayloadArray * payload_array, PayloadType element);

static bool set_last_bit(bool change_value, uint8_t *value);


/**
 * Hides binary data in the least significant bit (LSB) of each RGB value in a BMP file.
 * @param input_filepath  The path to the input BMP file.
 * @param hide_data   Pointer to an array of binary data to be hidden.
 * @param output_filepath  The path to the output BMP file.
 * @return  0 if the hiding process is successful, non-zero otherwise.
 */
static bool get_next_payload_bit(PayloadArray payload_array, size_t *array_index, size_t *element_bit_shift);

static bool get_last_bit(unsigned char value);

static bool payload_array_add_bit(PayloadArray *data, bool bit, size_t *array_index, size_t *element_bitshift);

static bool payloadarray_initialize(PayloadArray *data);
//endregion

//region FUNCTIONS DEFINITIONS

/**
 *
 * @param input_filepath
 * @param hide_data
 * @param output_filepath
 * @return
 * 0 succesfull
 * 2 if input file is not BMP or PNG
 * 3 if payload is too big for image
 * 6 other error
 */
int hide_data_lsb(const char *input_filepath, const PayloadArray hide_data,
                   const char *output_filepath) {
    //Check if the arguments are valid
    if (!input_filepath || !hide_data.array || !output_filepath) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return 6;
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
 * @param input_image_filepath
 * @param ptr_hidden_data
 * @return
 * 0 succesfull
 * 1 cannot open file
 * 2 if input file is not BMP or PNG; not 24 bit RGB
 * 6 other error
 */
int extract_data_lsb(const char *input_image_filepath, PayloadArray *ptr_hidden_data) {
    //Check if the arguments are valid
    if (!input_image_filepath || !ptr_hidden_data) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return 6;
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
        result = extract_data_lsb_bmp(input_image_filepath, ptr_hidden_data);

        //Check if the extraction process was successful
        if (result != 0) {
            LOG_MESSAGE(ERROR, "Error while extracting the payload.");
            return result;
        }

        return true;

    }

    // Check if the input file is a PNG file
    if (strcmp(extension, ".png") == 0) {
        result = extract_data_lsb_png(input_image_filepath, ptr_hidden_data);

        //Check if the extraction process was successful
        if (result != 0) {
            LOG_MESSAGE(ERROR, "Error while extracting the payload.");
            return result;
        }

        return true;
    }


    LOG_MESSAGE(ERROR, "Input file is not a BMP or PNG file.");
    return 2;
}


/**
 * Hides binary data in the Least Significant Bit (LSB) of each pixel in a PNG image.
 *
 * @param input_filepath - The path to the input PNG file.
 * @param hide_data - The binary data to hide. Each bit of the data will be embedded in the LSB of the image.
 * @param output_filepath - The path to the output PNG file.
 * @return
 * 0 succesfull
 * 2 if input file is not BMP; not 24 bit RGB
 * 3 if payload is too big for image
 * 6 other error
 */
int hide_data_lsb_png(const char *input_filepath, const PayloadArray hide_data,
                       const char *output_filepath) {
    //todo check if binary_data fits in BMP

    //Check if the arguments are valid
    if (!input_filepath || !hide_data.array || !output_filepath) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return 6;
    }

    if (strcmp(input_filepath, output_filepath) == 0) {
        LOG_MESSAGE(ERROR, "Input and output filepaths are the same.");
        return 6;
    }

    FILE *input_file = fopen(input_filepath, "rb");
    FILE *output_file = fopen(output_filepath, "wb");

    // Check if the file was opened successfully
    if (!input_file || !output_file) {
        // Unable to open file
        LOG_MESSAGE(ERROR, "Unable to open file.");

        // Close the files
        fclose(input_file);
        fclose(output_file);

        return 1;
    }

    // Check if the input file is a PNG file
    bool is_success = is_png_file(input_file);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Input file is not a PNG file.");

        // Close the files
        fclose(input_file);
        fclose(output_file);

        return 2;
    }

    // Create PNG read and write structures
    png_structp png_read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_structp png_write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_read_ptr || !png_write_ptr) {
        printf("Failed to create PNG read or write structure.\n");
        fclose(input_file);
        fclose(output_file);
        return 6;
    }

    // Create PNG info structures
    png_infop png_info_ptr = png_create_info_struct(png_read_ptr);
    png_infop png_end_info_ptr = png_create_info_struct(png_write_ptr);

    if (!png_info_ptr || !png_end_info_ptr) {
        printf("Failed to create PNG info structure.\n");
        png_destroy_read_struct(&png_read_ptr, NULL, NULL);
        png_destroy_write_struct(&png_write_ptr, NULL);
        fclose(input_file);
        fclose(output_file);
        return 6;
    }

    // Set up error handling
    if (setjmp(png_jmpbuf(png_read_ptr)) || setjmp(png_jmpbuf(png_write_ptr))) {
        LOG_MESSAGE(ERROR, "Error occurred during PNG read or write operation.");
        png_destroy_read_struct(&png_read_ptr, &png_info_ptr, &png_end_info_ptr);
        png_destroy_write_struct(&png_write_ptr, &png_info_ptr);
        fclose(input_file);
        fclose(output_file);
        return 6;
    }

    // Initialize PNG IO
    png_init_io(png_read_ptr, input_file);
    png_init_io(png_write_ptr, output_file);

    // Read PNG header
    png_read_info(png_read_ptr, png_info_ptr);

    //Check if the input file is 24-bit RGB PNG file
    int color_type = png_get_color_type(png_read_ptr, png_info_ptr);
    if (color_type != PNG_COLOR_TYPE_RGB) {
        LOG_MESSAGE(ERROR, "Input file is not a 24-bit RGB PNG image.");

        //Clean up
        png_destroy_read_struct(&png_read_ptr, &png_info_ptr, &png_end_info_ptr);
        png_destroy_write_struct(&png_write_ptr, &png_info_ptr);

        //Close the files
        fclose(input_file);
        fclose(output_file);

        return 2;
    }

    // Write PNG header
    png_write_info(png_write_ptr, png_info_ptr);


    // Get image dimensions
    png_uint_32 height = png_get_image_height(png_read_ptr, png_info_ptr);

    // Allocate memory for pixel row_array
    png_bytepp row_pointers = (png_bytepp) png_malloc(png_read_ptr, height *
                                                                    sizeof(png_bytep));

    // Read PNG image row_array
    for (int y = 0; y < height; ++y) {
        row_pointers[y] = (png_bytep) png_malloc(png_read_ptr, png_get_rowbytes(png_read_ptr, png_info_ptr));
        png_read_row(png_read_ptr, row_pointers[y], NULL);
    }

    //vars for bitshifts
    size_t payload_array_index = 0;
    size_t payload_element_bitshift = 0;

    // Hide binary row_array in the LSB of each pixel
    for (int y = 0; y < height; ++y) {
        size_t row_bytes = png_get_rowbytes(png_read_ptr, png_info_ptr);
        for (int x = 0; x < row_bytes; x += 3) {
            unsigned char * ptr_blue_value = &row_pointers[y][x + 2];

            //change last bit of b_value to ptr_hidden_data
            bool next_payload_bit = get_next_payload_bit(hide_data, &payload_array_index, &payload_element_bitshift);

            bool is_succes = set_last_bit(next_payload_bit, ptr_blue_value);
            if (!is_succes){
                LOG_MESSAGE( ERROR, "Cannot change last bit");

                for (int z = 0; z < height; ++z) {
                    png_free(png_read_ptr, row_pointers[z]);
                }
                png_free(png_read_ptr, row_pointers);
                png_destroy_read_struct(&png_read_ptr, &png_info_ptr, &png_end_info_ptr);
                png_destroy_write_struct(&png_write_ptr, &png_info_ptr);
                return 6;
            }
        }
    }

    // Write modified PNG image row_array

    png_uint_32 height_output = png_get_image_height(png_write_ptr, png_info_ptr);
    for (int y = 0; y < height_output; ++y) {//todo check if not problem
        png_write_row(png_write_ptr, row_pointers[y]);
    }

    // Write end of PNG
    png_write_end(png_write_ptr, png_end_info_ptr);


    // Clean up
    for (int y = 0; y < height; ++y) {
        png_free(png_read_ptr, row_pointers[y]);
    }
    png_free(png_read_ptr, row_pointers);
    png_destroy_read_struct(&png_read_ptr, &png_info_ptr, &png_end_info_ptr);
    png_destroy_write_struct(&png_write_ptr, &png_info_ptr);


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
int hide_data_lsb_bmp(const char *input_filepath, PayloadArray payload_data,
                       const char *output_filepath) {
    //todo check if binary_data fits in BMP

    //Check if the arguments are valid
    if (!input_filepath || !payload_data.array || !output_filepath) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return 6;
    }

    if (strcmp(input_filepath, output_filepath) == 0) {
        LOG_MESSAGE(ERROR, "Input and output filepaths are the same.");
        return 6;
    }

    FILE *input_file = fopen(input_filepath, "rb");
    FILE *output_file = fopen(output_filepath, "wb");

    // Check if the file was opened successfully
    if (!input_file || !output_file) {
        // Unable to open file
        LOG_MESSAGE(ERROR, "Unable to open file.");

        // Close the files
        fclose(input_file);
        fclose(output_file);

        return 1;
    }

    // Check if the input file is a BMP file
    // Read BMP header
    BMPHeader bmp_header;
    fread(&bmp_header, sizeof(BMPHeader), 1, input_file);
    if (bmp_header.signature != BMP_SIGNATURE) {
        LOG_MESSAGE(ERROR, "Input file is not a BMP file.");

        // Close the files
        fclose(input_file);
        fclose(output_file);

        return 2;
    }

    // Read DIB header
    DIBHeader dib_header;
    fread(&dib_header, sizeof(DIBHeader), 1, input_file);

    // Check if it's a 24-bit uncompressed BMP file
    if (dib_header.colorPlanes != RGB_BIT_SIZE || dib_header.compressionMethod != 0) {
        LOG_MESSAGE(ERROR, "Input file is not 24 bit RGB BMP file or there is compression.");

        // Close the files
        fclose(input_file);
        fclose(output_file);

        return 2;
    }

    // Write BMP header to output file
    fwrite(&bmp_header, sizeof(BMPHeader), 1, output_file);

    // Write DIB BMP header to output file
    fwrite(&dib_header, sizeof(DIBHeader), 1, output_file);

    //vars for bitshifts
    size_t payload_array_index = 0;
    size_t payload_element_bitshift = 0;

    //weight and width from header
    int width = dib_header.width;
    int height = dib_header.height;


    //Check if payload fits to image
    double pixel_count = width * (double )height;
    int bits_in_payload = (int)((double) payload_data.length * 8.0);
    if (pixel_count < bits_in_payload){
        LOG_MESSAGE(ERROR, "Payload is too big for image.");
        return 3;
    }

    //region Array for one row
    size_t pixel_arr_length = NUM_COLOR_PIXEL;

    // Move to the start of pixel data
    fseek(input_file, bmp_header.dataOffset, SEEK_SET);

    int bytes_per_row = width * NUM_COLOR_PIXEL;
    uint8_t* row_array = TRACKED_MALLOC(bytes_per_row * sizeof(uint8_t));
    if (!row_array){
        LOG_MESSAGE(ERROR, "Cannot allocate memory for row array");
        return 6;
    }

    for(int y = 0; y < height; ++y)
    {
        printf("row %d\n", y);
        //read the whole row
        fread(row_array, sizeof(uint8_t), bytes_per_row, input_file);
        for(int x = 0; x < bytes_per_row; x += NUM_COLOR_PIXEL)
        {
            uint8_t * ptr_blue_value = &row_array[x];//becouse bmp B G R

            //change last bit of b_value to ptr_hidden_data
            bool next_payload_bit = get_next_payload_bit(payload_data, &payload_array_index, &payload_element_bitshift);

            bool is_succes = set_last_bit(next_payload_bit, ptr_blue_value);
            if (!is_succes){
                LOG_MESSAGE( ERROR, "Cannot change last bit");
                TRACKED_FREE(row_array);
                return 6;
            }

        }

        //write whole row to output file
        fwrite(row_array, sizeof(uint8_t), bytes_per_row, output_file);
    }
    //endregion

    //region Array for whole image - too big
    /*size_t pixel_arr_length = NUM_COLOR_PIXEL;
    *//*uint8_t pixel[pixel_arr_length];*//*
    // Move to the start of pixel data
    fseek(input_file, bmp_header.dataOffset, SEEK_SET);
    int bytes_per_row = width * NUM_COLOR_PIXEL;
    *//*uint8_t* row_array = TRACKED_MALLOC(bytes_per_row * sizeof(uint8_t));*//*
    //todo save modified img to array and then to file

    //array with all the pixels
    size_t pixel_array_length = (size_t)height * (size_t)bytes_per_row;
    uint8_t * pixel_array = TRACKED_MALLOC(pixel_array_length * sizeof(uint8_t));
    if (!pixel_array){
        LOG_MESSAGE(ERROR, "Cannot allocate memory for pixel array");
        return 6;
    }

    fread(pixel_array, sizeof(uint8_t), pixel_array_length, input_file);

    for(int y = 0; y < height; y++) {
        printf("row %d\n", y);

        for(int x = 0; x < bytes_per_row; x++) {
            //skip other colors than blue
            if (x % 3 != 0){
                break;
            }

            size_t index = y * bytes_per_row + x;
            uint8_t * ptr_blue_value = &(pixel_array[index]);//becouse bmp B G R

            //change last bit of b_value to ptr_hidden_data
            bool next_payload_bit = get_next_payload_bit(payload_data, &payload_array_index, &payload_element_bitshift);

            bool is_succes = set_last_bit(next_payload_bit, ptr_blue_value);
            if (!is_succes){
                LOG_MESSAGE( ERROR, "Cannot change last bit");
                *//*TRACKED_FREE(pixel_array);*//*
                return 6;
            }

        }

    }

    //save the pixel array to file
    fwrite(pixel_array, sizeof(uint8_t), height * bytes_per_row, output_file);*/
    //endregion

    //region file read in every pixel
    /*for(int y = 0; y < height; ++y)
    {
        //read the whole row
        *//*fread(row_array, sizeof(unsigned char), row_bytes, input_file);*//*
        for(int x = 0; x < width; ++x)
        {
            fread(pixel, sizeof(pixel[0]), pixel_arr_length, input_file);
            *//*unsigned char r_value = row_array[j + 2];
            unsigned char g_value = row_array[j + 1];*//*
            *//*unsigned char* ptr_blue_value = &row_array[x];*//*
            uint8_t * ptr_blue_value = &pixel[0];//becouse bmp B G R

            //change last bit of b_value to ptr_hidden_data
            bool next_payload_bit = get_next_payload_bit(payload_data, &payload_array_index, &payload_element_bitshift);

            bool is_succes = set_last_bit(next_payload_bit, ptr_blue_value);
            if (!is_succes){
                LOG_MESSAGE( ERROR, "Cannot change last bit");
                return 6;
            }

            fwrite(pixel, sizeof(pixel[0]), pixel_arr_length, output_file);

        }
*//*
        //write whole row to output file
        fwrite(row_array, sizeof(unsigned char), row_bytes, output_file);*//*
    }*/
    //endregion

    // Close the files
    fclose(input_file);
    fclose(output_file);

    /*TRACKED_FREE(row_array);*/

    return 0;
}

/**
 * Extracts binary data from the least significant bit (LSB) of each RGB value in a BMP file.
 * @param input_image_filepath  The path to the input PNG file.
 * @param ptr_hidden_data  The path to the output file.
 * @return
 * 0 succesfull
 * 1 cannot open file
 * 2 if input file is not BMP or PNG; not 24 bit RGB
 * 6 other error
 */
static int extract_data_lsb_png(const char *input_image_filepath, PayloadArray *ptr_hidden_data) {
    //Check if the arguments are valid
    if (!input_image_filepath || !ptr_hidden_data) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return 6;
    }

    FILE *input_file = fopen(input_image_filepath, "rb");

    // Check if the file was opened successfully
    if (!input_file) {
        // Unable to open file
        LOG_MESSAGE(ERROR, "Unable to open file.");

        // Close the files
        fclose(input_file);

        return 1;
    }

    // Check if the input file is a PNG file
    bool is_success = is_png_file(input_file);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Input file is not a PNG file.");

        // Close the files
        fclose(input_file);

        return 2;
    }


    // Create PNG read and write structures
    png_structp png_read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_structp png_write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_read_ptr || !png_write_ptr) {
        printf("Failed to create PNG read or write structure.\n");
        fclose(input_file);
        return 6;
    }

    // Create PNG info structures
    png_infop png_info_ptr = png_create_info_struct(png_read_ptr);
    png_infop png_end_info_ptr = png_create_info_struct(png_write_ptr);

    if (!png_info_ptr || !png_end_info_ptr) {
        printf("Failed to create PNG info structure.\n");
        png_destroy_read_struct(&png_read_ptr, NULL, NULL);
        png_destroy_write_struct(&png_write_ptr, NULL);
        fclose(input_file);
        return 6;
    }

    // Set up error handling
    if (setjmp(png_jmpbuf(png_read_ptr)) || setjmp(png_jmpbuf(png_write_ptr))) {
        LOG_MESSAGE(ERROR, "Error occurred during PNG read or write operation.");
        png_destroy_read_struct(&png_read_ptr, &png_info_ptr, &png_end_info_ptr);
        png_destroy_write_struct(&png_write_ptr, &png_info_ptr);
        fclose(input_file);
        return 6;
    }

    // Initialize PNG IO
    png_init_io(png_read_ptr, input_file);

    // Read PNG header
    png_read_info(png_read_ptr, png_info_ptr);

    //Check if the input file is 24-bit RGB PNG file
    int color_type = png_get_color_type(png_read_ptr, png_info_ptr);
    if (color_type != PNG_COLOR_TYPE_RGB) {
        LOG_MESSAGE(ERROR, "Input file is not a 24-bit RGB PNG image.");

        //Clean up
        png_destroy_read_struct(&png_read_ptr, &png_info_ptr, &png_end_info_ptr);
        png_destroy_write_struct(&png_write_ptr, &png_info_ptr);

        //Close the files
        fclose(input_file);

        return 2;
    }

    // Write PNG header
    png_write_info(png_write_ptr, png_info_ptr);


    // Get image dimensions
    png_uint_32 height = png_get_image_height(png_read_ptr, png_info_ptr);

    // Allocate memory for pixel row_array
    png_bytepp row_pointers = (png_bytepp) png_malloc(png_read_ptr, height *
                                                                    sizeof(png_bytep));

    // Read PNG image row_array
    for (int y = 0; y < height; ++y) {
        row_pointers[y] = (png_bytep) png_malloc(png_read_ptr, png_get_rowbytes(png_read_ptr, png_info_ptr));
        png_read_row(png_read_ptr, row_pointers[y], NULL);
    }


    //vars for bitshifts
    size_t payload_array_index = 0;
    size_t payload_element_bitshift = 0;

    // Extract binary row_array in the LSB of each pixel
    for (int y = 0; y < height; ++y) {
        size_t row_bytes = png_get_rowbytes(png_read_ptr, png_info_ptr);
        for (int x = 0; x < row_bytes; x += 3) {
            unsigned char blue_value = row_pointers[y][x + 2];

            // Extract the LSB of the pixel value
            bool lsb_bit = get_last_bit(blue_value);

            // Write the LSB to the dynamic binary array
            is_success = payload_array_add_bit(ptr_hidden_data, lsb_bit, &payload_array_index, &payload_element_bitshift);
            if (!is_success) {
                LOG_MESSAGE(ERROR, "Cannot add bit to dynamic binary array.");
                // Clean up
                TRACKED_FREE(ptr_hidden_data->array);
                for (int z = 0; z < height; ++z) {
                    png_free(png_read_ptr, row_pointers[z]);
                }
                png_free(png_read_ptr, row_pointers);
                png_destroy_read_struct(&png_read_ptr, &png_info_ptr, &png_end_info_ptr);
                png_destroy_write_struct(&png_write_ptr, &png_info_ptr);
                fclose(input_file);
                return 6;
            }
        }
    }


    // Clean up
    for (int y = 0; y < height; ++y) {
        png_free(png_read_ptr, row_pointers[y]);
    }
    png_free(png_read_ptr, row_pointers);
    png_destroy_read_struct(&png_read_ptr, &png_info_ptr, &png_end_info_ptr);
    png_destroy_write_struct(&png_write_ptr, &png_info_ptr);


    // Close the files
    fclose(input_file);

    return 0;
}

/**
 * Extracts binary data from the least significant bit (LSB) of each RGB value in a BMP file.
 * @param input_image_filepath  The path to the input BMP file.
 * @param binary_data  Pointer to an array of binary data to be hidden.
 * @param ptr_hidden_data  The path to the output BMP file.
 * @return
 * 0 succesfull
 * 1 cannot open file
 * 2 if input file is not BMP or PNG; not 24 bit RGB
 * 6 other error
 */
static int extract_data_lsb_bmp(const char *input_image_filepath, PayloadArray *ptr_hidden_data) {
    //todo check if binary_data fits in BMP

    //Check if the arguments are valid
    if (!input_image_filepath) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return 6;
    }

    FILE *input_file = fopen(input_image_filepath, "rb");

    // Check if the file was opened successfully
    if (!input_file ) {
        // Unable to open file
        LOG_MESSAGE(ERROR, "Unable to open file.");

        // Close the files
        fclose(input_file);

        return 1;
    }

    // Check if the input file is a BMP file
    char signature[BMP_SIGNATURE_SIZE];
    fread(signature, sizeof(char), BMP_SIGNATURE_SIZE, input_file);
    if (/*strcmp(signature, BMP_SIGNATURE) != 0*/ true) {//todo change
        LOG_MESSAGE(ERROR, "Input file is not a BMP file.");

        // Close the files
        fclose(input_file);

        return 2;
    }

    //Check if the input file is 24 bit RGB PNG file
    fseek(input_file, 28, SEEK_SET);
    unsigned short bit_count;
    fread(&bit_count, sizeof(bit_count), 1, input_file);

    if (bit_count != RGB_BIT_SIZE) {
        LOG_MESSAGE(ERROR, "Input file is not 24 bit RGB PNG file.");

        // Close the files
        fclose(input_file);

        return 2;
    }

    // Read BMP header
    unsigned char header[BMP_HEADER_SIZE];
    fread(header, sizeof(unsigned char), BMP_HEADER_SIZE, input_file);

    //vars for bitshifts
    size_t payload_array_index = 0;
    size_t payload_element_bitshift = 0;

    //weight and width from header
    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
    int row_bytes = (width * 3 + 3) & (~3);//number of 8-bit numbers in row
    unsigned char row_array[row_bytes];//pointer na pole
    //unsigned char 8 bits - one color number - 24 bits for rgb triplet
    for(int i = 0; i < height; i++)
    {
        //read the whole row
        fread(row_array, sizeof(unsigned char), row_bytes, input_file);
        for(int j = 0; j < row_bytes; j += 3)
        {
            unsigned char r_value = row_array[j + 2];
            unsigned char g_value = row_array[j + 1];
            unsigned char b_value = row_array[j];

            //save lsb to payload_array
            bool lsb_bit = get_last_bit(b_value);

            bool is_succes = payload_array_add_bit(ptr_hidden_data, lsb_bit, &payload_array_index, &payload_element_bitshift);
            if (!is_succes){
                LOG_MESSAGE( ERROR, "Cannot add bit to payload array");
                return 6;
            }

            /*//change last bit of b_value to ptr_hidden_data
            bool next_payload_bit = get_next_payload_bit(ptr_hidden_data, &payload_array_index, &payload_element_index);

            bool is_succes = set_last_bit(next_payload_bit, &b_value);
            if (!is_succes){
                LOG_MESSAGE( ERROR, "Cannot change last bit");
                return false;
            }*/

        }
    }
    fclose(input_file);
    return 0;

}


//region UTILS FUNCTIONS
static bool set_last_bit(bool change_value, uint8_t *value){
    if (value == NULL){
        LOG_MESSAGE(ERROR, "Value is NULL.");
        return false;
    }

    if (change_value == 1){
        *value = *value | 0x01;
        return true;
    }

    //change to 0
    if (change_value == 0){
        *value = *value & 0xFE;
        return true;
    }

    return false;
}

static bool payload_array_add_bit(PayloadArray *data, bool bit, size_t *array_index, size_t *element_bitshift) {
    bool is_succes;

    // Ensure the payload array is initialized
    if (!data || !data->array) {
        LOG_MESSAGE(ERROR, "Payload array is not initialized.");
        return false;
    }

    // Ensure dynamic array is initialized
    if (data->length == 0) {
        is_succes = payloadarray_initialize(data);
        if (!is_succes) {
            LOG_MESSAGE(ERROR, "Failed to initialize dynamic array for payload.");
            return false;
        }
    }

    // Ensure valid array index
    if (*array_index >= data->length) {
        // Need to add a new element to the dynamic array
        PayloadType new_element = 0;
        is_succes = payloadarray_add_element(data, new_element);
        if (!is_succes) {
            LOG_MESSAGE(ERROR, "Failed to add new element to payload array.");
            return false;
        }
    }

    // Set the bit in the current element
    if (bit) {
        data->array[*array_index] |= (1 << *element_bitshift);
    } else {
        data->array[*array_index] &= ~(1 << *element_bitshift);
    }

    // Update array index and bit shift for the next iteration
    if (*element_bitshift == (sizeof(PayloadType) - 1)) {
        // Move to the next element in the array
        (*array_index)++;
        *element_bitshift = 0;
    } else {
        // Move to the next bit in the current element
        (*element_bitshift)++;
    }

    return true;

}

static bool payloadarray_initialize(PayloadArray *data) {
    data->length = 0;
    data->array = TRACKED_MALLOC(sizeof(PayloadType) * INITIAL_PAYLOADARRAY_SIZE);
    if (!data->array){
        LOG_MESSAGE(ERROR, "Failed to allocate memory for payload array.");
        return false;
    }
    data->capacity = INITIAL_PAYLOADARRAY_SIZE;

    return true;
}

static bool payloadarray_add_element(PayloadArray * payload_array, PayloadType element){
    return dynamicarray_add_element((DynamicArray *) payload_array, &element, sizeof(PayloadType));
}


static bool get_last_bit(unsigned char value) { return value & 0x01; }

static bool get_next_payload_bit(PayloadArray payload_array, size_t *array_index, size_t *element_bit_shift) {
    if (*array_index >= payload_array.length){
        return NULL;
    }
    if (*element_bit_shift >= sizeof(PayloadType)){
        return NULL;
    }

    bool next_bit = ((payload_array.array)[*array_index] >> *element_bit_shift) & 0x01;

    if (*element_bit_shift == (sizeof(PayloadType) - 1)){
        *element_bit_shift = 0;
    } else{
        (*element_bit_shift)++;
    }

    (*array_index)++;

    return next_bit;
}


static bool is_png_file(FILE *input_file) {// Check if the input file is a PNG file
    unsigned char signature[PNG_SIGNATURE_SIZE];
    fread(signature, 1, PNG_SIGNATURE_SIZE, input_file);
    bool is_success = !png_sig_cmp(signature, 0, PNG_SIGNATURE_SIZE);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Input file is not a PNG file.");

        return false;
    }

    return true;
}
//endregion

//endregion