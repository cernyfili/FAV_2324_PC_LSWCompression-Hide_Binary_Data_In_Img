//
// Author: Lenovo
// Date: 24.11.2023
// Description: 
//

#include "logger.h"

#include <stdio.h>
#include <stdarg.h>

// Array of structs for each log level
const LogLevelInfo LogLevelInfoArray[] = {
        {INFO, "INFO"},
        {WARNING, "WARNING"},
        {ERROR, "ERROR"}
};

void log_message(LogLevel level, const char* file, int line, const char *format, ...) {
    // Open the log file in append mode
    FILE *logFile = fopen("logfile.txt", "a");

    if (logFile == NULL) {
        fprintf(stderr, "Error opening log file\n");
        return;
    }

    // Log to file
    for (size_t i = 0; i < sizeof(LogLevelInfoArray) / sizeof(LogLevelInfoArray[0]); ++i) {
        if (level == LogLevelInfoArray[i].level) {
            printf("[%s] ", LogLevelInfoArray[i].string);
            fprintf(logFile, "[%s] ", LogLevelInfoArray[i].string);
            break;
        }
    }

    // Handle variadic arguments
    va_list args;
    va_start(args, format);

    // Log to console
    vprintf(format, args);
    printf("\n File: %s\n Line: %d\n", file, line);

    // Log to file
    vfprintf(logFile, format, args);
    fprintf(logFile, "\n File: %s\n Line: %d\n", file, line);

    va_end(args);

    // Close the log file
    fclose(logFile);
}

