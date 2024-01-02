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
#define BMP_SIGNATURE "BM"

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

//region FUNCTIONS DECLARATION
#define INITIAL_PAYLOADARRAY_SIZE 1000

/**
 * Hides binary data in the Least Significant Bit (LSB) of each pixel in a PNG image.
 *
 * @param input_filepath - The path to the input PNG file.
 * @param hide_data - The binary data to hide. Each bit of the data will be embedded in the LSB of the image.
 * @param output_filepath - The path to the output PNG file.
 * @return true if the data hiding is successful, false otherwise.
 */
static bool hide_data_lsb_png(const char *input_filepath, const PayloadArray hide_data,
                              const char *output_filepath);

/**
 * Hides binary data in the least significant bit (LSB) of each RGB value in a BMP file.
 *
 * @param input_filepath - Path to the input BMP file.
 * @param binary_data - Pointer to an array of binary data to be hidden.
 * @param output_filepath - Path to the output BMP file.
 * @return 0 if the hiding process is successful, non-zero otherwise.
 */
static bool hide_data_lsb_bmp(const char *input_filepath, const PayloadArray binary_data,
                              const char *output_filepath);

/**
 * Extracts binary data from the least significant bit (LSB) of each RGB value in a BMP file.
 * @param input_image_filepath  The path to the input BMP file.
 * @param ptr_hidden_data  The path to the output BMP file.
 * @return  0 if the hiding process is successful, non-zero otherwise.
 */
static bool extract_data_lsb_png(const char *input_image_filepath, PayloadArray *ptr_hidden_data);


/**
 * Extracts binary data from the least significant bit (LSB) of each RGB value in a BMP file.
 * @param input_image_filepath  The path to the input BMP file.
 * @param ptr_hidden_data  The path to the output BMP file.
 * @return  0 if the hiding process is successful, non-zero otherwise.
 */
static bool extract_data_lsb_bmp(const char *input_image_filepath, PayloadArray *ptr_hidden_data);

static bool is_png_file(FILE *input_file);

static bool payloadarray_add_element(PayloadArray * payload_array, PayloadType element);

//endregion

//region FUNCTIONS DEFINITIONS

/**
 * Hides binary data in the least significant bit (LSB) of each RGB value in a BMP file.
 * @param input_filepath  The path to the input BMP file.
 * @param hide_data   Pointer to an array of binary data to be hidden.
 * @param output_filepath  The path to the output BMP file.
 * @return  0 if the hiding process is successful, non-zero otherwise.
 */
static bool get_next_payload_bit(PayloadArray *payload_array, size_t *array_index, size_t *element_bit_shift);

static bool get_last_bit(unsigned char value);

static bool payload_array_add_bit(PayloadArray *data, bool bit, size_t *array_index, size_t *element_bitshift);

static bool payloadarray_initialize(PayloadArray *data);

bool hide_data_lsb(const char *input_filepath, const PayloadArray hide_data,
                   const char *output_filepath) {
    //Check if the arguments are valid
    if (!input_filepath || !hide_data.array || !output_filepath) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return false;
    }

    // Check if the input file has .bmp or .png extension
    char *extension = strrchr(input_filepath, '.');
    if (!extension) {
        LOG_MESSAGE(ERROR, "Input file has no extension.");


        return false;
    }

    // Check if the input file is a BMP file
    if (strcmp(extension, ".bmp") == 0) {
        bool result = hide_data_lsb_bmp(input_filepath, hide_data, output_filepath);

        //Check if the hiding process was successful
        if (!result) {
            LOG_MESSAGE(ERROR, "Error while hiding the payload.");
            return false;
        }

    } else if (strcmp(extension, ".png") == 0) {
        bool result = hide_data_lsb_png(input_filepath, hide_data, output_filepath);

        //Check if the hiding process was successful
        if (!result) {
            LOG_MESSAGE(ERROR, "Error while hiding the payload.");
            return false;
        }
    } else {
        LOG_MESSAGE(ERROR, "Input file is not a BMP or PNG file.");


        return false;
    }

    return true;
}

bool extract_data_lsb(const char *input_image_filepath, PayloadArray *ptr_hidden_data) {
    //Check if the arguments are valid
    if (!input_image_filepath || !ptr_hidden_data) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return false;
    }

    // Check if the input file has .bmp or .png extension
    char *extension = strrchr(input_image_filepath, '.');
    if (!extension) {
        LOG_MESSAGE(ERROR, "Input file has no extension.");
        return false;
    }

    // Check if the input file is a BMP file
    if (strcmp(extension, ".bmp") == 0) {
        bool is_succes = extract_data_lsb_bmp(input_image_filepath, ptr_hidden_data);

        //Check if the extraction process was successful
        if (!is_succes) {
            LOG_MESSAGE(ERROR, "Error while extracting the payload.");
            return false;
        }

        return true;

    }

    // Check if the input file is a PNG file
    if (strcmp(extension, ".png") == 0) {
        bool result = extract_data_lsb_png(input_image_filepath, ptr_hidden_data);

        //Check if the extraction process was successful
        if (!result) {
            LOG_MESSAGE(ERROR, "Error while extracting the payload.");
            return false;
        }

        return true;
    }


    LOG_MESSAGE(ERROR, "Input file is not a BMP or PNG file.");
    return false;
}


/**
 * Hides binary data in the Least Significant Bit (LSB) of each pixel in a PNG image.
 *
 * @param input_filepath - The path to the input PNG file.
 * @param hide_data - The binary data to hide. Each bit of the data will be embedded in the LSB of the image.
 * @param output_filepath - The path to the output PNG file.
 * @return true if the data hiding is successful, false otherwise.
 */
bool hide_data_lsb_png(const char *input_filepath, const PayloadArray hide_data,
                       const char *output_filepath) {
    //todo check if binary_data fits in BMP

    //Check if the arguments are valid
    if (!input_filepath || !hide_data.array || !output_filepath) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return false;
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

        return false;
    }

    // Check if the input file is a PNG file
    bool is_success = is_png_file(input_file);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Input file is not a PNG file.");

        // Close the files
        fclose(input_file);
        fclose(output_file);

        return false;
    }

    // Create PNG read and write structures
    png_structp png_read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_structp png_write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_read_ptr || !png_write_ptr) {
        printf("Failed to create PNG read or write structure.\n");
        fclose(input_file);
        fclose(output_file);
        return false;
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
        return false;
    }

    // Set up error handling
    if (setjmp(png_jmpbuf(png_read_ptr)) || setjmp(png_jmpbuf(png_write_ptr))) {
        LOG_MESSAGE(ERROR, "Error occurred during PNG read or write operation.");
        png_destroy_read_struct(&png_read_ptr, &png_info_ptr, &png_end_info_ptr);
        png_destroy_write_struct(&png_write_ptr, &png_info_ptr);
        fclose(input_file);
        fclose(output_file);
        return false;
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

        return false;
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

    // Hide binary row_array in the LSB of each pixel
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < png_get_rowbytes(png_read_ptr, png_info_ptr); ++x) {
            row_pointers[y][x] &= 0xFE; // Clear LSB
            row_pointers[y][x] |= hide_data.array[x] & 0x01; // Set LSB according to binary row_array
            //todo fix binary_data[x] should be counter
            //todo end when end of binary row_array
        }
    }

    // Write modified PNG image row_array

    png_uint_32 height_output = png_get_image_height(png_write_ptr, png_info_ptr);
    for (int y = 0; y < height_output; ++y) {
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

    return true;
}

/**
 * Hides binary data in the least significant bit (LSB) of each RGB value in a BMP file.
 *
 * @param input_filepath - Path to the input BMP file.
 * @param binary_data - Pointer to an array of binary data to be hidden.
 * @param output_filepath - Path to the output BMP file.
 * @return 0 if the hiding process is successful, non-zero otherwise.
 */
bool hide_data_lsb_bmp(const char *input_filepath, const PayloadArray binary_data,
                       const char *output_filepath) {
    //todo check if binary_data fits in BMP

    //Check if the arguments are valid
    if (!input_filepath || !binary_data.array || !output_filepath) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return false;
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

        return false;
    }

    // Check if the input file is a BMP file
    char signature[BMP_SIGNATURE_SIZE];
    fread(signature, sizeof(char), BMP_SIGNATURE_SIZE, input_file);
    if (strcmp(signature, BMP_SIGNATURE) != 0) {
        LOG_MESSAGE(ERROR, "Input file is not a BMP file.");

        // Close the files
        fclose(input_file);
        fclose(output_file);

        return false;
    }

    //Check if the input file is 24 bit RGB PNG file
    fseek(input_file, 28, SEEK_SET);
    unsigned short bit_count;
    fread(&bit_count, sizeof(bit_count), 1, input_file);

    if (bit_count != RGB_BIT_SIZE) {
        LOG_MESSAGE(ERROR, "Input file is not 24 bit RGB PNG file.");

        // Close the files
        fclose(input_file);
        fclose(output_file);

        return false;
    }

    // Read BMP header
    unsigned char header[BMP_HEADER_SIZE];
    fread(header, sizeof(unsigned char), BMP_HEADER_SIZE, input_file);

    // Write BMP header to output file
    fwrite(header, sizeof(unsigned char), BMP_HEADER_SIZE, output_file);

    // Calculate pixel row_array offset
    int data_offset = header[10] + (header[11] << 8) + (header[12] << 16) + (header[13] << 24);

    // Seek to pixel row_array offset
    fseek(input_file, data_offset, SEEK_SET);

    // Iterate over each pixel
    unsigned char pixel[PIXEL_COLOR_NUM];
    while (fread(pixel, sizeof(unsigned char), PIXEL_COLOR_NUM, input_file) == PIXEL_COLOR_NUM) {
        // Convert RGB values to binary
        unsigned char binary_pixel[PIXEL_COLOR_NUM];
        for (int i = 0; i < PIXEL_COLOR_NUM; i++) {
            binary_pixel[i] = pixel[i] & 0xFE; // Clear LSB
            binary_pixel[i] |= binary_data.array[i] & 0x01; // Set LSB according to binary row_array
            //todo binary shift
            //todo fix binary_data[i] should be counter
            //todo end when end binary row_array
        }

        // Write modified RGB values to output file
        fwrite(binary_pixel, sizeof(unsigned char), PIXEL_COLOR_NUM, output_file);
    }

    // Close the files
    fclose(input_file);
    fclose(output_file);

    return true;
}

/**
 * Extracts binary data from the least significant bit (LSB) of each RGB value in a BMP file.
 * @param input_image_filepath  The path to the input PNG file.
 * @param ptr_hidden_data  The path to the output file.
 * @return  0 if the hiding process is successful, non-zero otherwise.
 */
static bool extract_data_lsb_png(const char *input_image_filepath, PayloadArray *ptr_hidden_data) {
    //Check if the arguments are valid
    if (!input_image_filepath || !ptr_hidden_data) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return false;
    }

    FILE *input_file = fopen(input_image_filepath, "rb");

    // Check if the file was opened successfully
    if (!input_file) {
        // Unable to open file
        LOG_MESSAGE(ERROR, "Unable to open file.");

        // Close the files
        fclose(input_file);

        return false;
    }

    // Check if the input file is a PNG file
    bool is_success = is_png_file(input_file);
    if (!is_success) {
        LOG_MESSAGE(ERROR, "Input file is not a PNG file.");

        // Close the files
        fclose(input_file);

        return false;
    }


    // Create PNG read and write structures
    png_structp png_read_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_structp png_write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_read_ptr || !png_write_ptr) {
        printf("Failed to create PNG read or write structure.\n");
        fclose(input_file);
        return false;
    }

    // Create PNG info structures
    png_infop png_info_ptr = png_create_info_struct(png_read_ptr);
    png_infop png_end_info_ptr = png_create_info_struct(png_write_ptr);

    if (!png_info_ptr || !png_end_info_ptr) {
        printf("Failed to create PNG info structure.\n");
        png_destroy_read_struct(&png_read_ptr, NULL, NULL);
        png_destroy_write_struct(&png_write_ptr, NULL);
        fclose(input_file);
        return false;
    }

    // Set up error handling
    if (setjmp(png_jmpbuf(png_read_ptr)) || setjmp(png_jmpbuf(png_write_ptr))) {
        LOG_MESSAGE(ERROR, "Error occurred during PNG read or write operation.");
        png_destroy_read_struct(&png_read_ptr, &png_info_ptr, &png_end_info_ptr);
        png_destroy_write_struct(&png_write_ptr, &png_info_ptr);
        fclose(input_file);
        return false;
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

        return false;
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

    // Hide binary row_array in the LSB of each pixel
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < png_get_rowbytes(png_read_ptr, png_info_ptr); ++x) {
            //todo write it to dynamic binary array
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

    return true;


}

static bool set_last_bit(bool change_value, unsigned char *value){
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




/**
 * Extracts binary data from the least significant bit (LSB) of each RGB value in a BMP file.
 * @param input_image_filepath  The path to the input BMP file.
 * @param binary_data  Pointer to an array of binary data to be hidden.
 * @param ptr_hidden_data  The path to the output BMP file.
 * @return  0 if the hiding process is successful, non-zero otherwise.
 */
static bool extract_data_lsb_bmp(const char *input_image_filepath, PayloadArray *ptr_hidden_data) {
    //todo check if binary_data fits in BMP

    //Check if the arguments are valid
    if (!input_image_filepath) {
        LOG_MESSAGE(ERROR, "Invalid arguments.");
        return false;
    }

    FILE *input_file = fopen(input_image_filepath, "rb");

    // Check if the file was opened successfully
    if (!input_file ) {
        // Unable to open file
        LOG_MESSAGE(ERROR, "Unable to open file.");

        // Close the files
        fclose(input_file);

        return false;
    }

    // Check if the input file is a BMP file
    char signature[BMP_SIGNATURE_SIZE];
    fread(signature, sizeof(char), BMP_SIGNATURE_SIZE, input_file);
    if (strcmp(signature, BMP_SIGNATURE) != 0) {
        LOG_MESSAGE(ERROR, "Input file is not a BMP file.");

        // Close the files
        fclose(input_file);

        return false;
    }

    //Check if the input file is 24 bit RGB PNG file
    fseek(input_file, 28, SEEK_SET);
    unsigned short bit_count;
    fread(&bit_count, sizeof(bit_count), 1, input_file);

    if (bit_count != RGB_BIT_SIZE) {
        LOG_MESSAGE(ERROR, "Input file is not 24 bit RGB PNG file.");

        // Close the files
        fclose(input_file);

        return false;
    }

    // Read BMP header
    unsigned char header[BMP_HEADER_SIZE];
    fread(header, sizeof(unsigned char), BMP_HEADER_SIZE, input_file);
    size_t payload_array_index = 0;
    size_t payload_element_bitshift = 0;

    //weight and width from header
    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
    int row_count = (width * 3 + 3) & (~3);//number of 8-bit numbers in row
    unsigned char row_array[row_count];//pointer na pole
    //unsigned char 8 bits - one color number - 24 bits for rgb triplet
    for(int i = 0; i < height; i++)
    {
        //read the whole row
        fread(row_array, sizeof(unsigned char), row_count, input_file);
        for(int j = 0; j < width*3; j += 3)
        {
            unsigned char r_value = row_array[j + 2];
            unsigned char g_value = row_array[j + 1];
            unsigned char b_value = row_array[j];

            //save lsb to payload_array
            bool lsb_bit = get_last_bit(b_value);

            bool is_succes = payload_array_add_bit(ptr_hidden_data, lsb_bit, &payload_array_index, &payload_element_bitshift);
            if (!is_succes){
                LOG_MESSAGE( ERROR, "Cannot add bit to payload array");
                return false;
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
    return true;

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

static bool get_next_payload_bit(PayloadArray *payload_array, size_t *array_index, size_t *element_bit_shift) {
    if (*array_index >= payload_array->length){
        return NULL;
    }
    if (*element_bit_shift >= sizeof(PayloadType)){
        return NULL;
    }

    bool next_bit = ((payload_array->array)[*array_index] >> *element_bit_shift) & 0x01;

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