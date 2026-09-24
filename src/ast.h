#ifndef AVA_AST_H
#define AVA_AST_H

#include "source.h"

#include <stdbool.h>
#include <stddef.h>

/** A binding with an optional type annotation and an integer literal initializer. */
typedef struct {
    AvaSpan span;
    AvaSpan name;
    bool has_type;
    AvaSpan type;
    AvaSpan value;
} AvaAstLet;

/** Owns the array of declarations. Spans borrow text from the source. */
typedef struct {
    AvaAstLet *items;
    size_t count;
    size_t capacity;
} AvaAstProgram;

void ava_ast_program_dispose(AvaAstProgram *program);

#endif
