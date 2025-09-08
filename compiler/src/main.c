#include <stdio.h>
#include <stdlib.h>
#include "buffer.h"
#include "scanner.h"
#include "error.h"
#include "parser.h"
#include "codegen.h"

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

    Scanner sc; scanner_init(&sc, src);
    int hadError = 0;
    Program* prog = parse_program(&sc, &hadError);
    if (hadError || !prog) {
        free(code.data);
        exit(1);
    }

    codegen_c_program(&code, prog);
    ast_free_program(prog);

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
