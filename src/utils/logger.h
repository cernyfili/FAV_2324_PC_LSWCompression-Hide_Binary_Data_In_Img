//
// Author: Lenovo
// Date: 24.11.2023
// Description: 
//

#ifndef FAV_PC_SP_23_24_BINARYDATAINIMG_LOGGER_H
#define FAV_PC_SP_23_24_BINARYDATAINIMG_LOGGER_H

#include <stdbool.h>

#define LOG_MESSAGE(level, format, ...) log_message(level, __FILE__, __LINE__, format __VA_OPT__(,) __VA_ARGS__)

typedef enum {
    INFO,
    WARNING,
    ERROR
} LogLevel;

/**
 * Struct to hold both enum value and string representation
 */
typedef struct {
    LogLevel level;
    const char *string;
    bool is_enabled;
} LogLevelInfo;


void log_message(LogLevel level, const char* file, int line, const char *format, ...);

void init_logger();

#endif //FAV_PC_SP_23_24_BINARYDATAINIMG_LOGGER_H
