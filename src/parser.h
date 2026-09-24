#ifndef AVA_PARSER_H
#define AVA_PARSER_H

#include "ast.h"
#include "token.h"

typedef enum {
    AVA_PARSE_OK = 0,
    AVA_PARSE_INVALID_ARGUMENT,
    AVA_PARSE_DESTINATION_NOT_EMPTY,
    AVA_PARSE_EXPECTED_LET,
    AVA_PARSE_EXPECTED_NAME,
    AVA_PARSE_EXPECTED_TYPE,
    AVA_PARSE_EXPECTED_EQUAL,
    AVA_PARSE_EXPECTED_INTEGER,
    AVA_PARSE_OUT_OF_MEMORY
} AvaParseStatus;

typedef struct {
    AvaParseStatus status;
    AvaSpan span;
} AvaParseResult;

/** Parses a sequence of let bindings from a token list ending in EOF.
 * The destination must be empty. On a syntax or allocation failure, partial AST nodes
 * are released. The tokens and source text remain owned by the caller.
 */
AvaParseResult ava_parse(const AvaTokenList *tokens, AvaAstProgram *program);

#endif
