#include "codegen.h"

static void emit_stmt(Buffer* out, const Stmt* s) {
    switch (s->kind) {
        case STMT_PRINT: {
            const Expr* e = s->as.print.expr;
            switch (e->kind) {
                case EXPR_LIT: {
                    switch (e->as.literal.kind) {
                        case LIT_STRING:
                            buf_append_fmt(out, "    printf(\"%.*s\\n\");\n", (int)e->as.literal.value.len, e->as.literal.value.ptr);
                            break;
                        case LIT_INT:
                            buf_append_fmt(out, "    printf(\"%%d\\n\", %.*s);\n", (int)e->as.literal.value.len, e->as.literal.value.ptr);
                            break;
                        case LIT_FLOAT:
                            buf_append_fmt(out, "    printf(\"%%g\\n\", %.*s);\n", (int)e->as.literal.value.len, e->as.literal.value.ptr);
                            break;
                    }
                    break;
                }
            }
            break;
        }
    }
}

void codegen_c_program(Buffer* out, const Program* prog) {
    buf_append(out, "#include <stdio.h>\n\n");
    buf_append(out, "int main() {\n");

    for (size_t i = 0; i < prog->stmts.count; ++i) {
        const Stmt* s = prog->stmts.items[i];
        if (s) emit_stmt(out, s);
    }

    buf_append(out, "    return 0;\n}\n");
}
