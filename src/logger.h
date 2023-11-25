//
// Author: Lenovo
// Date: 24.11.2023
// Description: 
//

#ifndef FAV_PC_SP_23_24_BINARYDATAINIMG_LOGGER_H
#define FAV_PC_SP_23_24_BINARYDATAINIMG_LOGGER_H

typedef enum {
    INFO,
    WARNING,
    ERROR
} LogLevel;

// Struct to hold both enum value and string representation
typedef struct {
    LogLevel level;
    const char *string;
} LogLevelInfo;


void log_message(LogLevel level, char* file, int line, const char *format, ...);

#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_LOGGER_H
