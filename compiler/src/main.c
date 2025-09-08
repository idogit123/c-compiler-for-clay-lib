#include <stdio.h>
#include <stdlib.h>
#include "buffer.h"
#include "scanner.h"
#include "error.h"

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

static void compile_to_c(const char* src, const char* out_path) {
    Buffer code;
    buf_init(&code);

    buf_append(&code, "#include <stdio.h>\n\nint main() {\n");

    Scanner sc; scanner_init(&sc, src);
    for (;;) {
        Token t = scan_token(&sc);
        if (t.type == T_EOF) break;
        if (t.type == T_ERROR) { free(code.data); exit(1); } // stop on error
        if (t.type == T_NEWLINE) continue;
        if (t.type != T_PRINT) {
            print_error(&sc, "Expected 'print' statement");
            free(code.data);
            exit(1);
        }

        Token next = scan_token(&sc);
        if (next.type == T_ERROR) { free(code.data); exit(1); }
        if (next.type == T_STRING) {
            buf_append_fmt(&code, "    printf(\"%.*s\\n\");\n", (int)next.as.str.len, next.as.str.ptr);
        }
        else if (next.type == T_FLOAT) {
            buf_append_fmt(&code, "    printf(\"%%g\\n\", %.*s);\n", (int)next.as.str.len, next.as.str.ptr);
        }
        else if (next.type == T_INT) {
            buf_append_fmt(&code, "    printf(\"%%d\\n\", %.*s);\n", (int)next.as.str.len, next.as.str.ptr);
        }
        else {
            print_error(&sc, "Unexpected token after print. Expected string or number.\n");
            free(code.data);
            exit(1);
        }

        Token end = scan_token(&sc);
        if (end.type == T_ERROR) { free(code.data); exit(1); }
        if (end.type != T_NEWLINE && end.type != T_EOF) {
            print_error(&sc, "Expected newline after print statement\n");
            free(code.data);
            exit(1);
        }
    }

    buf_append(&code, "    return 0;\n}\n");

    FILE* out = fopen(out_path, "w");
    if (!out) {
        fprintf(stderr, "Cannot write to %s\n", out_path);
        free(code.data);
        exit(1);
    }
    fwrite(code.data, 1, code.len, out);
    fclose(out);
    free(code.data);
}

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
