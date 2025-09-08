#include <stdio.h>
#include "error.h"

void print_error(Scanner* s, const char* msg) {
    size_t len = (size_t)(s->current - s->start);
    fprintf(stderr, "[%d:%d] Error: %s at '", s->line, s->col - (int)len, msg);
    fwrite(s->start, 1, len, stderr);
    fprintf(stderr, "'\n");
}
