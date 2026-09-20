#ifndef AVA_SOURCE_DIAGNOSTIC_H
#define AVA_SOURCE_DIAGNOSTIC_H

#include "diagnostic.h"
#include "source.h"

/**
 * Converts a source-load failure into a diagnostic.
 *
 * The output diagnostic is reset before conversion.
 * Returns true when a diagnostic is produced.
 * Returns false when result represents a success or
 * diagnostic is NULL.
 *
 * The produced diagnostic borrows path and all other strings.
 * A reason obtained from the C library may be replaced by a
 * later system-error lookup, so the diagnostic should be
 * rendered immediately.
 */
bool ava_source_load_result_to_diagnostic(const char *path, AvaSourceLoadResult result,
                                          AvaDiagnostic *diagnostic);

#endif