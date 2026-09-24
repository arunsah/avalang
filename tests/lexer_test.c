#include "lexer.h"

#include <stdio.h>
#include <string.h>

static int failure_count = 0;

#define CHECK(condition)                                                                           \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition);          \
            failure_count += 1;                                                                    \
        }                                                                                          \
    } while (0)

static AvaSource source_from_text(char *text) {
    return (AvaSource){
        .path = NULL,
        .bytes = text,
        .length = strlen(text),
    };
}

static void check_token(const AvaTokenList *tokens, size_t index, AvaTokenKind kind, size_t start,
                        size_t end) {
    CHECK(index < tokens->count);

    if (index >= tokens->count) {
        return;
    }

    CHECK(tokens->items[index].kind == kind);
    CHECK(tokens->items[index].span.start == start);
    CHECK(tokens->items[index].span.end == end);
}

static void test_declaration(void) {
    char text[] = "let answer: i32 = 42";
    AvaSource source = source_from_text(text);
    AvaTokenList tokens = {0};

    AvaLexResult result = ava_lex(&source, &tokens);

    CHECK(result.status == AVA_LEX_OK);
    CHECK(tokens.count == 7);

    check_token(&tokens, 0, AVA_TOKEN_LET, 0, 3);
    check_token(&tokens, 1, AVA_TOKEN_IDENTIFIER, 4, 10);
    check_token(&tokens, 2, AVA_TOKEN_COLON, 10, 11);
    check_token(&tokens, 3, AVA_TOKEN_IDENTIFIER, 12, 15);
    check_token(&tokens, 4, AVA_TOKEN_EQUAL, 16, 17);
    check_token(&tokens, 5, AVA_TOKEN_INTEGER, 18, 20);
    check_token(&tokens, 6, AVA_TOKEN_EOF, 20, 20);

    ava_token_list_dispose(&tokens);
    CHECK(tokens.items == NULL);
    CHECK(tokens.count == 0);
    CHECK(tokens.capacity == 0);
}

static void test_empty_source(void) {
    char text[] = "";
    AvaSource source = source_from_text(text);
    AvaTokenList tokens = {0};

    AvaLexResult result = ava_lex(&source, &tokens);

    CHECK(result.status == AVA_LEX_OK);
    CHECK(tokens.count == 1);
    check_token(&tokens, 0, AVA_TOKEN_EOF, 0, 0);

    ava_token_list_dispose(&tokens);
}

static void test_keyword_boundary(void) {
    char text[] = "let letdown _let let2";
    AvaSource source = source_from_text(text);
    AvaTokenList tokens = {0};

    AvaLexResult result = ava_lex(&source, &tokens);

    CHECK(result.status == AVA_LEX_OK);
    CHECK(tokens.count == 5);
    check_token(&tokens, 0, AVA_TOKEN_LET, 0, 3);
    check_token(&tokens, 1, AVA_TOKEN_IDENTIFIER, 4, 11);
    check_token(&tokens, 2, AVA_TOKEN_IDENTIFIER, 12, 16);
    check_token(&tokens, 3, AVA_TOKEN_IDENTIFIER, 17, 21);
    check_token(&tokens, 4, AVA_TOKEN_EOF, 21, 21);

    ava_token_list_dispose(&tokens);
}

static void test_failure_discards_partial_tokens(void) {
    char text[] = "let @";
    AvaSource source = source_from_text(text);
    AvaTokenList tokens = {0};

    AvaLexResult result = ava_lex(&source, &tokens);

    CHECK(result.status == AVA_LEX_UNEXPECTED_CHARACTER);
    CHECK(result.span.start == 4);
    CHECK(result.span.end == 5);
    CHECK(tokens.items == NULL);
    CHECK(tokens.count == 0);
    CHECK(tokens.capacity == 0);
}

static void test_nonempty_destination_is_preserved(void) {
    char text[] = "let";
    AvaSource source = source_from_text(text);
    AvaTokenList tokens = {0};

    CHECK(ava_lex(&source, &tokens).status == AVA_LEX_OK);

    AvaToken *original_items = tokens.items;
    size_t original_count = tokens.count;

    AvaLexResult result = ava_lex(&source, &tokens);

    CHECK(result.status == AVA_LEX_DESTINATION_NOT_EMPTY);
    CHECK(tokens.items == original_items);
    CHECK(tokens.count == original_count);

    ava_token_list_dispose(&tokens);
}

int main(void) {
    test_declaration();
    test_empty_source();
    test_keyword_boundary();
    test_failure_discards_partial_tokens();
    test_nonempty_destination_is_preserved();

    if (failure_count != 0) {
        fprintf(stderr, "%d lexer test(s) failed\n", failure_count);
        return 1;
    }

    printf("all lexer tests passed\n");
    return 0;
}
