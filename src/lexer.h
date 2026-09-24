#ifndef AVA_LEXER_H
#define AVA_LEXER_H

#include "source.h"
#include "token.h"

#include <stddef.h>

#define AVA_LEX_MAX_TOKENS ((size_t)4 * 1024U * 1024U)

typedef enum {
    AVA_LEX_OK = 0,
    AVA_LEX_INVALID_ARGUMENT,
    AVA_LEX_DESTINATION_NOT_EMPTY,
    AVA_LEX_UNEXPECTED_CHARACTER,
    AVA_LEX_TOO_MANY_TOKENS,
    AVA_LEX_OUT_OF_MEMORY
} AvaLexStatus;

typedef struct {
    AvaLexStatus status;
    AvaSpan span;
} AvaLexResult;

/**
 * Converts a loaded source into tokens.
 *
 * The destination must be an empty token list.
 * On success, tokens owns the generated token sequence.
 * On failure after lexing begins, tokens is returned to its empty state.
 *
 * The lexer borrows source and does not render diagnostics.
 */
AvaLexResult ava_lex(const AvaSource *source, AvaTokenList *tokens);

#endif
