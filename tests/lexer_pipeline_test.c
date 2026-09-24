#include "diagnostic.h"
#include "lexer.h"
#include "lexer_diagnostic.h"
#include "source.h"
#include "token_dump.h"

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

static void test_valid_source(const char *path) {
    AvaSource source = {0};
    AvaSourceLoadResult load = ava_source_load(path, &source);
    CHECK(load.status == AVA_SOURCE_LOAD_OK);
    if (load.status != AVA_SOURCE_LOAD_OK) {
        return;
    }

    AvaTokenList tokens = {0};
    AvaLexResult lex = ava_lex(&source, &tokens);
    CHECK(lex.status == AVA_LEX_OK);
    if (lex.status == AVA_LEX_OK) {
        FILE *stream = tmpfile();
        CHECK(stream != NULL);
        if (stream != NULL) {
            CHECK(ava_token_dump(stream, &source, &tokens));
            CHECK(stream_equals(stream, "1:1 let        \"let\"\n"
                                        "1:5 identifier \"answer\"\n"
                                        "1:11 :          \":\"\n"
                                        "1:13 identifier \"i32\"\n"
                                        "1:17 =          \"=\"\n"
                                        "1:19 integer    \"42\"\n"
                                        "2:1 eof        <eof>\n"));
            CHECK(fclose(stream) == 0);
        }
    }

    ava_token_list_dispose(&tokens);
    ava_source_dispose(&source);
}

static void test_invalid_source(const char *path) {
    AvaSource source = {0};
    AvaSourceLoadResult load = ava_source_load(path, &source);
    CHECK(load.status == AVA_SOURCE_LOAD_OK);
    if (load.status != AVA_SOURCE_LOAD_OK) {
        return;
    }

    AvaTokenList tokens = {0};
    AvaLexResult lex = ava_lex(&source, &tokens);
    CHECK(lex.status == AVA_LEX_UNEXPECTED_CHARACTER);
    CHECK(tokens.items == NULL);

    AvaDiagnostic diagnostic = {0};
    CHECK(ava_lex_result_to_diagnostic(&source, lex, &diagnostic));
    CHECK(diagnostic.path == source.path);
    CHECK(diagnostic.line == 1);
    CHECK(diagnostic.column == 19);
    CHECK(strcmp(diagnostic.code, "AVA_LEX_UNEXPECTED_CHARACTER") == 0);

    FILE *stream = tmpfile();
    CHECK(stream != NULL);
    if (stream != NULL) {
        CHECK(ava_diagnostic_render(stream, &diagnostic));
        char expected[1024];
        int length = snprintf(expected, sizeof(expected),
                              "%s:1:19: error[AVA_LEX_UNEXPECTED_CHARACTER]: unexpected character\n"
                              "help: this lexer currently accepts ASCII identifiers, decimal "
                              "integers, ':', and '='\n",
                              path);
        CHECK(length >= 0 && (size_t)length < sizeof(expected));
        if (length >= 0 && (size_t)length < sizeof(expected)) {
            CHECK(stream_equals(stream, expected));
        }
        CHECK(fclose(stream) == 0);
    }

    ava_token_list_dispose(&tokens);
    ava_source_dispose(&source);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <valid-source> <invalid-source>\n", argv[0]);
        return 2;
    }

    test_valid_source(argv[1]);
    test_invalid_source(argv[2]);

    if (failure_count != 0) {
        fprintf(stderr, "%d lexer pipeline test(s) failed\n", failure_count);
        return 1;
    }
    return 0;
}
