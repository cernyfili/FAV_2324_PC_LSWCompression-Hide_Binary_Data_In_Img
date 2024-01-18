//
// Author: Filip Cerny
// Date: 24.11.2023
// Description: This file contains logger functions.
//

#include "logger.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

/**
 * Represents whether the console logging is enabled.
 */
#define IS_CONSOLE_LOG_ENABLED true

/**
 * Array of log level info structs.
 */
const struct loglevelinfo loglevelinfo_array[] = {
        {INFO,    "INFO",    true},
        {WARNING, "WARNING", true},
        {ERROR,   "ERROR",   true}
};

/**
 * Logs a formatted message with the specified log level, including file name and line number information.
 * The format string and additional arguments follow the printf-style formatting.
 *
 * @param level Log level (INFO, WARNING, or ERROR).
 * @param file File name where the log message is generated.
 * @param line Line number where the log message is generated.
 * @param format Format string for the log message.
 * @param ... Additional arguments for the format string.
 */
void log_message(loglevel_enum level, const char *file, int line, const char *format, ...) {
    if (!format) {
        printf("No text to log.");
        return;
    }

    bool is_console_log_enabled = IS_CONSOLE_LOG_ENABLED;

    // Log text for the given log level
    for (size_t i = 0; i < sizeof(loglevelinfo_array) / sizeof(loglevelinfo_array[0]); i++) {
        if (level == loglevelinfo_array[i].level && loglevelinfo_array[i].is_enabled) {
            // Log to console
            if (is_console_log_enabled) {
                printf("[%s] ", loglevelinfo_array[i].string);
            }

            // Handle variadic arguments
            va_list args;
            va_start(args, format);

            // Log to console

            if (is_console_log_enabled) {
                vprintf(format, args);
                printf("\n File: \"%s:%d\"\n Line: %d\n", file, line, line);
            }

            va_end(args);
            break;
        }
    }
}

