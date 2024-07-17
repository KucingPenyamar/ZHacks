#ifndef DIRECTIVE_STATEMENT
#define DIRECTIVE_STATEMENT
#include <stddef.h>

const char *targetProcessName = "com.unity.test";
void inject(const char *gameDataDir, void *data, size_t length);

#define hook_input(ret, func, ...) \
    ret (*orig##func)(__VA_ARGS__); \
    ret my##func(__VA_ARGS__)

#endif