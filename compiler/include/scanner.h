#ifndef SCANNER_H
#define SCANNER_H

#include <stddef.h>

typedef struct {
    const char* ptr;
    size_t len;
} Slice;

typedef enum { T_PRINT, T_STRING, T_INT, T_FLOAT, T_NEWLINE, T_EOF, T_ERROR } TokenType;

typedef struct {
    const char* start;
    const char* current;
    int line, col;
} Scanner;

typedef struct {
    TokenType type;
    Slice lexeme;
    union {
        Slice str;    // only for T_STRING and T_NUMBER
    } as;
    int line, col;
} Token;

void scanner_init(Scanner* s, const char* src);
Token scan_token(Scanner* s);

#endif // SCANNER_H
