/*
//
// Author: Lenovo
// Date: 28.12.2023
// Description: 
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>  // for assert function
#include "../payload.h"
#include "../compression.h"
#include "../utils/utils.h"
#include "../utils/dictionary.h"
#include "../utils/data_structures.h"
#include "../image.h"

// Test case for compress_payload
void test_compression_payload() {
    // Create some sample data
    char * test_str = "Ahoj tohle je test jak mi funguje komprese a dekomprese dat.";
    printf("test_str: %s\n", test_str);
    PayloadArray data;
    data.array = TRACKED_MALLOC(CALC_STR_REAL_LEN(test_str) * sizeof(PayloadType));
    data.capacity = (size_t) CALC_STR_REAL_LEN(test_str);
    data.length = data.capacity;
    memcpy(data.array, test_str, data.capacity);

    // Initialize and populate the data array

    // Call the compress_payload function
    PayloadArray compressed_data;
    bool success = compress_payload(data, &compressed_data);

    // Assert that the compression was successful
    assert(success);

    // Assert that the compressed_data array is not empty
    assert(compressed_data.length > 0);

    //TEST DECOMPRESS
    // Initialize and populate the compressed_data array

    // Call the decompress_payload function
    char* uncompressed_data;
    success = decompress_payload(compressed_data, &uncompressed_data);

    printf("uncompressed_data: %s\n", uncompressed_data);

    // Assert that the decompression was successful
    assert(success);

    // Assert that the uncompressed_data array is not empty

    // Clean up the memory

    // Clean up the memory
    free(compressed_data.array);
}

void test_hide_data_bmp(){
    PayloadArray data;
    char * test_str = "test";
    u_int32_t size = 9;
    */
/*char * test_str = "??????????";*//*

    data.array = TRACKED_MALLOC((CALC_STR_MEM_SIZE(strlen(test_str)) + 4) * sizeof(PayloadType));
    data.capacity = 9;
    data.length = data.capacity;
    memcpy(data.array, &size, 4);
    memcpy(data.array + 4, test_str, data.capacity);

    //test next bit function
    */
/*size_t array_index = 0;
    size_t element_bit_shift = 0;
    for (int i = 0; i < (double)data.length * BITS_IN_BYTE; ++i) {
        bool bit;
        bool is_succes = payloadarray_get_next_bit(data, &array_index, &element_bit_shift, &bit);
        if (!is_succes){
            printf("error");
            return;
        }
        printf("%d", bit);
    }*//*




    char * input_filepath = "/mnt/g/Můj disk/0_MAIN/škola/FAV/3.rocnik_23_24/_FAV_ZS_23_24/PC/semestralka/FAV_PC_SP_23_24_BinaryDataInImg/data/test_data/test_data_01/input_img_100x100.bmp";
    char * output_filepath = "/mnt/g/Můj disk/0_MAIN/škola/FAV/3.rocnik_23_24/_FAV_ZS_23_24/PC/semestralka/FAV_PC_SP_23_24_BinaryDataInImg/data/test_data/test_data_01/test.bmp";
    int result = hide_data_lsb(input_filepath, data,output_filepath);

    print_rgb_values(output_filepath);

    PayloadArray hidden_data;
    payloadarray_initialize(&hidden_data);


    extract_data_lsb(output_filepath, &hidden_data);

    printf("end");
}

void test_clenup_lib(){
    struct cleanupcommand * clenup_list = NULL;

    char * test_str_1 = TRACKED_MALLOC(10);
    CLEANUP_ADD_COMMAND(&clenup_list, test_str_1);
    if (!test_str_1){
        cleanup_run_commands(&clenup_list);
        printf("test_str_1 is NULL");
        return;
    }

    char * test_str_2 = TRACKED_MALLOC(10);
    CLEANUP_ADD_COMMAND(&clenup_list, test_str_2);
    if (!test_str_2){
        cleanup_run_commands(&clenup_list);
        printf("test_str_2 is NULL");
        return;
    }

    cleanup_run_commands(&clenup_list);

}

// Run the tests
void run_tests() {
    test_clenup_lib();
}
*/
