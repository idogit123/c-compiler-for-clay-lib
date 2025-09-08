#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>

typedef struct {
    char* data;
    size_t len;
    size_t capacity;
} Buffer;

void buf_init(Buffer* b);
void buf_append(Buffer* b, const char* str);
void buf_append_fmt(Buffer* b, const char* fmt, ...);

#endif // BUFFER_H
