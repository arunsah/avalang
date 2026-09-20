#include "source.h"

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

static void test_invalid_arguments(const char *basic_path) {
    AvaSource source = {0};

    AvaSourceLoadResult result = ava_source_load(NULL, &source);
    CHECK(result.status == AVA_SOURCE_LOAD_INVALID_ARGUMENT);

    result = ava_source_load(basic_path, NULL);
    CHECK(result.status == AVA_SOURCE_LOAD_INVALID_ARGUMENT);
}

static void test_empty_source(const char *empty_path) {
    AvaSource source = {0};

    AvaSourceLoadResult result = ava_source_load(empty_path, &source);
    CHECK(result.status == AVA_SOURCE_LOAD_OK);

    if (result.status != AVA_SOURCE_LOAD_OK) {
        return;
    }

    CHECK(source.path != NULL);

    // If the loader incorrectly returns success with a null path or buffer,
    // strcmp and source.bytes[0] would crash the test.
    if (source.path != NULL) {
        CHECK(source.path != empty_path);
        CHECK(strcmp(source.path, empty_path) == 0);
    }

    CHECK(source.bytes != NULL);
    CHECK(source.length == 0);

    if (source.bytes != NULL) {
        CHECK(source.bytes[0] == '\0');
    }

    ava_source_dispose(&source);

    // Disposal resets the path, bytes, and length.
    CHECK(source.path == NULL);
    CHECK(source.bytes == NULL);
    CHECK(source.length == 0);

    // Not needed for cleanup but intentionally verifies
    // that disposing an already empty source is safe.
    // Repeated disposal must remain safe.
    ava_source_dispose(&source);
}

static void test_basic_source(const char *basic_path) {
    static const char expected[] = "let answer = 42\n";
    const size_t expected_length = sizeof(expected) - 1;

    AvaSource source = {0};
    AvaSourceLoadResult result = ava_source_load(basic_path, &source);

    CHECK(result.status == AVA_SOURCE_LOAD_OK);

    if (result.status != AVA_SOURCE_LOAD_OK) {
        return;
    }

    CHECK(source.path != NULL);

    if (source.path != NULL) {
        CHECK(source.path != basic_path);
        CHECK(strcmp(source.path, basic_path) == 0);
    }

    CHECK(source.bytes != NULL);
    CHECK(source.length == expected_length);

    if (source.bytes != NULL && source.length == expected_length) {
        CHECK(memcmp(source.bytes, expected, expected_length) == 0);
        CHECK(source.bytes[source.length] == '\0');
    }

    ava_source_dispose(&source);
}

static void test_missing_source(const char *missing_path) {
    AvaSource source = {0};

    AvaSourceLoadResult result = ava_source_load(missing_path, &source);
    CHECK(result.status == AVA_SOURCE_LOAD_OPEN_FAILED);
    CHECK(source.path == NULL);
    CHECK(source.bytes == NULL);
    CHECK(source.length == 0);
}

static void test_loaded_destination(const char *basic_path) {
    AvaSource source = {0};
    AvaSourceLoadResult result = ava_source_load(basic_path, &source);

    CHECK(result.status == AVA_SOURCE_LOAD_OK);

    if (result.status != AVA_SOURCE_LOAD_OK) {
        return;
    }

    char *original_path = source.path;
    char *original_bytes = source.bytes;
    size_t original_length = source.length;

    result = ava_source_load(basic_path, &source);

    CHECK(result.status == AVA_SOURCE_LOAD_DESTINATION_NOT_EMPTY);
    CHECK(source.path == original_path);
    CHECK(source.bytes == original_bytes);
    CHECK(source.length == original_length);

    ava_source_dispose(&source);
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "usage: %s <empty-source> <basic-source> <missing-source>\n", argv[0]);
        return 2;
    }

    test_invalid_arguments(argv[2]);  // <basic-source>
    test_empty_source(argv[1]);       // <empty-source>
    test_basic_source(argv[2]);       // <basic-source>
    test_missing_source(argv[3]);     // <missing-source>
    test_loaded_destination(argv[2]); // <basic-source>

    if (failure_count != 0) {
        fprintf(stderr, "%d source test(s) failed\n", failure_count);
        return 1;
    }

    printf("all source tests passed\n");
    return 0;
}