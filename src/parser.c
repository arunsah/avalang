#include "parser.h"

#include <stdint.h>
#include <stdlib.h>

enum { INITIAL_AST_CAPACITY = 8 };

static AvaParseResult parse_result(AvaParseStatus status, AvaSpan span) {
    return (AvaParseResult){.status = status, .span = span};
}

static AvaParseResult fail_parse(AvaAstProgram *program, AvaParseStatus status, AvaSpan span) {
    ava_ast_program_dispose(program);
    return parse_result(status, span);
}

static bool push_let(AvaAstProgram *program, AvaAstLet binding) {
    if (program->count == program->capacity) {
        size_t capacity = program->capacity == 0 ? INITIAL_AST_CAPACITY : program->capacity * 2;
        if (capacity < program->capacity || capacity > SIZE_MAX / sizeof(*program->items)) {
            return false;
        }
        AvaAstLet *items = realloc(program->items, capacity * sizeof(*items));
        if (items == NULL) {
            return false;
        }
        program->items = items;
        program->capacity = capacity;
    }

    program->items[program->count] = binding;
    program->count += 1;
    return true;
}

AvaParseResult ava_parse(const AvaTokenList *tokens, AvaAstProgram *program) {
    if (tokens == NULL || tokens->items == NULL || tokens->count == 0 || program == NULL ||
        tokens->items[tokens->count - 1].kind != AVA_TOKEN_EOF) {
        return parse_result(AVA_PARSE_INVALID_ARGUMENT, (AvaSpan){0});
    }
    if (program->items != NULL || program->count != 0 || program->capacity != 0) {
        return parse_result(AVA_PARSE_DESTINATION_NOT_EMPTY, (AvaSpan){0});
    }

    size_t index = 0;
    while (index < tokens->count - 1) {
        const AvaToken *token = &tokens->items[index];
        if (token->kind != AVA_TOKEN_LET) {
            return fail_parse(program, AVA_PARSE_EXPECTED_LET, token->span);
        }

        AvaAstLet binding = {.span.start = token->span.start};
        index += 1;
        token = &tokens->items[index];
        if (token->kind != AVA_TOKEN_IDENTIFIER) {
            return fail_parse(program, AVA_PARSE_EXPECTED_NAME, token->span);
        }
        binding.name = token->span;

        index += 1;
        token = &tokens->items[index];
        if (token->kind == AVA_TOKEN_COLON) {
            index += 1;
            token = &tokens->items[index];
            if (token->kind != AVA_TOKEN_IDENTIFIER) {
                return fail_parse(program, AVA_PARSE_EXPECTED_TYPE, token->span);
            }
            binding.has_type = true;
            binding.type = token->span;
            index += 1;
            token = &tokens->items[index];
        }

        if (token->kind != AVA_TOKEN_EQUAL) {
            return fail_parse(program, AVA_PARSE_EXPECTED_EQUAL, token->span);
        }
        index += 1;
        token = &tokens->items[index];
        if (token->kind != AVA_TOKEN_INTEGER) {
            return fail_parse(program, AVA_PARSE_EXPECTED_INTEGER, token->span);
        }
        binding.value = token->span;
        binding.span.end = token->span.end;
        if (!push_let(program, binding)) {
            return fail_parse(program, AVA_PARSE_OUT_OF_MEMORY, token->span);
        }
        index += 1;
    }

    return parse_result(AVA_PARSE_OK, tokens->items[index].span);
}
