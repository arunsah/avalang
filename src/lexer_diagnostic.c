#include "lexer_diagnostic.h"

bool ava_lex_result_to_diagnostic(const AvaSource *source, AvaLexResult result,
                                  AvaDiagnostic *diagnostic) {
    if (diagnostic == NULL) {
        return false;
    }

    *diagnostic = (AvaDiagnostic){0};

    const char *code = NULL;
    const char *message = NULL;
    const char *help = NULL;
    bool has_location = false;

    switch (result.status) {
    case AVA_LEX_OK:
        return false;
    case AVA_LEX_INVALID_ARGUMENT:
        code = "AVA_INTERNAL_INVALID_LEX_ARGUMENT";
        message = "lexer received an invalid argument";
        help = "report this as an AvaLang compiler bug";
        break;
    case AVA_LEX_DESTINATION_NOT_EMPTY:
        code = "AVA_INTERNAL_LEX_DESTINATION_NOT_EMPTY";
        message = "lexer destination is not empty";
        help = "report this as an AvaLang compiler bug";
        break;
    case AVA_LEX_UNEXPECTED_CHARACTER:
        code = "AVA_LEX_UNEXPECTED_CHARACTER";
        message = "unexpected character";
        help = "this lexer currently accepts ASCII identifiers, decimal integers, ':', and '='";
        has_location = true;
        break;
    case AVA_LEX_TOO_MANY_TOKENS:
        code = "AVA_LEX_TOO_MANY_TOKENS";
        message = "source file exceeds the token limit";
        help = "split the source into smaller files";
        has_location = true;
        break;
    case AVA_LEX_OUT_OF_MEMORY:
        code = "AVA_LEX_OUT_OF_MEMORY";
        message = "cannot allocate memory for tokens";
        help = "free memory or reduce the source file size before trying again";
        has_location = true;
        break;
    default:
        code = "AVA_INTERNAL_UNKNOWN_LEX_STATUS";
        message = "lexer returned an unknown status";
        help = "report this as an AvaLang compiler bug";
        break;
    }

    size_t line = 0;
    size_t column = 0;
    const char *path = NULL;

    if (has_location && source != NULL &&
        ava_source_location(source, result.span.start, &line, &column)) {
        path = source->path;
    }

    *diagnostic = (AvaDiagnostic){
        .severity = AVA_DIAGNOSTIC_ERROR,
        .code = code,
        .message = message,
        .path = path,
        .reason = NULL,
        .help = help,
        .line = line,
        .column = column,
    };
    return true;
}
