//
// Author: Filip Cerny
// Date: 07.12.2023
// Description: Contains data structures and functions related to binary data manipulation.
//

#ifndef FAV_PC_SP_23_24_BINARYDATAINIMG_BINARY_DATA_H
#define FAV_PC_SP_23_24_BINARYDATAINIMG_BINARY_DATA_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>


/**
 *  Represents the type of data in the binarydata.
 */
typedef uint8_t binarydata_type;

/**
 * Represents an array for storing binarydata data.
 */
struct binarydataarray {
    binarydata_type *array; /**< Pointer to the binarydata data array. */
    size_t length;      /**< Current number of binarydata elements in the array. */
    size_t capacity;    /**< Maximum number of binarydata elements the array can hold. */
};


/**
 * Retrieves the next bit from a binarydata array, updating the array index and bit shift accordingly.
 *
 * @param binarydata_array Payload array to extract the bit from.
 * @param array_index Pointer to the current index in the array.
 * @param element_bit_shift Pointer to the current bit shift within the binarydata element.
 * @param ptr_return_nextbit Pointer to store the retrieved bit.
 * @return true if the operation is successful, false otherwise.
 */
bool binarydataarray_get_next_bit(struct binarydataarray binarydata_array, size_t *array_index, size_t *element_bit_shift,
                                  bool *ptr_return_nextbit);

/**
 * Adds a bit to a binarydata array, updating the array index and bit shift accordingly.
 *
 * @param data Pointer to the binarydata array.
 * @param bit Bit to be added.
 * @param array_index Pointer to the current index in the array.
 * @param element_bitshift Pointer to the current bit shift within the binarydata element.
 * @return true if the addition is successful, false otherwise.
 */
bool binarydataarray_add_bit(struct binarydataarray *data, bool bit, size_t *array_index, size_t *element_bitshift);

/**
 * Initializes a binarydata array with default values.
 *
 * @param data Pointer to the binarydata array to be initialized.
 * @return true if the initialization is successful, false otherwise.
 */
bool binarydataarray_initialize(struct binarydataarray *data);


#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_BINARY_DATA_H
