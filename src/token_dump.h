#ifndef AVA_TOKEN_DUMP_H
#define AVA_TOKEN_DUMP_H

#include "source.h"
#include "token.h"

#include <stdbool.h>
#include <stdio.h>

/** Writes one token per line, escaping source bytes that would obscure the output. */
bool ava_token_dump(FILE *stream, const AvaSource *source, const AvaTokenList *tokens);

#endif
