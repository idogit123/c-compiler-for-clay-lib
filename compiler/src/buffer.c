#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "buffer.h"

void buf_init(Buffer* b) {
    b->capacity = 1024;
    b->len = 0;
    b->data = (char*)malloc(b->capacity);
    if (!b->data) { fprintf(stderr,"Out of memory\n"); exit(1); }
    b->data[0] = '\0';
}

void buf_append(Buffer* b, const char* str) {
    size_t slen = strlen(str);
    if (b->len + slen + 1 > b->capacity) {
        b->capacity *= 2;
        b->data = (char*)realloc(b->data, b->capacity);
        if (!b->data) { fprintf(stderr,"Out of memory\n"); exit(1); }
    }
    memcpy(b->data + b->len, str, slen);
    b->len += slen;
    b->data[b->len] = '\0';
}

void buf_append_fmt(Buffer* b, const char* fmt, ...) {
    char tmp[1024];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(tmp, sizeof(tmp), fmt, args);
    va_end(args);
    if (n > 0) buf_append(b, tmp);
}
