#include "parser_diagnostic.h"

bool ava_parse_result_to_diagnostic(const AvaSource *source, AvaParseResult result,
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
    case AVA_PARSE_OK:
        return false;
    case AVA_PARSE_INVALID_ARGUMENT:
        code = "AVA_INTERNAL_INVALID_PARSE_ARGUMENT";
        message = "parser received an invalid argument";
        help = "report this as an AvaLang compiler bug";
        break;
    case AVA_PARSE_DESTINATION_NOT_EMPTY:
        code = "AVA_INTERNAL_PARSE_DESTINATION_NOT_EMPTY";
        message = "parser destination is not empty";
        help = "report this as an AvaLang compiler bug";
        break;
    case AVA_PARSE_EXPECTED_LET:
        code = "AVA_PARSE_EXPECTED_LET";
        message = "expected 'let' declaration";
        help = "this parser currently accepts only 'let' bindings";
        has_location = true;
        break;
    case AVA_PARSE_EXPECTED_NAME:
        code = "AVA_PARSE_EXPECTED_NAME";
        message = "expected a binding name after 'let'";
        has_location = true;
        break;
    case AVA_PARSE_EXPECTED_TYPE:
        code = "AVA_PARSE_EXPECTED_TYPE";
        message = "expected a type name after ':'";
        has_location = true;
        break;
    case AVA_PARSE_EXPECTED_EQUAL:
        code = "AVA_PARSE_EXPECTED_EQUAL";
        message = "expected '=' before the initializer";
        has_location = true;
        break;
    case AVA_PARSE_EXPECTED_INTEGER:
        code = "AVA_PARSE_EXPECTED_INTEGER";
        message = "expected an integer literal initializer";
        help = "this parser currently accepts only decimal integer literals as initializers";
        has_location = true;
        break;
    case AVA_PARSE_OUT_OF_MEMORY:
        code = "AVA_PARSE_OUT_OF_MEMORY";
        message = "cannot allocate memory for syntax tree";
        help = "free memory or reduce the source file size before trying again";
        has_location = true;
        break;
    default:
        code = "AVA_INTERNAL_UNKNOWN_PARSE_STATUS";
        message = "parser returned an unknown status";
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
        .line = line,
        .column = column,
        .help = help,
    };
    return true;
}
