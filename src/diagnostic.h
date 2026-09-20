#ifndef AVA_DIAGNOSTIC_H
#define AVA_DIAGNOSTIC_H

#include <stdbool.h>
#include <stdio.h>

/**
 * Describes the importance and effect of a diagnostic.
 */
typedef enum {
    AVA_DIAGNOSTIC_ERROR = 0,
    AVA_DIAGNOSTIC_WARNING,
    AVA_DIAGNOSTIC_NOTE
} AvaDiagnosticSeverity;

/**
 * Represents one compiler diagnostic.
 *
 * All strings are borrowed (owns no memory) and must remain valid until
 * the diagnostic has been rendered. Code and message are required.
 * Path, reason and help are optional and may be NULL.
 */
typedef struct {
    AvaDiagnosticSeverity severity;
    const char *code;
    const char *message;
    const char *path;
    const char *reason;
    const char *help;
} AvaDiagnostic;

/**
 * Writes one human-readable diagnostic to stream.
 *
 * Returns false when the arguments or required diagnostic fields are
 * invalid, the severity is unknown or writing fails.
 * Optional empty strings are not rendered.
 *
 * The function does not flush or close stream.
 */
bool ava_diagnostic_render(FILE *stream, const AvaDiagnostic *diagnostic);

#endif