//
// Author: Lenovo
// Date: 07.11.2023
// Description: 
//

//Local includes
#include "image.h"
#include "logger.h"

//Lib includes
#include "../lib/lpng1639/png.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <zlib.h>


#define SIGNATURE "KIVPCSP_FilipCerny_hidden_payload_data"


/**
 * CHECKED
 *
 * Function to read binary data from a file and return an array of boolean values
 * @param filename  is the path to the file
 * @return
 *  a pointer to the boolean array
 *  NULL if there was an error
 */
bool *read_payload_file(const char filename) {
    //SANITY CHECK
    if (!filename) {
        return NULL;
    }

    // Open the file
    FILE *file = fopen(filename, "rb");
    if (!file) {
        log_message(ERROR, __FILE__, __LINE__, "Error: Unable to open or read file %s.", filename);
        return NULL;
    }

    // Determine the size of the file
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);

    // Allocate memory for the boolean array
    bool *data = (bool *) malloc(size * sizeof(bool));
    if (!data) {
        fclose(file);
        log_message(ERROR, __FILE__, __LINE__, "Error: Memory allocation failed.");
        return NULL;
    }

    // Read binary data and convert to boolean values
    size_t bytesRead = fread(data, sizeof(bool), size, file);
    if (bytesRead != size) {
        fclose(file);
        free(data);
        log_message(ERROR, __FILE__, __LINE__, "Error: Unable to read the entire file.");
        return NULL;
    }

    // Close the file
    fclose(file);

    return data;
}

/**
 * Function to compute the CRC32 of a boolean array
 * @param array  is the boolean array
 * @return  the CRC32
 * 0 if there was an error
 */
uint32_t compute_crc32(const bool *array) {
    //SANITY CHECK
    if (!array) {
        log_message( ERROR, __FILE__, __LINE__, "Error: Invalid array pointer.");
        return 0;
    }

    // Calculate the length of the array
    size_t length = sizeof(array) / sizeof(array[0]);

    uint32_t crc = crc32(0L, Z_NULL, 0);  // Initialize CRC32

    // Iterate through the boolean array and update CRC32
    for (size_t i = 0; i < length; ++i) {
        // Convert boolean value to a byte (0 or 1)
        uint8_t byte = array[i] ? 1 : 0;

        // Update CRC32 with the byte
        crc = crc32(crc, &byte, 1);
    }

    return crc;
}




/**
 * //todo Problem with libpng
 * Function to modify the least significant bit of the blue color channel in each RGB pixel
 * @param input_file the path to the input PNG file
 * @param output_file the path to the output PNG file
 * @return
 *  0 if successful
 *  1 if there was an error
 *  2 if the output file could not be created
 *  3 if the input file could not be opened
 *  4 if the output file could not be opened
 *  5 if an error occurred while reading the PNG file
 *  6 if the input PNG file is not in RGB color mode
 */
int hide_payload_data_png(const char *input_file, const char *output_file) {
    //todo chekc 24-bit RGB png
    //todo same errors as not magical values


    //region SANITY CHECK
    // check right png format of the input file
    if (!png_check_sig(input_file, 8)) {
        fprintf(stderr, "Error: Input PNG file is not in PNG format\n");
        return 5;
    }
    //endregion

    //open input and output files
    FILE *fp_in = fopen(input_file, "rb");
    if (!fp_in) {
        fprintf(stderr, "Error: Unable to open input file %s\n", input_file);
        return 1;
    }

    FILE *fp_out = fopen(output_file, "wb");
    if (!fp_out) {
        fclose(fp_in);
        fprintf(stderr, "Error: Unable to open output file %s\n", output_file);
        return 2;
    }


    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png) {
        fclose(fp_in);
        fclose(fp_out);
        fprintf(stderr, "Error: Unable to create read struct\n");
        return 3;
    }

    png_infop info = png_create_info_struct(png);
    if (!info) {
        png_destroy_read_struct(&png, NULL, NULL);
        fclose(fp_in);
        fclose(fp_out);
        fprintf(stderr, "Error: Unable to create info struct\n");
        return 4;
    }

    png_bytep row_pointers[1];

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp_in);
        fclose(fp_out);
        fprintf(stderr, "Error: An error occurred while reading the PNG file\n");
        return 5;
    }

    png_init_io(png, fp_in);
    png_read_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);
    png_byte bit_depth = png_get_bit_depth(png, info);
    png_byte color_type = png_get_color_type(png, info);

    if (color_type != PNG_COLOR_TYPE_RGB) {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp_in);
        fclose(fp_out);
        fprintf(stderr, "Error: Input PNG file is not in RGB color mode\n");
        return 6;
    }

    // Get image details
    int width = png_get_image_width(png, info);
    int height = png_get_image_height(png, info);
    png_bytep row = png_get_row_pointers(png, info)[0];

    if (bit_depth != 8) {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp_in);
        fclose(fp_out);
        fprintf(stderr, "Error: Unsupported bit depth. Only 8-bit depth is supported\n");
        return 7;
    }

    // Iterate through each pixel and modify the least significant bit of the blue channel
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t *pixel = &(row[(y * width + x) * 3]);
            pixel[2] |= 0x01; // Set the least significant bit of the blue channel to 1
        }
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp_in);
        fclose(fp_out);
        fprintf(stderr, "Error: An error occurred while writing the PNG file\n");
        return 8;
    }

    // Write the modified image to the output file
    png_structp write_png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!write_png) {
        png_destroy_read_struct(&png, &info, NULL);
        fclose(fp_in);
        fclose(fp_out);
        fprintf(stderr, "Error: Unable to create write struct\n");
        return 9;
    }

    png_infop write_info = png_create_info_struct(write_png);
    if (!write_info) {
        png_destroy_read_struct(&png, &info, NULL);
        png_destroy_write_struct(&write_png, NULL);
        fclose(fp_in);
        fclose(fp_out);
        fprintf(stderr, "Error: Unable to create write info struct\n");
        return 10;
    }

    png_set_compression_level(write_png, 6);

    if (setjmp(png_jmpbuf(write_png))) {
        png_destroy_read_struct(&png, &info, NULL);
        png_destroy_write_struct(&write_png, &write_info);
        fclose(fp_in);
        fclose(fp_out);
        fprintf(stderr, "Error: An error occurred while initializing the write process\n");
        return 11;
    }

    png_init_io(write_png, fp_out);

    png_set_IHDR(write_png, write_info, width, height, bit_depth, color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(write_png, write_info);

    // Write the modified image data
    for (int y = 0; y < height; y++) {
        png_write_row(write_png, row);
    }

    png_write_end(write_png, NULL);
    png_destroy_write_struct(&write_png, &write_info);

    fclose(fp_in);
    fclose(fp_out);

    printf("Image modification completed.\n");

    return 0;
}



// Function to read BMP image
//todo delete witdth and height

uint8_t *read_bmp(const char *filename, int *width, int *height) {


    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Unable to open or read BMP file %s.\n", filename);
        return NULL;
    }

    fseek(file, 18, SEEK_SET); // Move to width and height information
    fread(width, sizeof(int), 1, file);
    fread(height, sizeof(int), 1, file);

    fseek(file, 54, SEEK_SET); // Move to start of image data
    uint8_t *data = (uint8_t *) malloc((*width) * (*height) * 3);
    fread(data, sizeof(uint8_t), (*width) * (*height) * 3, file);

    fclose(file);
    return data;
}

// Function to hide payload data in the last bit of the blue channel in an RGB image
//todo delete witdth and height
int hide_payload_data_bmp(const char *image_filepath, const int width, const int height, const int *payload_data) {
    //Check right bmp format of the input file
    if (!bmp_check_sig(filename)) {
        return NULL;
    }

    uint8_t *image = read_bmp(image_filepath, &width, &height);
    if (!image) {
        return 1;
    }

    // Hide payload data in the image
    for (int i = 0; i < width * height; ++i) {
        // Modify the last bit of the blue channel
        image[i * 3 + 2] = (image[i * 3 + 2] & 0xFE) | (payload_data[i] & 0x01);
    }

    // Save the modified image
    FILE *output_file = fopen("output.bmp", "wb");
    fwrite(image, sizeof(uint8_t), width * height * 3, output_file);
    fclose(output_file);

    // Clean up
    free(image);

    return 0;
}