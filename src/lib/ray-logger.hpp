#pragma once
#include "ray.hpp"

/**
 * @brief Coloured console log callback for raylib's TraceLog system.
 *
 * Replaces the default raylib logger. Prefixes each message with a coloured
 * tag: blue [DEBUG], green [INFO], red [ERROR], yellow [WARN].
 */
inline void custom_log_callback(const int log_level, const char *text, const va_list args) {
    // get rid of raylib internal annoying messages
    if (log_level == LOG_INFO) {
        if (strstr(text, "uploaded successfully") != nullptr ||
            strstr(text, "loaded successfully") != nullptr ||
            strstr(text, "Unloaded") != nullptr) {
            return;
            }
    }
    switch (log_level) {
        case LOG_DEBUG:
            printf("\033[34m[D]\033[0m ");
            break;
        case LOG_INFO:
            printf("\033[32m[I]\033[0m ");
            break;
        case LOG_ERROR:
            printf("\033[31m[E]\033[0m ");
            break;
        case LOG_WARNING:
            printf("\033[33m[W]\033[0m ");
            break;
        default:
            printf("[U] ");
            break;
    }
    vprintf(text, args);
    printf("\n");
}
