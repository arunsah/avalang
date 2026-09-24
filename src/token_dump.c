#include "token_dump.h"

static bool write_escaped_byte(FILE *stream, unsigned char byte) {
    switch (byte) {
    case '\\':
        return fputs("\\\\", stream) >= 0;
    case '"':
        return fputs("\\\"", stream) >= 0;
    case '\n':
        return fputs("\\n", stream) >= 0;
    case '\r':
        return fputs("\\r", stream) >= 0;
    case '\t':
        return fputs("\\t", stream) >= 0;
    default:
        if (byte < 0x20U || byte >= 0x7fU) {
            return fprintf(stream, "\\x%02X", (unsigned int)byte) >= 0;
        }
        return fputc((int)byte, stream) != EOF;
    }
}

bool ava_token_dump(FILE *stream, const AvaSource *source, const AvaTokenList *tokens) {
    if (stream == NULL || source == NULL || source->bytes == NULL || tokens == NULL ||
        tokens->items == NULL || tokens->count == 0) {
        return false;
    }

    for (size_t index = 0; index < tokens->count; index += 1) {
        const AvaToken *token = &tokens->items[index];
        if (token->span.start > token->span.end || token->span.end > source->length) {
            return false;
        }

        size_t line = 0;
        size_t column = 0;
        if (!ava_source_location(source, token->span.start, &line, &column)) {
            return false;
        }

        if (fprintf(stream, "%zu:%zu %-10s ", line, column, ava_token_kind_name(token->kind)) < 0) {
            return false;
        }

        if (token->kind == AVA_TOKEN_EOF) {
            if (fputs("<eof>\n", stream) < 0) {
                return false;
            }
            continue;
        }

        if (fputc('"', stream) == EOF) {
            return false;
        }
        for (size_t byte_index = token->span.start; byte_index < token->span.end; byte_index += 1) {
            if (!write_escaped_byte(stream, (unsigned char)source->bytes[byte_index])) {
                return false;
            }
        }
        if (fputs("\"\n", stream) < 0) {
            return false;
        }
    }

    return ferror(stream) == 0;
}
