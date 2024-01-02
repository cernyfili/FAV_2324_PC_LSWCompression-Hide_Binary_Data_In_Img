//
// Author: Lenovo
// Date: 07.11.2023
// Description: 
//

#ifndef FAV_PC_SP_23_24_BINARYDATAINIMG_COMPRESSION_H
#define FAV_PC_SP_23_24_BINARYDATAINIMG_COMPRESSION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils/data_structures.h"




bool compress_payload(PayloadArray data, PayloadArray *ptr_compressed_data);

bool decompress_payload(PayloadArray compressed_data, PayloadArray *ptr_uncompressed_data);



#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_COMPRESSION_H
