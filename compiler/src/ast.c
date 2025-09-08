#include <stdlib.h>
#include <stdio.h>
#include "ast.h"

static void* xmalloc(size_t n) {
    void* p = malloc(n);
    if (!p) { fprintf(stderr, "Compiler Error: Out of memory\n"); exit(1); }
    return p;
}

Program* ast_new_program(void) {
    Program* p = (Program*)xmalloc(sizeof(Program));
    p->stmts.items = NULL;
    p->stmts.count = 0;
    p->stmts.capacity = 0;
    return p;
}

void ast_stmt_array_push(StmtArray* arr, Stmt* s) {
    if (arr->count + 1 > arr->capacity) {
        size_t newCap = arr->capacity < 4 ? 4 : arr->capacity * 2;
        arr->items = (Stmt**)realloc(arr->items, newCap * sizeof(Stmt*));
        if (!arr->items) { fprintf(stderr, "Compiler Error: Out of memory\n"); exit(1); }
        arr->capacity = newCap;
    }
    arr->items[arr->count++] = s;
}

Expr* ast_new_literal_expr(LiteralKind kind, Slice value, int line, int col) {
    Expr* e = (Expr*)xmalloc(sizeof(Expr));
    e->kind = EXPR_LIT;
    e->line = line;
    e->col = col;
    e->as.literal.kind = kind;
    e->as.literal.value = value;
    return e;
}

Stmt* ast_new_print_stmt(Expr* expr, int line, int col) {
    Stmt* s = (Stmt*)xmalloc(sizeof(Stmt));
    s->kind = STMT_PRINT;
    s->line = line;
    s->col = col;
    s->as.print.expr = expr;
    return s;
}

Expr* ast_new_add_expr(Expr* left, Expr* right, int line, int col) {
    Expr* e = (Expr*)xmalloc(sizeof(Expr));
    e->kind = EXPR_ADD;
    e->line = line;
    e->col = col;
    e->as.add.left = left;
    e->as.add.right = right;
    return e;
}

static void ast_free_expr(Expr* e) {
    if (!e) return;
    switch (e->kind) {
        case EXPR_ADD:
            ast_free_expr(e->as.add.left);
            ast_free_expr(e->as.add.right);
            break;
        default:
            break;
    }
    free(e);
}

static void ast_free_stmt(Stmt* s) {
    if (!s) return;
    switch (s->kind) {
        case STMT_PRINT:
            ast_free_expr(s->as.print.expr);
            break;
    }
    free(s);
}

void ast_free_program(Program* p) {
    if (!p) return;
    for (size_t i = 0; i < p->stmts.count; ++i) {
        ast_free_stmt(p->stmts.items[i]);
    }
    free(p->stmts.items);
    free(p);
}
