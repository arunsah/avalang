#include "token.h"

#include <stdlib.h>

const char *ava_token_kind_name(AvaTokenKind kind) {
    switch (kind) {
    case AVA_TOKEN_IDENTIFIER:
        return "identifier";
    case AVA_TOKEN_INTEGER:
        return "integer";
    case AVA_TOKEN_LET:
        return "let";
    case AVA_TOKEN_COLON:
        return ":";
    case AVA_TOKEN_EQUAL:
        return "=";
    case AVA_TOKEN_EOF:
        return "eof";
    }

    return "unknown";
}

void ava_token_list_dispose(AvaTokenList *tokens) {
    if (tokens == NULL) {
        return;
    }

    free(tokens->items);
    *tokens = (AvaTokenList){0};
}
