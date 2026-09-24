#include "diagnostic.h"

static bool has_text(const char *text) {
    return text != NULL && text[0] != '\0';
}

static const char *severity_name(AvaDiagnosticSeverity severity) {
    switch (severity) {
    case AVA_DIAGNOSTIC_ERROR:
        return "error";
    case AVA_DIAGNOSTIC_WARNING:
        return "warning";
    case AVA_DIAGNOSTIC_NOTE:
        return "note";
    }

    return NULL;
}

bool ava_diagnostic_render(FILE *stream, const AvaDiagnostic *diagnostic) {
    if (stream == NULL || diagnostic == NULL) {
        return false;
    }

    const char *severity = severity_name(diagnostic->severity);

    // Validate required fields before writing any output.
    if (severity == NULL || !has_text(diagnostic->code) || !has_text(diagnostic->message)) {
        return false;
    }

    if ((diagnostic->line == 0) != (diagnostic->column == 0)) {
        return false;
    }

    if (has_text(diagnostic->path)) {
        if (fprintf(stream, "%s:", diagnostic->path) < 0) {
            return false;
        }
    }

    if (diagnostic->line != 0) {
        if (fprintf(stream, "%zu:%zu:", diagnostic->line, diagnostic->column) < 0) {
            return false;
        }
    }

    if (has_text(diagnostic->path) || diagnostic->line != 0) {
        if (fputc(' ', stream) == EOF) {
            return false;
        }
    }

    if (fprintf(stream, "%s[%s]: %s\n", severity, diagnostic->code, diagnostic->message) < 0) {
        return false;
    }

    if (has_text(diagnostic->reason)) {
        if (fprintf(stream, "reason: %s\n", diagnostic->reason) < 0) {
            return false;
        }
    }

    if (has_text(diagnostic->help)) {
        if (fprintf(stream, "help: %s\n", diagnostic->help) < 0) {
            return false;
        }
    }

    return ferror(stream) == 0;
}