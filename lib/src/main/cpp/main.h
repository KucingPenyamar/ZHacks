#ifndef DIRECTIVE_STATEMENT
#define DIRECTIVE_STATEMENT
#include <stddef.h>

const char *targetProcessName = "com.unity.test";
void inject(const char *gameDataDir, void *data, size_t length);

#endif