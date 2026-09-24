#ifndef AVA_LEXER_DIAGNOSTIC_H
#define AVA_LEXER_DIAGNOSTIC_H

#include "diagnostic.h"
#include "lexer.h"

/** Converts a lexer failure into a borrowed diagnostic. */
bool ava_lex_result_to_diagnostic(const AvaSource *source, AvaLexResult result,
                                  AvaDiagnostic *diagnostic);

#endif
