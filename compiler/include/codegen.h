#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "buffer.h"

void codegen_c_program(Buffer* out, const Program* prog);

#endif // CODEGEN_H
