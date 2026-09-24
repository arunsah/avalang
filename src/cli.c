#include "cli.h"
#include "ast_dump.h"
#include "diagnostic.h"
#include "lexer.h"
#include "lexer_diagnostic.h"
#include "parser.h"
#include "parser_diagnostic.h"
#include "source.h"
#include "source_diagnostic.h"
#include "token_dump.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifndef AVA_VERSION
#define AVA_VERSION "development"
#endif

static void print_summary(FILE *stream, const char *program) {
    fprintf(stream, "AvaLang bootstrap compiler %s\n", AVA_VERSION);
    fprintf(stream, "Inspect AvaLang source files.\n\n");
    fprintf(stream, "Usage: %s [option]\n", program);
    fprintf(stream, "       %s dump <tokens|ast> <file.ava>\n", program);
    fprintf(stream, "Try '%s --help' for more information.\n", program);
}

static void print_help(FILE *stream, const char *program) {
    fprintf(stream, "AvaLang bootstrap compiler\n\n");
    fprintf(stream, "Usage: %s [option]\n", program);
    fprintf(stream, "       %s dump <tokens|ast> <file.ava>\n\n", program);
    fprintf(stream, "Options:\n");
    fprintf(stream, "   -h, --help      Show this help\n");
    fprintf(stream, "   -V, --version   Show version information\n");
    fprintf(stream, "\nCommands:\n");
    fprintf(stream, "   dump tokens <file.ava>   Show source tokens\n");
    fprintf(stream, "   dump ast <file.ava>      Show parsed let bindings\n");
}

static int dump_source(const char *path, bool show_ast) {
    AvaSource source = {0};
    AvaSourceLoadResult load_result = ava_source_load(path, &source);
    if (load_result.status != AVA_SOURCE_LOAD_OK) {
        AvaDiagnostic diagnostic = {0};
        if (ava_source_load_result_to_diagnostic(path, load_result, &diagnostic)) {
            (void)ava_diagnostic_render(stderr, &diagnostic);
        }
        return 2;
    }

    AvaTokenList tokens = {0};
    AvaLexResult lex_result = ava_lex(&source, &tokens);
    if (lex_result.status != AVA_LEX_OK) {
        AvaDiagnostic diagnostic = {0};
        if (ava_lex_result_to_diagnostic(&source, lex_result, &diagnostic)) {
            (void)ava_diagnostic_render(stderr, &diagnostic);
        }
        ava_source_dispose(&source);
        return 2;
    }

    AvaAstProgram program = {0};
    if (show_ast) {
        AvaParseResult parse_result = ava_parse(&tokens, &program);
        if (parse_result.status != AVA_PARSE_OK) {
            AvaDiagnostic diagnostic = {0};
            if (ava_parse_result_to_diagnostic(&source, parse_result, &diagnostic)) {
                (void)ava_diagnostic_render(stderr, &diagnostic);
            }
            ava_token_list_dispose(&tokens);
            ava_source_dispose(&source);
            return 2;
        }
    }

    bool written = show_ast ? ava_ast_dump(stdout, &source, &program)
                            : ava_token_dump(stdout, &source, &tokens);
    ava_ast_program_dispose(&program);
    ava_token_list_dispose(&tokens);
    ava_source_dispose(&source);

    if (!written) {
        fprintf(stderr, "ava: cannot write dump output\n");
        return 2;
    }
    return 0;
}

int ava_cli_run(int argc, char **argv) {
    if (argc == 1) {
        print_summary(stdout, argv[0]);
        return 0;
    }

    const char *option = argv[1];

    if (argc == 2) {
        if (strcmp(option, "-h") == 0 || strcmp(option, "--help") == 0) {
            print_help(stdout, argv[0]);
            return 0;
        }

        if (strcmp(option, "-V") == 0 || strcmp(option, "--version") == 0) {
            printf("AvaLang %s\n", AVA_VERSION);
            return 0;
        }
    }

    if (strcmp(option, "dump") == 0) {
        if (argc == 4 && (strcmp(argv[2], "tokens") == 0 || strcmp(argv[2], "ast") == 0)) {
            return dump_source(argv[3], strcmp(argv[2], "ast") == 0);
        }
        fprintf(stderr, "error: expected 'dump <tokens|ast> <file.ava>'\n\n");
        print_help(stderr, argv[0]);
        return 2;
    }

    fprintf(stderr, "error: unknown option '%s'\n\n", option);
    print_help(stderr, argv[0]);
    return 2;
}
