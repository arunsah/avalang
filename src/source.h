#ifndef AVA_SOURCE_H
#define AVA_SOURCE_H

#include <stdbool.h>
#include <stddef.h>

#define AVA_SOURCE_MAX_BYTES ((size_t)64 * 1024U * 1024U) // 64 MiB

/**
 * Represents a half-open byte range [start, end) in a source file.
 */
typedef struct {
    size_t start;
    size_t end;
} AvaSpan;

/**
 * Represents an empty or loaded source file.
 * A loaded source owns path and bytes.
 * The byte at bytes[length] is a null terminator provided for convenience
 * and is not part of the source contents.
 */
typedef struct {
    char *path;
    char *bytes;
    size_t length;
} AvaSource;

typedef enum {
    AVA_SOURCE_LOAD_OK = 0,
    AVA_SOURCE_LOAD_INVALID_ARGUMENT,
    AVA_SOURCE_LOAD_DESTINATION_NOT_EMPTY,
    AVA_SOURCE_LOAD_OPEN_FAILED,
    AVA_SOURCE_LOAD_SEEK_FAILED,
    AVA_SOURCE_LOAD_SIZE_FAILED,
    AVA_SOURCE_LOAD_TOO_LARGE,
    AVA_SOURCE_LOAD_OUT_OF_MEMORY,
    AVA_SOURCE_LOAD_READ_FAILED
} AvaSourceLoadStatus;

typedef struct {
    AvaSourceLoadStatus status;
    // Saved errno value when the failed operation provides one.
    int system_error;
} AvaSourceLoadResult;

/**
 * Loads path into an empty source. On failure, source remains empty.
 * The caller owns a successfully loaded source and must dispose it.
 */
AvaSourceLoadResult ava_source_load(const char *path, AvaSource *source);

/**
 * Releases memory owned by source and resets it to the empty state.
 * Passing NULL or disposing an empty source is allowed.
 */
void ava_source_dispose(AvaSource *source);

/**
 * Converts a source byte offset to a one-based line and byte column.
 *
 * The offset may equal source->length, which locates EOF.
 * Returns false for invalid arguments or an offset beyond the source.
 */
bool ava_source_location(const AvaSource *source, size_t offset, size_t *line, size_t *column);

#endif
