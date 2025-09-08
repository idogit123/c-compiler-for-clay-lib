#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

/* ------------ code buffer ------------ */
typedef struct {
    char* data;
    size_t len;
    size_t capacity;
} Buffer;

static void buf_init(Buffer* b) {
    b->capacity = 1024;
    b->len = 0;
    b->data = malloc(b->capacity);
    if (!b->data) { fprintf(stderr,"Out of memory\n"); exit(1); }
    b->data[0] = '\0';
}

static void buf_append(Buffer* b, const char* str) {
    size_t slen = strlen(str);
    if (b->len + slen + 1 > b->capacity) {
        b->capacity *= 2;
        b->data = realloc(b->data, b->capacity);
        if (!b->data) { fprintf(stderr,"Out of memory\n"); exit(1); }
    }
    memcpy(b->data + b->len, str, slen);
    b->len += slen;
    b->data[b->len] = '\0';
}

static void buf_append_fmt(Buffer* b, const char* fmt, ...) {
    char tmp[1024];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(tmp, sizeof(tmp), fmt, args);
    va_end(args);
    if (n > 0) buf_append(b, tmp);
}


/* ---------- tokenizer (partial: Scanner forward definition) ---------- */
typedef struct {
    const char* start;
    const char* current;
    int line, col;
} Scanner;



/* ---------- error handling ---------- */
static void print_error(Scanner* s, const char* msg) {
    size_t len = (size_t)(s->current - s->start);
    fprintf(stderr, "[%d:%d] Error: %s at '", s->line, s->col - (int)len, msg);
    fwrite(s->start, 1, len, stderr);
    fprintf(stderr, "'\n");
}

/* ---------- file reading ---------- */
static char* read_entire_file(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    if (n < 0) { fclose(f); return NULL; }
    fseek(f, 0, SEEK_SET);
    char* buf = (char*)malloc((size_t)n + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t read = fread(buf, 1, (size_t)n, f);
    fclose(f);
    buf[read] = '\0';
    return buf;
}

/* ---------- tokenizer ---------- */
typedef enum { T_PRINT, T_STRING, T_NEWLINE, T_EOF, T_ERROR, T_THEN } TokenType;

typedef struct { const char* ptr; size_t len; } Slice;

typedef struct {
    TokenType type;
    Slice lexeme;
    union {
        Slice str;    // only for T_STRING
    } as;
    int line, col;
} Token;

static int is_at_end(Scanner* s) { return *s->current == '\0'; }
static char advance(Scanner* s) { char c = *s->current; if (c) { s->current++; s->col++; } return c; }
static char peek(Scanner* s) { return *s->current; }
static void skip_spaces(Scanner* s) { while (isspace(peek(s)) && peek(s) != '\n') advance(s); }

static Token make_token(Scanner* s, TokenType type) {
    Token t;
    t.type = type;
    t.lexeme.ptr = s->start;
    t.lexeme.len = (size_t)(s->current - s->start);
    t.line = s->line;
    t.col = s->col - (int)t.lexeme.len;
    return t;
}

static Token error_token(Scanner* s, const char* msg) {
    print_error(s, msg);
    return make_token(s, T_ERROR);
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

static Token scan_token(Scanner* s) {
    skip_spaces(s);
    s->start = s->current;

    if (is_at_end(s)) return make_token(s, T_EOF);

    char c = advance(s);
    if (c == '\n') { s->line++; s->col = 1; return make_token(s, T_NEWLINE); }
    if (c == '"') return string_token(s);
    if (is_ident_start(c)) return identifier_or_keyword(s);

    return error_token(s, "Unexpected character");
}

/* ---------- code generation ---------- */
static void compile_to_c(const char* src, const char* out_path) {
    Buffer code;
    buf_init(&code);

    buf_append(&code, "#include <stdio.h>\n\nint main() {\n");

    Scanner sc = { .start = src, .current = src, .line = 1, .col = 1 };
    for (;;) {
        Token t = scan_token(&sc);
        if (t.type == T_EOF) break;
        if (t.type == T_ERROR) { free(code.data); exit(1); } // stop compilation on error
        if (t.type == T_NEWLINE) continue;
        if (t.type != T_PRINT) { 
            print_error(&sc, "Expected 'print' statement"); 
            free(code.data); 
            exit(1); 
        }

        Token next = scan_token(&sc);
        if (next.type == T_ERROR) { free(code.data); exit(1); }
        if (next.type != T_STRING) { print_error(&sc, "Expected string after print\n"); free(code.data); exit(1); }
        buf_append_fmt(&code, "    printf(\"%.*s\\n\");\n", (int)next.as.str.len, next.as.str.ptr);
        
        Token end = scan_token(&sc);
        if (end.type == T_ERROR) { free(code.data); exit(1); }
        if (end.type != T_NEWLINE && end.type != T_EOF && end.type != T_THEN) { 
            print_error(&sc, "Expected newline after print statement\n"); 
            free(code.data); 
            exit(1);
        }
    }

    buf_append(&code, "    return 0;\n}\n");

    FILE* out = fopen(out_path, "w");
    if (!out) { 
        fprintf(stderr,"Cannot write to %s\n", out_path); 
        free(code.data); 
        exit(1); 
    }
    fwrite(code.data, 1, code.len, out);
    fclose(out);
    free(code.data);
}

/* ---------- main ---------- */
int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: compiler.exe input.clay output.c\n");
        return 1;
    }

    char* src = read_entire_file(argv[1]);
    if (!src) { fprintf(stderr, "Could not read file: %s\n", argv[1]); return 1; }

    compile_to_c(src, argv[2]);
    free(src);

    printf("Compilation successful: %s -> %s\n", argv[1], argv[2]);
    return 0;
}
