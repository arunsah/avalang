#include "lexer.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

enum { INITIAL_TOKEN_CAPACITY = 16 };

static AvaLexResult make_lex_result(AvaLexStatus status, AvaSpan span) {
    return (AvaLexResult){
        .status = status,
        .span = span,
    };
}

static bool is_digit(unsigned char character) {
    return character >= '0' && character <= '9';
}

static bool is_identifier_start(unsigned char character) {
    return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') ||
           character == '_';
}

static bool is_identifier_continue(unsigned char character) {
    return is_identifier_start(character) || is_digit(character);
}

static bool is_whitespace(unsigned char character) {
    return character == ' ' || character == '\t' || character == '\n' || character == '\r';
}

static bool span_equals(const AvaSource *source, AvaSpan span, const char *text) {
    size_t text_length = strlen(text);
    size_t span_length = span.end - span.start;

    return span_length == text_length && memcmp(source->bytes + span.start, text, text_length) == 0;
}

static AvaTokenKind identifier_kind(const AvaSource *source, AvaSpan span) {
    if (span_equals(source, span, "let")) {
        return AVA_TOKEN_LET;
    }

    return AVA_TOKEN_IDENTIFIER;
}

static AvaLexStatus push_token(AvaTokenList *tokens, AvaTokenKind kind, AvaSpan span) {
    if (tokens->count >= AVA_LEX_MAX_TOKENS) {
        return AVA_LEX_TOO_MANY_TOKENS;
    }

    if (tokens->count == tokens->capacity) {
        size_t new_capacity = tokens->capacity == 0 ? INITIAL_TOKEN_CAPACITY : tokens->capacity * 2;

        if (new_capacity > AVA_LEX_MAX_TOKENS) {
            new_capacity = AVA_LEX_MAX_TOKENS;
        }

        AvaToken *new_items = realloc(tokens->items, new_capacity * sizeof(*new_items));

        if (new_items == NULL) {
            return AVA_LEX_OUT_OF_MEMORY;
        }

        tokens->items = new_items;
        tokens->capacity = new_capacity;
    }

    tokens->items[tokens->count] = (AvaToken){
        .kind = kind,
        .span = span,
    };
    tokens->count += 1;

    return AVA_LEX_OK;
}

static AvaLexResult fail_lexing(AvaTokenList *tokens, AvaLexStatus status, AvaSpan span) {
    ava_token_list_dispose(tokens);
    return make_lex_result(status, span);
}

AvaLexResult ava_lex(const AvaSource *source, AvaTokenList *tokens) {
    if (source == NULL || source->bytes == NULL || tokens == NULL) {
        return make_lex_result(AVA_LEX_INVALID_ARGUMENT, (AvaSpan){0});
    }

    if (tokens->items != NULL || tokens->count != 0 || tokens->capacity != 0) {
        return make_lex_result(AVA_LEX_DESTINATION_NOT_EMPTY, (AvaSpan){0});
    }

    size_t index = 0;

    while (index < source->length) {
        unsigned char character = (unsigned char)source->bytes[index];

        if (is_whitespace(character)) {
            index += 1;
            continue;
        }

        if (is_identifier_start(character)) {
            size_t start = index;
            index += 1;

            while (index < source->length &&
                   is_identifier_continue((unsigned char)source->bytes[index])) {
                index += 1;
            }

            AvaSpan span = {
                .start = start,
                .end = index,
            };
            AvaLexStatus status = push_token(tokens, identifier_kind(source, span), span);

            if (status != AVA_LEX_OK) {
                return fail_lexing(tokens, status, span);
            }

            continue;
        }

        if (is_digit(character)) {
            size_t start = index;
            index += 1;

            while (index < source->length && is_digit((unsigned char)source->bytes[index])) {
                index += 1;
            }

            AvaSpan span = {
                .start = start,
                .end = index,
            };
            AvaLexStatus status = push_token(tokens, AVA_TOKEN_INTEGER, span);

            if (status != AVA_LEX_OK) {
                return fail_lexing(tokens, status, span);
            }

            continue;
        }

        AvaSpan span = {
            .start = index,
            .end = index + 1,
        };
        AvaTokenKind kind;

        switch (character) {
        case ':':
            kind = AVA_TOKEN_COLON;
            break;
        case '=':
            kind = AVA_TOKEN_EQUAL;
            break;
        default:
            return fail_lexing(tokens, AVA_LEX_UNEXPECTED_CHARACTER, span);
        }

        AvaLexStatus status = push_token(tokens, kind, span);

        if (status != AVA_LEX_OK) {
            return fail_lexing(tokens, status, span);
        }

        index += 1;
    }

    AvaSpan eof_span = {
        .start = source->length,
        .end = source->length,
    };
    AvaLexStatus status = push_token(tokens, AVA_TOKEN_EOF, eof_span);

    if (status != AVA_LEX_OK) {
        return fail_lexing(tokens, status, eof_span);
    }

    return make_lex_result(AVA_LEX_OK, eof_span);
}
