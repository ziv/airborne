/**
 * @file Logger.h
 * @brief Custom raylib log callback that adds ANSI colours to console output.
 */
#pragma once
#include "raylib.h"

/**
 * @brief Coloured console log callback for raylib's TraceLog system.
 *
 * Replaces the default raylib logger. Prefixes each message with a coloured
 * tag: blue [DEBUG], green [INFO], red [ERROR], yellow [WARN].
 */
inline void CustomLogCallback(const int logLevel, const char *text, const va_list args) {
    // get rid of raylib internal annoying messages
    if (logLevel == LOG_INFO) {
        if (strstr(text, "uploaded successfully") != nullptr ||
            strstr(text, "loaded successfully") != nullptr ||
            strstr(text, "Unloaded") != nullptr) {
            return;
        }
    }
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
