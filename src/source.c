#include "source.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AvaSourceLoadResult make_load_result(AvaSourceLoadStatus status, int system_error) {
    return (AvaSourceLoadResult){
        .status = status,
        .system_error = system_error,
    };
}

AvaSourceLoadResult ava_source_load(const char *path, AvaSource *source) {
    if (path == NULL || source == NULL) {
        return make_load_result(AVA_SOURCE_LOAD_INVALID_ARGUMENT, 0);
    }

    if (source->path != NULL || source->bytes != NULL || source->length != 0) {
        return make_load_result(AVA_SOURCE_LOAD_DESTINATION_NOT_EMPTY, 0);
    }

    errno = 0;
    FILE *file = fopen(path, "rb");

    if (file == NULL) {
        return make_load_result(AVA_SOURCE_LOAD_OPEN_FAILED, errno);
    }

    AvaSourceLoadStatus status = AVA_SOURCE_LOAD_OK;
    int system_error = 0;
    char *path_copy = NULL;
    char *bytes = NULL;

    errno = 0;

    if (fseek(file, 0, SEEK_END) != 0) {
        status = AVA_SOURCE_LOAD_SEEK_FAILED;
        system_error = errno;
        goto fail;
    }

    errno = 0;

    // Measure the file before allocating so the implementation limit can be
    // enforced without reading an arbitrarily large file into memory.
    long file_size = ftell(file);

    if (file_size < 0) {
        status = AVA_SOURCE_LOAD_SIZE_FAILED;
        system_error = errno;
        goto fail;
    }

    if (file_size > (long)AVA_SOURCE_MAX_BYTES) {
        status = AVA_SOURCE_LOAD_TOO_LARGE;
        goto fail;
    }

    errno = 0;

    if (fseek(file, 0, SEEK_SET) != 0) {
        status = AVA_SOURCE_LOAD_SEEK_FAILED;
        system_error = errno;
        goto fail;
    }

    size_t path_length = strlen(path);

    errno = 0;
    path_copy = malloc(path_length + 1);

    if (path_copy == NULL) {
        status = AVA_SOURCE_LOAD_OUT_OF_MEMORY;
        system_error = errno;
        goto fail;
    }

    memcpy(path_copy, path, path_length + 1);

    size_t length = (size_t)file_size;

    errno = 0;
    bytes = malloc(length + 1);

    if (bytes == NULL) {
        status = AVA_SOURCE_LOAD_OUT_OF_MEMORY;
        system_error = errno;
        goto fail;
    }

    errno = 0;
    size_t bytes_read = fread(bytes, 1, length, file);
    if (bytes_read != length) {
        status = AVA_SOURCE_LOAD_READ_FAILED;
        system_error = errno;
        goto fail;
    }

    // The terminator is available to later stages but is not included in length.
    bytes[length] = '\0';

    (void)fclose(file);

    // Transfer ownership only after the complete file has been read.
    source->path = path_copy;
    source->bytes = bytes;
    source->length = length;

    return make_load_result(AVA_SOURCE_LOAD_OK, 0);

fail:
    (void)fclose(file);
    free(path_copy);
    free(bytes);

    return make_load_result(status, system_error);
}

void ava_source_dispose(AvaSource *source) {
    if (source == NULL) {
        return;
    }

    free(source->path);
    free(source->bytes);

    *source = (AvaSource){0};
}

bool ava_source_location(const AvaSource *source, size_t offset, size_t *line, size_t *column) {
    if (source == NULL || source->bytes == NULL || line == NULL || column == NULL ||
        offset > source->length) {
        return false;
    }

    size_t current_line = 1;
    size_t current_column = 1;

    for (size_t index = 0; index < offset; index += 1) {
        if (source->bytes[index] == '\n') {
            current_line += 1;
            current_column = 1;
        } else {
            current_column += 1;
        }
    }

    *line = current_line;
    *column = current_column;
    return true;
}
