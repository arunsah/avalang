#ifndef AVA_AST_DUMP_H
#define AVA_AST_DUMP_H

#include "ast.h"

#include <stdbool.h>
#include <stdio.h>

/** Writes the syntax tree using source spans and the source's original text. */
bool ava_ast_dump(FILE *stream, const AvaSource *source, const AvaAstProgram *program);

#endif
