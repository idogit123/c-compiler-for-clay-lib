#include <stdlib.h>
#include <stdio.h>
#include "parser.h"
#include "error.h"

typedef struct {
    Scanner* sc;
    Token current;
    Token previous;
    int hadError;
} Parser;

static void advance_p(Parser* p) {
    p->previous = p->current;
    p->current = scan_token(p->sc);
    if (p->current.type == T_ERROR) {
        // print_error already called by scanner
        p->hadError = 1;
    }
}

static Expr* parse_literal(Parser* p) {
    Token t = p->current;
    switch (t.type) {
        case T_STRING: {
            advance_p(p);
            return ast_new_literal_expr(LIT_STRING, t.as.str, t.line, t.col);
        }
        case T_INT: {
            advance_p(p);
            return ast_new_literal_expr(LIT_INT, t.as.str, t.line, t.col);
        }
        case T_FLOAT: {
            advance_p(p);
            return ast_new_literal_expr(LIT_FLOAT, t.as.str, t.line, t.col);
        }
        default:
            print_error(p->sc, "Expected a literal after 'print'");
            p->hadError = 1;
            return NULL;
    }
}

static Stmt* parse_print(Parser* p) {
    Token kw = p->current; // currently at T_PRINT
    advance_p(p); // consume 'print'
    Expr* e = parse_literal(p);
    if (p->hadError) return NULL;

    // Accept NEWLINE or EOF immediately following
    if (p->current.type == T_NEWLINE) {
        advance_p(p);
    } else if (p->current.type != T_EOF) {
        print_error(p->sc, "Expected newline after print statement");
        p->hadError = 1;
        return NULL;
    }

    return ast_new_print_stmt(e, kw.line, kw.col);
}

static Stmt* parse_stmt(Parser* p) {
    if (p->current.type == T_PRINT) return parse_print(p);
    if (p->current.type == T_EOF) return NULL;
    if (p->current.type == T_NEWLINE) { advance_p(p); return NULL; }

    print_error(p->sc, "Expected 'print' statement");
    p->hadError = 1;
    return NULL;
}

Program* parse_program(Scanner* sc, int* hadError) {
    Parser p = {0};
    p.sc = sc;
    p.hadError = 0;

    advance_p(&p); // initialize current
    Program* prog = ast_new_program();

    while (p.current.type != T_EOF && !p.hadError) {
        Stmt* s = parse_stmt(&p);
        if (p.hadError) break;
        if (s) ast_stmt_array_push(&prog->stmts, s);
    }

    if (hadError) *hadError = p.hadError;
    if (p.hadError) {
        ast_free_program(prog);
        return NULL;
    }
    return prog;
}
