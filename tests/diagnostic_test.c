#include "diagnostic.h"

#include <stdio.h>
#include <string.h>

enum { OUTPUT_CAPACITY = 512 };

static int failure_count = 0;

#define CHECK(condition)                                                                           \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition);          \
            failure_count += 1;                                                                    \
        }                                                                                          \
    } while (0)

static void check_output(const AvaDiagnostic *diagnostic, const char *expected) {
    FILE *stream = tmpfile();
    CHECK(stream != NULL);

    if (stream == NULL) {
        return;
    }

    bool rendered = ava_diagnostic_render(stream, diagnostic);
    CHECK(rendered);

    if (!rendered) {
        (void)fclose(stream);
        return;
    }

    int flush_result = fflush(stream);
    CHECK(flush_result == 0);

    if (flush_result != 0) {
        (void)fclose(stream);
        return;
    }

    int seek_result = fseek(stream, 0, SEEK_SET);
    CHECK(seek_result == 0);

    if (seek_result != 0) {
        (void)fclose(stream);
        return;
    }

    char actual[OUTPUT_CAPACITY];
    size_t length = fread(actual, 1, sizeof(actual) - 1, stream);
    int trailing = fgetc(stream);

    CHECK(trailing == EOF);
    CHECK(ferror(stream) == 0);

    actual[length] = '\0';
    CHECK(strcmp(actual, expected) == 0);
    CHECK(fclose(stream) == 0);
}

static void check_rejected(const AvaDiagnostic *diagnostic) {
    FILE *stream = tmpfile();
    CHECK(stream != NULL);

    if (stream == NULL) {
        return;
    }

    CHECK(!ava_diagnostic_render(stream, diagnostic));
    CHECK(fflush(stream) == 0);
    CHECK(fseek(stream, 0, SEEK_SET) == 0);
    CHECK(fgetc(stream) == EOF);
    CHECK(ferror(stream) == 0);
    CHECK(fclose(stream) == 0);
}

static void test_error_with_all_fields(void) {
    AvaDiagnostic diagnostic = {
        .severity = AVA_DIAGNOSTIC_ERROR,
        .code = "AVA_SOURCE_OPEN_FAILED",
        .message = "cannot open source file",
        .path = "missing.ava",
        .reason = "No such file or directory",
        .help = "check that the path exists and is readable",
    };

    check_output(&diagnostic,
                 "missing.ava: error[AVA_SOURCE_OPEN_FAILED]: cannot open source file\n"
                 "reason: No such file or directory\n"
                 "help: check that the path exists and is readable\n");
}

static void test_warning_without_path_or_reason(void) {
    AvaDiagnostic diagnostic = {
        .severity = AVA_DIAGNOSTIC_WARNING,
        .code = "AVA_EXAMPLE_WARNING",
        .message = "example warning",
        .help = "example help",
    };

    check_output(&diagnostic, "warning[AVA_EXAMPLE_WARNING]: example warning\n"
                              "help: example help\n");
}

static void test_note(void) {
    AvaDiagnostic diagnostic = {
        .severity = AVA_DIAGNOSTIC_NOTE,
        .code = "AVA_EXAMPLE_NOTE",
        .message = "example note",
    };

    check_output(&diagnostic, "note[AVA_EXAMPLE_NOTE]: example note\n");
}

static void test_empty_optional_fields(void) {
    AvaDiagnostic diagnostic = {
        .severity = AVA_DIAGNOSTIC_ERROR,
        .code = "AVA_EXAMPLE_ERROR",
        .message = "example error",
        .path = "",
        .reason = "",
        .help = "",
    };

    check_output(&diagnostic, "error[AVA_EXAMPLE_ERROR]: example error\n");
}

static void test_invalid_diagnostics(void) {
    AvaDiagnostic diagnostic = {
        .severity = AVA_DIAGNOSTIC_ERROR,
        .code = "AVA_EXAMPLE_ERROR",
        .message = "example error",
    };

    CHECK(!ava_diagnostic_render(NULL, &diagnostic));
    check_rejected(NULL);

    diagnostic.code = NULL;
    check_rejected(&diagnostic);

    diagnostic.code = "";
    check_rejected(&diagnostic);

    diagnostic.code = "AVA_EXAMPLE_ERROR";
    diagnostic.message = NULL;
    check_rejected(&diagnostic);

    diagnostic.message = "";
    check_rejected(&diagnostic);

    diagnostic.message = "example error";
    diagnostic.severity = (AvaDiagnosticSeverity)99;
    check_rejected(&diagnostic);
}

int main(void) {
    test_error_with_all_fields();
    test_warning_without_path_or_reason();
    test_note();
    test_empty_optional_fields();
    test_invalid_diagnostics();

    if (failure_count != 0) {
        fprintf(stderr, "%d diagnostic test(s) failed\n", failure_count);
        return 1;
    }

    printf("all diagnostic tests passed\n");
    return 0;
}