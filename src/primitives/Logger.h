#pragma once
#include "raylib.h"

/**
 * Just add some colors to the console...
 */
inline void CustomLogCallback(const int logLevel, const char *text, const va_list args) {
    switch (logLevel) {
        case LOG_DEBUG:
            printf("\033[34m[DEBUG]\033[0m ");
            break;
        case LOG_INFO:
            printf("\033[32m[INFO]\033[0m  ");
            break;
        case LOG_ERROR:
            printf("\033[31m[ERROR]\033[0m  ");
            break;
        case LOG_WARNING:
            printf("\033[33m[WARN]\033[0m   ");
            break;
        default:
            printf("[UNKNOWN] ");
            break;
    }
    vprintf(text, args);
    printf("\n");
}
