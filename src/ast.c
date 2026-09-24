#include "ast.h"

#include <stdlib.h>

void ava_ast_program_dispose(AvaAstProgram *program) {
    if (program == NULL) {
        return;
    }

    free(program->items);
    *program = (AvaAstProgram){0};
}
