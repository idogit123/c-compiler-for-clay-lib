#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "scanner.h"

// Parse the whole program. Fail-fast on first error.
// Returns a Program* on success; sets *hadError and may return NULL on error.
Program* parse_program(Scanner* sc, int* hadError);

#endif // PARSER_H
