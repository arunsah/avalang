#include "cli.h"

#include <stdio.h>
#include <string.h>

#ifndef AVA_VERSION
#define AVA_VERSION "development"
#endif

static void print_summary(FILE *stream, const char *program) {
    fprintf(stream, "AvaLang bootstrap compiler %s\n", AVA_VERSION);
    fprintf(stream, "Compile and run AvaLang programs.\n\n");
    fprintf(stream, "Usage: %s [option]\n", program);
    fprintf(stream, "Try '%s --help' for more information.\n", program);
}

static void print_help(FILE *stream, const char *program) {
    fprintf(stream, "AvaLang bootstrap compiler\n\n");
    fprintf(stream, "Usage: %s [option]\n\n", program);
    fprintf(stream, "Options:\n");
    fprintf(stream, "   -h, --help      Show this help\n");
    fprintf(stream, "   -V, --version   Show version information\n");
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

    fprintf(stderr, "error: unknown option '%s'\n\n", option);
    print_help(stderr, argv[0]);
    return 2;
}
