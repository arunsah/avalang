#include "source_diagnostic.h"

#include <string.h>

static const char *system_error_reason(int system_error) {
    if (system_error == 0) {
        return NULL;
    }

    return strerror(system_error);
}

bool ava_source_load_result_to_diagnostic(const char *path, AvaSourceLoadResult result,
                                          AvaDiagnostic *diagnostic) {
    if (diagnostic == NULL) {
        return false;
    }

    *diagnostic = (AvaDiagnostic){0};

    const char *code = NULL;
    const char *message = NULL;
    const char *diagnostic_path = path;
    const char *reason = NULL;
    const char *help = NULL;

    switch (result.status) {
    case AVA_SOURCE_LOAD_OK:
        return false;

    case AVA_SOURCE_LOAD_INVALID_ARGUMENT:
        code = "AVA_INTERNAL_INVALID_SOURCE_LOAD_ARGUMENT";
        message = "source loader received an invalid argument";
        diagnostic_path = NULL;
        help = "report this as an AvaLang compiler bug";
        break;

    case AVA_SOURCE_LOAD_DESTINATION_NOT_EMPTY:
        code = "AVA_INTERNAL_SOURCE_DESTINATION_NOT_EMPTY";
        message = "source loader destination is not empty";
        diagnostic_path = NULL;
        help = "report this as an AvaLang compiler bug";
        break;

    case AVA_SOURCE_LOAD_OPEN_FAILED:
        code = "AVA_SOURCE_OPEN_FAILED";
        message = "cannot open source file";
        reason = system_error_reason(result.system_error);
        help = "check that the path exists and is readable";
        break;

    case AVA_SOURCE_LOAD_SEEK_FAILED:
        code = "AVA_SOURCE_SEEK_FAILED";
        message = "cannot seek in source file";
        reason = system_error_reason(result.system_error);
        help = "check that the path refers to a readable regular file";
        break;

    case AVA_SOURCE_LOAD_SIZE_FAILED:
        code = "AVA_SOURCE_SIZE_FAILED";
        message = "cannot determine source file size";
        reason = system_error_reason(result.system_error);
        help = "check that the path refers to a readable regular file";
        break;

    case AVA_SOURCE_LOAD_TOO_LARGE:
        code = "AVA_SOURCE_TOO_LARGE";
        message = "source file exceeds the 64 MiB limit";
        help = "reduce the source file to 64 MiB or less";
        break;

    case AVA_SOURCE_LOAD_OUT_OF_MEMORY:
        code = "AVA_SOURCE_OUT_OF_MEMORY";
        message = "cannot allocate memory for source file";
        reason = system_error_reason(result.system_error);
        help = "free memory or reduce the source file size before trying again";
        break;

    case AVA_SOURCE_LOAD_READ_FAILED:
        code = "AVA_SOURCE_READ_FAILED";
        message = "cannot read the complete source file";
        reason = system_error_reason(result.system_error);
        help = "check that the path refers to a readable regular file";
        break;

    default:
        code = "AVA_INTERNAL_UNKNOWN_SOURCE_LOAD_STATUS";
        message = "source loader returned an unknown status";
        diagnostic_path = NULL;
        help = "report this as an AvaLang compiler bug";
        break;
    }

    *diagnostic = (AvaDiagnostic){
        .severity = AVA_DIAGNOSTIC_ERROR,
        .code = code,
        .message = message,
        .path = diagnostic_path,
        .reason = reason,
        .help = help,
    };

    return true;
}