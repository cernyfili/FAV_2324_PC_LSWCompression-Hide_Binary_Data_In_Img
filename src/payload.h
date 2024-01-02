//
// Author: Lenovo
// Date: 02.12.2023
// Description: 
//

#ifndef FAV_PC_SP_23_24_BINARYDATAINIMG_PAYLOAD_H
#define FAV_PC_SP_23_24_BINARYDATAINIMG_PAYLOAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "utils/data_structures.h"

bool prepare_payload_data(const char *filename, PayloadArray *output);

int extract_payload_from_data(PayloadArray hidden_data, PayloadArray *output);

#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_PAYLOAD_H
