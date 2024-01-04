//
// Author: Lenovo
// Date: 24.11.2023
// Description: 
//

#include "logger.h"
#include "utils.h"
#include <time.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

// Global variables to control logging behavior for each level
int logInfoToConsole = 0;   // Set to 1 to enable INFO logs to console
int logWarningToConsole = 0; // Set to 1 to enable WARNING logs to console
int logErrorToConsole = 1;   // Set to 1 to enable ERROR logs to console

int logInfoToFile = 1;      // Set to 1 to enable INFO logs to file
int logWarningToFile = 1;   // Set to 1 to enable WARNING logs to file
int logErrorToFile = 1;     // Set to 1 to enable ERROR logs to file

int counter = 0;
char* log_file_name = NULL;

// Array of structs for each log level
const LogLevelInfo LogLevelInfoArray[] = {
        {INFO, "INFO", false},
        {WARNING, "WARNING", false},
        {ERROR, "ERROR", true}
};

void init_logger() {
    time_t t;
    struct tm *tm_info;
    char date_string[20]; // Adjust the size based on your needs

    time(&t);
    tm_info = localtime(&t);

    strftime(date_string, sizeof(date_string), "%Y-%m-%d", tm_info);

    // Open the log file in append mode
    char* postfix = "_logfile.txt";

    log_file_name = TRACKED_MALLOC(CALC_STR_MEM_SIZE(strlen(date_string) + strlen(postfix) + 1));
    strcpy(log_file_name, date_string);
    strcat(log_file_name, postfix);

    char * prefix = "logs/";
    char* log_file_path = TRACKED_MALLOC(CALC_STR_MEM_SIZE(strlen(prefix) + strlen(log_file_name)));
    strcpy(log_file_path, prefix);
    strcat(log_file_path, log_file_name);
    FILE *log_file = fopen(log_file_name, "w");

    if (log_file == NULL) {
        printf("Error opening log file\n");
        return;
    }

    // Close the log file
    fclose(log_file);
}


void log_message(LogLevel level, const char* file, int line, const char *format, ...) {
    /*time_t t;
    struct tm *tm_info;
    char date_string[20]; // Adjust the size based on your needs

    time(&t);
    tm_info = localtime(&t);

    strftime(date_string, sizeof(date_string), "%Y-%m-%d", tm_info);

    // Open the log file in append mode
    char* postfix = "_logfile.txt";

    char * log_file_name = TRACKED_MALLOC(CALC_STR_MEM_SIZE(strlen(date_string) + strlen(postfix) + 1));
    strcpy(log_file_name, date_string);
    strcat(log_file_name, postfix);

    char * prefix = "logs/";
    char* log_file_path = TRACKED_MALLOC(CALC_STR_MEM_SIZE(strlen(prefix) + strlen(log_file_name)));
    strcpy(log_file_path, prefix);
    strcat(log_file_path, log_file_name);*/
    FILE *log_file = fopen(log_file_name, "a");

    if (log_file == NULL) {
        printf("Error opening log file\n");
        return;
    }

    // Log text for the given log level
    for (size_t i = 0; i < sizeof(LogLevelInfoArray) / sizeof(LogLevelInfoArray[0]); ++i) {
        if (level == LogLevelInfoArray[i].level && LogLevelInfoArray[i].is_enabled) {
            printf("[%s] ", LogLevelInfoArray[i].string);
            fprintf(log_file, "[%s] ", LogLevelInfoArray[i].string);

            // Handle variadic arguments
            va_list args;
            va_start(args, format);

            // Log to console
            vprintf(format, args);
            printf("\n File: \"%s:%d\"\n Line: %d\n", file, line, line);

            // Log to file
            vfprintf(log_file, format, args);
            fprintf(log_file, "\n File: %s\n Line: %d\n", file, line);

            va_end(args);
            break;
        }
    }



    // Close the log file
    fclose(log_file);
}

