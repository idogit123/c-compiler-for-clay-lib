#ifndef AST_H
#define AST_H

#include <stddef.h>
#include "scanner.h"

typedef enum {
    LIT_STRING,
    LIT_INT,
    LIT_FLOAT
} LiteralKind;

typedef struct Expr Expr;
typedef struct Stmt Stmt;

typedef enum {
    EXPR_LIT,
    EXPR_ADD
} ExprKind;

struct Expr {
    ExprKind kind;
    int line, col;
    union {
        struct {
            LiteralKind kind;
            Slice value; // points to original source
        } literal;
        struct {
            struct Expr* left;
            struct Expr* right;
        } add;
    } as;
};

typedef enum {
    STMT_PRINT
} StmtKind;

struct Stmt {
    StmtKind kind;
    int line, col;
    union {
        struct {
            Expr* expr;
        } print;
    } as;
};

typedef struct {
    Stmt** items;
    size_t count;
    size_t capacity;
} StmtArray;

typedef struct {
    StmtArray stmts;
} Program;

// Constructors / utils
Program* ast_new_program(void);
void ast_stmt_array_push(StmtArray* arr, Stmt* s);

Expr* ast_new_literal_expr(LiteralKind kind, Slice value, int line, int col);
Expr* ast_new_add_expr(Expr* left, Expr* right, int line, int col);
Stmt* ast_new_print_stmt(Expr* expr, int line, int col);

// Destruction
void ast_free_program(Program* p);

#endif // AST_H
