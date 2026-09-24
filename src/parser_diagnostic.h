#ifndef AVA_PARSER_DIAGNOSTIC_H
#define AVA_PARSER_DIAGNOSTIC_H

#include "diagnostic.h"
#include "parser.h"

/** Converts a parser failure into a borrowed diagnostic. */
bool ava_parse_result_to_diagnostic(const AvaSource *source, AvaParseResult result,
                                    AvaDiagnostic *diagnostic);

#endif
