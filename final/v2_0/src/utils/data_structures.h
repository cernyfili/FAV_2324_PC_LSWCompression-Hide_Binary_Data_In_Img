//
// Author: Filip Cerny
// Date: 07.12.2023
// Description: Contains data structures and functions related to binary data manipulation.
//

#ifndef FAV_PC_SP_23_24_BINARYDATAINIMG_DATA_STRUCTURES_H
#define FAV_PC_SP_23_24_BINARYDATAINIMG_DATA_STRUCTURES_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>


/**
 *  Represents the type of data in the payload.
 */
typedef uint8_t payload_type;

/**
 * Represents an array for storing payload data.
 */
struct payloadarray {
    payload_type *array; /**< Pointer to the payload data array. */
    size_t length;      /**< Current number of payload elements in the array. */
    size_t capacity;    /**< Maximum number of payload elements the array can hold. */
};


/**
 * Retrieves the next bit from a payload array, updating the array index and bit shift accordingly.
 *
 * @param payload_array Payload array to extract the bit from.
 * @param array_index Pointer to the current index in the array.
 * @param element_bit_shift Pointer to the current bit shift within the payload element.
 * @param ptr_return_nextbit Pointer to store the retrieved bit.
 * @return true if the operation is successful, false otherwise.
 */
bool payloadarray_get_next_bit(struct payloadarray payload_array, size_t *array_index, size_t *element_bit_shift,
                               bool *ptr_return_nextbit);

/**
 * Adds a bit to a payload array, updating the array index and bit shift accordingly.
 *
 * @param data Pointer to the payload array.
 * @param bit Bit to be added.
 * @param array_index Pointer to the current index in the array.
 * @param element_bitshift Pointer to the current bit shift within the payload element.
 * @return true if the addition is successful, false otherwise.
 */
bool payload_array_add_bit(struct payloadarray *data, bool bit, size_t *array_index, size_t *element_bitshift);

/**
 * Initializes a payload array with default values.
 *
 * @param data Pointer to the payload array to be initialized.
 * @return true if the initialization is successful, false otherwise.
 */
bool payloadarray_initialize(struct payloadarray *data);


#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_DATA_STRUCTURES_H
