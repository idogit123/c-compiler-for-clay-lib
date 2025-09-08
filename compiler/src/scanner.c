#include <string.h>
#include <ctype.h>
#include "scanner.h"
#include "error.h"

static int is_at_end(Scanner* s) { return *s->current == '\0'; }
static char advance(Scanner* s) { char c = *s->current; if (c) { s->current++; s->col++; } return c; }
static char peek(Scanner* s) { return *s->current; }
static void skip_spaces(Scanner* s) { while (isspace((unsigned char)peek(s)) && peek(s) != '\n') advance(s); }
static int is_digit(char c) { return c >= '0' && c <= '9'; }

static Token make_token(Scanner* s, TokenType type) {
    Token t;
    t.type = type;
    t.lexeme.ptr = s->start;
    t.lexeme.len = (size_t)(s->current - s->start);
    t.line = s->line;
    t.col = s->col - (int)t.lexeme.len;
    return t;
}

void scanner_init(Scanner* s, const char* src) {
    s->start = src;
    s->current = src;
    s->line = 1;
    s->col = 1;
}

static Token error_token(Scanner* s, const char* msg) {
    print_error(s, msg);
    return make_token(s, T_ERROR);
}

static Token number_token(Scanner* s) {
    const char* start = s->current - 1; // first digit is already consumed
    while (is_digit(peek(s))) advance(s);

    Token t = make_token(s, T_NUMBER);
    t.as.str.ptr = start;
    t.as.str.len = (size_t)(s->current - start);
    return t;
}

static Token string_token(Scanner* s) {
    const char* content_start = s->current;
    int start_col = s->col;

    while (!is_at_end(s) && peek(s) != '"') {
        char c = advance(s);
        if (c == '\n') { s->line++; s->col = 1; }
    }

    if (is_at_end(s)) return error_token(s, "Unterminated string");

    advance(s); // consume closing quote

    Token t = make_token(s, T_STRING);
    t.as.str.ptr = content_start;
    t.as.str.len = (size_t)((s->current - 1) - content_start);
    t.col = start_col - 1;
    return t;
}

static int is_ident_start(char c) { return isalpha((unsigned char)c) || c == '_'; }
static int is_ident(char c) { return isalnum((unsigned char)c) || c == '_'; }

static Token identifier_or_keyword(Scanner* s) {
    while (is_ident(peek(s))) advance(s);
    size_t len = (size_t)(s->current - s->start);

    if (len == 5 && memcmp(s->start, "print", 5) == 0) {
        return make_token(s, T_PRINT);
    }
    else if (len == 4 && memcmp(s->start, "then", 4) == 0) {
        return make_token(s, T_THEN);
    }
    return error_token(s, "Unknown identifier");
}



Token scan_token(Scanner* s) {
    skip_spaces(s);
    s->start = s->current;

    if (is_at_end(s)) return make_token(s, T_EOF);

    char c = advance(s);
    if (c == '\n') { s->line++; s->col = 1; return make_token(s, T_NEWLINE); }
    if (c == '"') return string_token(s);
    if (is_digit(c)) return number_token(s);
    if (is_ident_start(c)) return identifier_or_keyword(s);

    return error_token(s, "Unexpected character");
}