//
// Author: Filip Cerny
// Date: 07.12.2023
// Description: This file contains functions for manipulating binary data.
//

#include "binary_data.h"
#include "utils.h"

/**
 * Represents the initial size of the binarydata array.
 */
#define INITIAL_BINARYDATAARRAY_SIZE 1000
/**
 * Represents the increment of the binarydata array size.
 */
#define DIC_ARR_INC 1000


/**
 * Represents a dynamic array.
 */
struct dynamicarray {
    void *array;     /**< Pointer to the array data. */
    size_t length;   /**< Current number of elements in the array. */
    size_t capacity; /**< Maximum number of elements the array can hold. */
};

//region FUNCTION DECLARATIONS

/**
 * Adds an element to a dynamic array, ensuring capacity and handling memory allocation.
 *
 * @param dynamic_array Pointer to the dynamic array.
 * @param element Pointer to the element to be added.
 * @param element_size Size of the element in bytes.
 * @return true if the addition is successful, false otherwise.
 */
static bool dynamicarray_add_element(struct dynamicarray *dynamic_array, void *element, size_t element_size);


/**
 * Adds an element to a binarydata array, ensuring capacity and handling memory allocation.
 *
 * @param binarydata_array Pointer to the binarydata array.
 * @param element Payload element to be added.
 * @return true if the addition is successful, false otherwise.
 */
static bool binarydataarray_add_element(struct binarydataarray *binarydata_array, binarydata_type element);
//endregion

//region FUNCTION DEFINITIONS
//region PRIVATE FUNCTIONS
/**
 * Adds an element to a generic dynamic array.
 *
 * @param dynamic_array The struct dynamicarray to which the element is added.
 * @param element Pointer to the element to be added.
 * @param element_size Size of the element.
 * @return True if the addition process is successful, false otherwise.
 */
static bool dynamicarray_add_element(struct dynamicarray *dynamic_array, void *element, size_t element_size) {
    if (!dynamic_array || !element || element_size == 0) {
        LOG_MESSAGE(ERROR, "Wrong argument");
        return false;
    }

    //if value doesnt fit
    if (dynamic_array->length == dynamic_array->capacity) {
        size_t new_capacity = dynamic_array->capacity + DIC_ARR_INC;

        void *temp_ptr = realloc(dynamic_array->array, new_capacity * element_size);
        if (temp_ptr == NULL) {
            LOG_MESSAGE(ERROR, "Memory allocation failed.");
            TRACKED_FREE(dynamic_array->array);

            return false; // Memory allocation failed
        }

        dynamic_array->array = temp_ptr;
        dynamic_array->capacity = new_capacity;
        temp_ptr = NULL;
    }

    memcpy(dynamic_array->array + dynamic_array->length * element_size, element, element_size);
    (dynamic_array->length)++;

    return true;
}


/**
 * Adds an element to a struct binarydataarray.
 *
 * @param binarydata_array The struct binarydataarray to which the element is added.
 * @param element The element to be added.
 * @return True if the addition process is successful, false otherwise.
 */
static bool binarydataarray_add_element(struct binarydataarray *binarydata_array, binarydata_type element) {
    if (!binarydata_array || !binarydata_array->array) {
        LOG_MESSAGE(ERROR, "Argument is NULL!");
        return false;
    }
    //if value doesnt fit
    if (binarydata_array->length == binarydata_array->capacity) {
        size_t new_capacity = binarydata_array->capacity + DIC_ARR_INC;

        void *temp_ptr = TRACKED_MALLOC(new_capacity * sizeof(binarydata_type));
        if (temp_ptr == NULL) {
            LOG_MESSAGE(ERROR, "Memory allocation failed.");
            TRACKED_FREE(binarydata_array->array);
            return false; // Memory allocation failed
        }
        memcpy(temp_ptr, binarydata_array->array, binarydata_array->capacity * sizeof(binarydata_type));
        TRACKED_FREE(binarydata_array->array);
        binarydata_array->array = temp_ptr;
        binarydata_array->capacity = new_capacity;
        temp_ptr = NULL;
    }

    binarydata_array->array[binarydata_array->length] = element;
    (binarydata_array->length)++;

    return true;

    /*
    return dynamicarray_add_element((struct dynamicarray *) binarydata_array, &element, sizeof(binarydata_type));*/
}


//endregion


/**
 * Initializes a struct binarydataarray.
 *
 * @param data The struct binarydataarray to be initialized.
 * @return True if the initialization process is successful, false otherwise.
 */
bool binarydataarray_initialize(struct binarydataarray *data) {
    if (!data) {
        LOG_MESSAGE(ERROR, "struct binarydataarray is NULL!");
        return false;
    }

    data->length = 0;
    data->array = TRACKED_MALLOC(sizeof(binarydata_type) * INITIAL_BINARYDATAARRAY_SIZE);
    if (!data->array) {
        LOG_MESSAGE(ERROR, "Failed to allocate memory for binarydata array.");
        return false;
    }
    data->capacity = INITIAL_BINARYDATAARRAY_SIZE;

    return true;
}

/**
 * Adds a bit to a struct binarydataarray.
 *
 * @param data The struct binarydataarray to which the bit is added.
 * @param bit The bit to be added.
 * @param array_index Pointer to the current index in the array.
 * @param element_bitshift Pointer to the current bit shift within an element.
 * @return True if the addition process is successful, false otherwise.
 */
bool binarydataarray_add_bit(struct binarydataarray *data, bool bit, size_t *array_index, size_t *element_bitshift) {
    if (!data || !array_index || !element_bitshift) {
        LOG_MESSAGE(ERROR, "Argument is NULL!");
        return false;
    }
    if ((*element_bitshift) >= (size_t) (sizeof(binarydata_type) * BITS_IN_BYTE)) {
        LOG_MESSAGE(ERROR, "Element bit shift is out of bounds");
        return false;
    }

    bool is_success;

    // Ensure dynamic array is initialized
    if (data->capacity == 0) {
        is_success = binarydataarray_initialize(data);
        if (!is_success) {
            LOG_MESSAGE(ERROR, "Failed to initialize dynamic array for binarydata.");
            return false;
        }
    }

    // Ensure valid array index
    if (*array_index >= data->length) {
        // Need to add a new element to the dynamic array
        binarydata_type new_element = 0;
        is_success = binarydataarray_add_element(data, new_element);
        if (!is_success) {
            LOG_MESSAGE(ERROR, "Failed to add new element to binarydata array.");
            return false;
        }
    }

    size_t last_bit_shift = (size_t) ((sizeof(binarydata_type) * BITS_IN_BYTE) - 1);
    // Set the bit in the current element
    if (bit) {
        data->array[*array_index] |= (1 << (last_bit_shift - (*element_bitshift)));
    } else {
        data->array[*array_index] &= ~(1 << (last_bit_shift - (*element_bitshift)));
    }

    // Update array index and bit shift for the next iteration
    if (*element_bitshift == last_bit_shift) {
        // Move to the next element in the array
        (*array_index)++;
        *element_bitshift = 0;
    } else {
        // Move to the next bit in the current element
        (*element_bitshift)++;
    }

    return true;

}

/**
 * Retrieves the next bit from a struct binarydataarray.
 *
 * @param binarydata_array The struct binarydataarray from which the bit is retrieved.
 * @param array_index Pointer to the current index in the array.
 * @param element_bit_shift Pointer to the current bit shift within an element.
 * @param ptr_return_nextbit Pointer to store the retrieved bit.
 * @return True if the retrieval process is successful, false otherwise.
 */
bool binarydataarray_get_next_bit(struct binarydataarray binarydata_array, size_t *array_index, size_t *element_bit_shift,
                                  bool *ptr_return_nextbit) {
    if (*array_index >= binarydata_array.length) {
        LOG_MESSAGE(ERROR, "Array index out of bounds.");
        return false;
    }
    if (*element_bit_shift >= (size_t) (sizeof(binarydata_type) * BITS_IN_BYTE)) {
        LOG_MESSAGE(ERROR, "Element bit shift out of bounds.");
        return false;
    }
    if (binarydata_array.array == NULL || binarydata_array.capacity == 0) {
        LOG_MESSAGE(ERROR, "PayloadArray is empty.");
        return false;
    }
    if (!ptr_return_nextbit) {
        LOG_MESSAGE(ERROR, "Argument is NULL!");
        return false;
    }


    size_t last_bit_shift = (size_t) ((sizeof(binarydata_type) * BITS_IN_BYTE) - 1);
    *ptr_return_nextbit = ((binarydata_array.array)[(*array_index)] >> (last_bit_shift - (*element_bit_shift))) & 1;


    if (*element_bit_shift == last_bit_shift) {
        *element_bit_shift = 0;
        (*array_index)++;
    } else {
        (*element_bit_shift)++;
    }


    return true;
}
//endregion