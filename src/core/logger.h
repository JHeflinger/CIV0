#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

#define LOG_RESET "\033[0m"
#define LOG_RED "\033[31m"
#define LOG_BLUE "\033[34m"
#define LOG_GREEN "\033[32m"
#define LOG_YELLOW "\033[33m"
#define LOG_PURPLE "\033[35m"
#define LOG_CYAN "\033[36m"

#define LOG_INFO(...)  {printf("%s[INFO]%s  ", LOG_GREEN, LOG_RESET);  printf(__VA_ARGS__); printf("\n");}
#define LOG_FATAL(...) {printf("%s[FATAL]%s ", LOG_RED, LOG_RESET);    printf(__VA_ARGS__); printf("\n"); exit(1);}
#define LOG_WARN(...)  {printf("%s[WARN]%s  ", LOG_YELLOW, LOG_RESET); printf(__VA_ARGS__); printf("\n");}
#define LOG_DEBUG(...) {printf("%s[DEBUG]%s ", LOG_BLUE, LOG_RESET);   printf(__VA_ARGS__); printf("\n");}
#define LOG_CUSTOM(precursor, ...) {printf("%s[%s]%s  ", LOG_CYAN, precursor, LOG_RESET);   printf(__VA_ARGS__); printf("\n");}
#define LOG_SCAN(...)  {printf("%s[INPUT]%s ", LOG_PURPLE, LOG_RESET); scanf(__VA_ARGS__);}
#define LOG_ASSERT(check, ...) {if (!check) LOG_FATAL(__VA_ARGS__);}

#endif
