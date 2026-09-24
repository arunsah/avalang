#include "ast_dump.h"
#include "lexer.h"
#include "parser.h"
#include "parser_diagnostic.h"

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
    return (AvaSource){.path = "example.ava", .bytes = text, .length = strlen(text)};
}

static bool stream_equals(FILE *stream, const char *expected) {
    if (fflush(stream) != 0 || fseek(stream, 0, SEEK_SET) != 0) {
        return false;
    }
    for (size_t index = 0; expected[index] != '\0'; index += 1) {
        if (fgetc(stream) != (unsigned char)expected[index]) {
            return false;
        }
    }
    return fgetc(stream) == EOF && ferror(stream) == 0;
}

static void test_bindings(void) {
    char text[] = "let answer: i32 = 42\nlet next = 7\n";
    AvaSource source = source_from_text(text);
    AvaTokenList tokens = {0};
    AvaAstProgram program = {0};

    CHECK(ava_lex(&source, &tokens).status == AVA_LEX_OK);
    AvaParseResult result = ava_parse(&tokens, &program);
    CHECK(result.status == AVA_PARSE_OK);
    CHECK(program.count == 2);

    if (program.count == 2) {
        CHECK(program.items[0].has_type);
        CHECK(program.items[0].name.start == 4);
        CHECK(program.items[0].type.start == 12);
        CHECK(program.items[0].value.start == 18);
        CHECK(!program.items[1].has_type);
        CHECK(program.items[1].name.start == 25);

        FILE *stream = tmpfile();
        CHECK(stream != NULL);
        if (stream != NULL) {
            CHECK(ava_ast_dump(stream, &source, &program));
            CHECK(stream_equals(stream, "let [0,20)\n"
                                        "  name [4,10): \"answer\"\n"
                                        "  type [12,15): \"i32\"\n"
                                        "  integer [18,20): \"42\"\n"
                                        "let [21,33)\n"
                                        "  name [25,29): \"next\"\n"
                                        "  integer [32,33): \"7\"\n"));
            CHECK(fclose(stream) == 0);
        }
    }

    ava_ast_program_dispose(&program);
    CHECK(program.items == NULL && program.count == 0 && program.capacity == 0);
    ava_token_list_dispose(&tokens);
}

static void test_missing_type(void) {
    char text[] = "let answer: = 42";
    AvaSource source = source_from_text(text);
    AvaTokenList tokens = {0};
    AvaAstProgram program = {0};

    CHECK(ava_lex(&source, &tokens).status == AVA_LEX_OK);
    AvaParseResult result = ava_parse(&tokens, &program);
    CHECK(result.status == AVA_PARSE_EXPECTED_TYPE);
    CHECK(result.span.start == 12);
    CHECK(program.items == NULL && program.count == 0);

    AvaDiagnostic diagnostic = {0};
    CHECK(ava_parse_result_to_diagnostic(&source, result, &diagnostic));
    CHECK(diagnostic.line == 1 && diagnostic.column == 13);
    CHECK(strcmp(diagnostic.code, "AVA_PARSE_EXPECTED_TYPE") == 0);

    ava_ast_program_dispose(&program);
    ava_token_list_dispose(&tokens);
}

static void test_missing_initializer_at_eof(void) {
    char text[] = "let answer =";
    AvaSource source = source_from_text(text);
    AvaTokenList tokens = {0};
    AvaAstProgram program = {0};

    CHECK(ava_lex(&source, &tokens).status == AVA_LEX_OK);
    AvaParseResult result = ava_parse(&tokens, &program);
    CHECK(result.status == AVA_PARSE_EXPECTED_INTEGER);
    CHECK(result.span.start == source.length && result.span.end == source.length);
    ava_ast_program_dispose(&program);
    ava_token_list_dispose(&tokens);
}

static void test_partial_program_cleanup(void) {
    char text[] = "let first = 1\nlet second: = 2";
    AvaSource source = source_from_text(text);
    AvaTokenList tokens = {0};
    AvaAstProgram program = {0};

    CHECK(ava_lex(&source, &tokens).status == AVA_LEX_OK);
    AvaParseResult result = ava_parse(&tokens, &program);
    CHECK(result.status == AVA_PARSE_EXPECTED_TYPE);
    CHECK(program.items == NULL && program.count == 0 && program.capacity == 0);
    ava_token_list_dispose(&tokens);
}

static void test_nonempty_destination(void) {
    char text[] = "let x = 1";
    AvaSource source = source_from_text(text);
    AvaTokenList tokens = {0};
    AvaAstProgram program = {0};

    CHECK(ava_lex(&source, &tokens).status == AVA_LEX_OK);
    CHECK(ava_parse(&tokens, &program).status == AVA_PARSE_OK);
    AvaAstLet *original = program.items;
    CHECK(ava_parse(&tokens, &program).status == AVA_PARSE_DESTINATION_NOT_EMPTY);
    CHECK(program.items == original && program.count == 1);

    ava_ast_program_dispose(&program);
    ava_token_list_dispose(&tokens);
}

int main(void) {
    test_bindings();
    test_missing_type();
    test_missing_initializer_at_eof();
    test_partial_program_cleanup();
    test_nonempty_destination();

    if (failure_count != 0) {
        fprintf(stderr, "%d parser test(s) failed\n", failure_count);
        return 1;
    }
    return 0;
}
