#include "source_diagnostic.h"

#include <errno.h>
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

typedef struct {
    AvaSourceLoadStatus status;
    int system_error;
    const char *code;
    const char *message;
    const char *path;
    const char *help;
    bool has_system_reason;
} MappingCase;

static bool text_equals(const char *actual, const char *expected) {
    if (actual == NULL || expected == NULL) {
        return actual == expected;
    }

    return strcmp(actual, expected) == 0;
}

static void check_mapping(const MappingCase *test_case) {
    AvaSourceLoadResult result = {
        .status = test_case->status,
        .system_error = test_case->system_error,
    };

    AvaDiagnostic diagnostic = {0};

    CHECK(ava_source_load_result_to_diagnostic("example.ava", result, &diagnostic));
    CHECK(diagnostic.severity == AVA_DIAGNOSTIC_ERROR);
    CHECK(text_equals(diagnostic.code, test_case->code));
    CHECK(text_equals(diagnostic.message, test_case->message));
    CHECK(text_equals(diagnostic.path, test_case->path));
    CHECK(text_equals(diagnostic.help, test_case->help));

    if (test_case->has_system_reason) {
        CHECK(diagnostic.reason != NULL);

        if (diagnostic.reason != NULL) {
            CHECK(strcmp(diagnostic.reason, strerror(test_case->system_error)) == 0);
        }
    } else {
        CHECK(diagnostic.reason == NULL);
    }
}

static void test_status_mappings(void) {
    static const MappingCase cases[] = {
        {
            .status = AVA_SOURCE_LOAD_INVALID_ARGUMENT,
            .code = "AVA_INTERNAL_INVALID_SOURCE_LOAD_ARGUMENT",
            .message = "source loader received an invalid argument",
            .help = "report this as an AvaLang compiler bug",
        },
        {
            .status = AVA_SOURCE_LOAD_DESTINATION_NOT_EMPTY,
            .code = "AVA_INTERNAL_SOURCE_DESTINATION_NOT_EMPTY",
            .message = "source loader destination is not empty",
            .help = "report this as an AvaLang compiler bug",
        },
        {
            .status = AVA_SOURCE_LOAD_OPEN_FAILED,
            .system_error = EACCES,
            .code = "AVA_SOURCE_OPEN_FAILED",
            .message = "cannot open source file",
            .path = "example.ava",
            .help = "check that the path exists and is readable",
            .has_system_reason = true,
        },
        {
            .status = AVA_SOURCE_LOAD_SEEK_FAILED,
            .system_error = EIO,
            .code = "AVA_SOURCE_SEEK_FAILED",
            .message = "cannot seek in source file",
            .path = "example.ava",
            .help = "check that the path refers to a readable regular file",
            .has_system_reason = true,
        },
        {
            .status = AVA_SOURCE_LOAD_SIZE_FAILED,
            .system_error = EIO,
            .code = "AVA_SOURCE_SIZE_FAILED",
            .message = "cannot determine source file size",
            .path = "example.ava",
            .help = "check that the path refers to a readable regular file",
            .has_system_reason = true,
        },
        {
            .status = AVA_SOURCE_LOAD_TOO_LARGE,
            .system_error = EIO,
            .code = "AVA_SOURCE_TOO_LARGE",
            .message = "source file exceeds the 64 MiB limit",
            .path = "example.ava",
            .help = "reduce the source file to 64 MiB or less",
        },
        {
            .status = AVA_SOURCE_LOAD_OUT_OF_MEMORY,
            .system_error = ENOMEM,
            .code = "AVA_SOURCE_OUT_OF_MEMORY",
            .message = "cannot allocate memory for source file",
            .path = "example.ava",
            .help = "free memory or reduce the source file size before trying again",
            .has_system_reason = true,
        },
        {
            .status = AVA_SOURCE_LOAD_READ_FAILED,
            .system_error = EIO,
            .code = "AVA_SOURCE_READ_FAILED",
            .message = "cannot read the complete source file",
            .path = "example.ava",
            .help = "check that the path refers to a readable regular file",
            .has_system_reason = true,
        },
        {
            .status = (AvaSourceLoadStatus)999,
            .code = "AVA_INTERNAL_UNKNOWN_SOURCE_LOAD_STATUS",
            .message = "source loader returned an unknown status",
            .help = "report this as an AvaLang compiler bug",
        },
    };

    const size_t case_count = sizeof(cases) / sizeof(cases[0]);

    for (size_t index = 0; index < case_count; index += 1) {
        check_mapping(&cases[index]);
    }
}

static void test_success_resets_output(void) {
    AvaSourceLoadResult result = {
        .status = AVA_SOURCE_LOAD_OK,
    };

    AvaDiagnostic diagnostic = {
        .severity = AVA_DIAGNOSTIC_WARNING,
        .code = "OLD_CODE",
        .message = "old message",
        .path = "old.ava",
        .reason = "old reason",
        .help = "old help",
    };

    CHECK(!ava_source_load_result_to_diagnostic("example.ava", result, &diagnostic));
    CHECK(diagnostic.code == NULL);
    CHECK(diagnostic.message == NULL);
    CHECK(diagnostic.path == NULL);
    CHECK(diagnostic.reason == NULL);
    CHECK(diagnostic.help == NULL);
}

static void test_null_output(void) {
    AvaSourceLoadResult result = {
        .status = AVA_SOURCE_LOAD_OPEN_FAILED,
        .system_error = EACCES,
    };

    CHECK(!ava_source_load_result_to_diagnostic("example.ava", result, NULL));
}

static void test_real_missing_file(const char *missing_path) {
    AvaSource source = {0};
    AvaSourceLoadResult result = ava_source_load(missing_path, &source);

    CHECK(result.status == AVA_SOURCE_LOAD_OPEN_FAILED);

    AvaDiagnostic diagnostic = {0};
    CHECK(ava_source_load_result_to_diagnostic(missing_path, result, &diagnostic));
    CHECK(text_equals(diagnostic.code, "AVA_SOURCE_OPEN_FAILED"));
    CHECK(text_equals(diagnostic.message, "cannot open source file"));
    CHECK(diagnostic.path == missing_path);
    CHECK(text_equals(diagnostic.help, "check that the path exists and is readable"));

    if (result.system_error != 0) {
        CHECK(diagnostic.reason != NULL);
    }

    CHECK(source.path == NULL);
    CHECK(source.bytes == NULL);
    CHECK(source.length == 0);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <missing-source>\n", argv[0]);
        return 2;
    }

    test_status_mappings();
    test_success_resets_output();
    test_null_output();
    test_real_missing_file(argv[1]);

    if (failure_count != 0) {
        fprintf(stderr, "%d source diagnostic test(s) failed\n", failure_count);
        return 1;
    }

    printf("all source diagnostic tests passed\n");
    return 0;
}
