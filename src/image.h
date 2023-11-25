//
// Author: Lenovo
// Date: 07.11.2023
// Description: 
//

#include <stdbool.h>

#ifndef FAV_PC_SP_23_24_BINARYDATAINIMG_IMAGE_H
#define FAV_PC_SP_23_24_BINARYDATAINIMG_IMAGE_H

/**
 * Function to read binary data from a file and return an array of boolean values
 * @param filename  is the path to the file
 * @return  a pointer to the boolean array
 *  NULL if there was an error
 */
bool *read_payload_file(char filename);



#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_IMAGE_H
