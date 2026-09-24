#ifndef AVA_TOKEN_H
#define AVA_TOKEN_H

#include "source.h"

#include <stddef.h>

/**
 * Identifies the lexical meaning of a token.
 */
typedef enum {
    AVA_TOKEN_IDENTIFIER = 0,
    AVA_TOKEN_INTEGER,
    AVA_TOKEN_LET,
    AVA_TOKEN_COLON,
    AVA_TOKEN_EQUAL,
    AVA_TOKEN_EOF
} AvaTokenKind;

/**
 * Represents one token as a kind and a byte range in its source.
 */
typedef struct {
    AvaTokenKind kind;
    AvaSpan span;
} AvaToken;

/**
 * Owns a dynamically allocated sequence of tokens.
 *
 * The zero-initialized value is an empty list.
 */
typedef struct {
    AvaToken *items;
    size_t count;
    size_t capacity;
} AvaTokenList;

/**
 * Returns the readable name of a token kind.
 */
const char *ava_token_kind_name(AvaTokenKind kind);

/**
 * Releases the token storage and resets the list to its empty state.
 *
 * Passing NULL or disposing an empty list is allowed.
 */
void ava_token_list_dispose(AvaTokenList *tokens);

#endif
