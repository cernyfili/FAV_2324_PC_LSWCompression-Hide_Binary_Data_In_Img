//
// Author: Filip Cerny
// Date: 07.12.2023
// Description: This file contains functions for manipulating binary data.
//

#include "data_structures.h"
#include "utils.h"

/**
 * Represents the initial size of the payload array.
 */
#define INITIAL_PAYLOADARRAY_SIZE 1000
/**
 * Represents the increment of the payload array size.
 */
#define DIC_ARR_INC 1000


/**
 * Represents a dynamic array.
 */
typedef struct {
    void *array;     /**< Pointer to the array data. */
    size_t length;   /**< Current number of elements in the array. */
    size_t capacity; /**< Maximum number of elements the array can hold. */
} DynamicArray;

//region FUNCTION DECLARATIONS

/**
 * Adds an element to a dynamic array, ensuring capacity and handling memory allocation.
 *
 * @param dynamic_array Pointer to the dynamic array.
 * @param element Pointer to the element to be added.
 * @param element_size Size of the element in bytes.
 * @return true if the addition is successful, false otherwise.
 */
static bool dynamicarray_add_element(DynamicArray *dynamic_array, void* element, size_t element_size);


/**
 * Initializes a payload array with default values.
 *
 * @param data Pointer to the payload array to be initialized.
 * @return true if the initialization is successful, false otherwise.
 */
static bool payloadarray_initialize(PayloadArray *data);


/**
 * Adds an element to a payload array, ensuring capacity and handling memory allocation.
 *
 * @param payload_array Pointer to the payload array.
 * @param element Payload element to be added.
 * @return true if the addition is successful, false otherwise.
 */
static bool payloadarray_add_element(PayloadArray *payload_array, PayloadType element);
//endregion

//region FUNCTION DEFINITIONS
//region PRIVATE FUNCTIONS
/**
 * Adds an element to a generic dynamic array.
 *
 * @param dynamic_array The DynamicArray to which the element is added.
 * @param element Pointer to the element to be added.
 * @param element_size Size of the element.
 * @return True if the addition process is successful, false otherwise.
 */
static bool dynamicarray_add_element(DynamicArray *dynamic_array, void* element, size_t element_size) {
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
 * Initializes a PayloadArray.
 *
 * @param data The PayloadArray to be initialized.
 * @return True if the initialization process is successful, false otherwise.
 */
static bool payloadarray_initialize(PayloadArray *data) {
    if (!data) {
        LOG_MESSAGE(ERROR, "PayloadArray is NULL!");
        return false;
    }

    data->length = 0;
    data->array = TRACKED_MALLOC(sizeof(PayloadType) * INITIAL_PAYLOADARRAY_SIZE);
    if (!data->array) {
        LOG_MESSAGE(ERROR, "Failed to allocate memory for payload array.");
        return false;
    }
    data->capacity = INITIAL_PAYLOADARRAY_SIZE;

    return true;
}
//endregion

/**
 * Adds a bit to a PayloadArray.
 *
 * @param data The PayloadArray to which the bit is added.
 * @param bit The bit to be added.
 * @param array_index Pointer to the current index in the array.
 * @param element_bitshift Pointer to the current bit shift within an element.
 * @return True if the addition process is successful, false otherwise.
 */
bool payload_array_add_bit(PayloadArray *data, bool bit, size_t *array_index, size_t *element_bitshift) {
    if(!data || !array_index || !element_bitshift) {
        LOG_MESSAGE(ERROR, "Argument is NULL!");
        return false;
    }
    if((*array_index) >= data->capacity) {
        LOG_MESSAGE(ERROR, "Array index is out of bounds");
        return false;
    }
    if((*element_bitshift) >= (size_t)(sizeof(PayloadType) * BITS_IN_BYTE)) {
        LOG_MESSAGE(ERROR, "Element bit shift is out of bounds");
        return false;
    }

    bool is_succes;

    // Ensure dynamic array is initialized
    if (data->capacity == 0) {
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

    size_t last_bit_shift = (size_t)((sizeof(PayloadType) * BITS_IN_BYTE) - 1);
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
 * Adds an element to a PayloadArray.
 *
 * @param payload_array The PayloadArray to which the element is added.
 * @param element The element to be added.
 * @return True if the addition process is successful, false otherwise.
 */
static bool payloadarray_add_element(PayloadArray *payload_array, PayloadType element) {
    if(!payload_array || !payload_array->array) {
        LOG_MESSAGE(ERROR, "Argument is NULL!");
        return false;
    }
    return dynamicarray_add_element((DynamicArray *) payload_array, &element, sizeof(PayloadType));
}

/**
 * Retrieves the next bit from a PayloadArray.
 *
 * @param payload_array The PayloadArray from which the bit is retrieved.
 * @param array_index Pointer to the current index in the array.
 * @param element_bit_shift Pointer to the current bit shift within an element.
 * @param ptr_return_nextbit Pointer to store the retrieved bit.
 * @return True if the retrieval process is successful, false otherwise.
 */
bool payloadarray_get_next_bit(PayloadArray payload_array, size_t *array_index, size_t *element_bit_shift,
                               bool *ptr_return_nextbit) {
    if (*array_index >= payload_array.length) {
        LOG_MESSAGE(ERROR, "Array index out of bounds.");
        return false;
    }
    if (*element_bit_shift >= (size_t)(sizeof(PayloadType) * BITS_IN_BYTE)) {
        LOG_MESSAGE(ERROR, "Element bit shift out of bounds.");
        return false;
    }
    if(payload_array.array == NULL || payload_array.capacity == 0) {
        LOG_MESSAGE(ERROR, "PayloadArray is empty.");
        return false;
    }
    if(!ptr_return_nextbit) {
        LOG_MESSAGE(ERROR, "Argument is NULL!");
        return false;
    }


    size_t last_bit_shift = (size_t)((sizeof(PayloadType) * BITS_IN_BYTE) - 1);
    *ptr_return_nextbit = ((payload_array.array)[(*array_index)] >> (last_bit_shift - (*element_bit_shift))) & 1;


    if (*element_bit_shift == last_bit_shift) {
        *element_bit_shift = 0;
        (*array_index)++;
    } else {
        (*element_bit_shift)++;
    }


    return true;
}
//endregion