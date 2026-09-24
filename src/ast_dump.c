#include "ast_dump.h"

static bool write_field(FILE *stream, const AvaSource *source, const char *name, AvaSpan span) {
    if (span.start > span.end || span.end > source->length ||
        fprintf(stream, "  %s [%zu,%zu): \"", name, span.start, span.end) < 0) {
        return false;
    }
    size_t length = span.end - span.start;
    return fwrite(source->bytes + span.start, 1, length, stream) == length &&
           fputs("\"\n", stream) >= 0;
}

bool ava_ast_dump(FILE *stream, const AvaSource *source, const AvaAstProgram *program) {
    if (stream == NULL || source == NULL || source->bytes == NULL || program == NULL ||
        (program->count != 0 && program->items == NULL)) {
        return false;
    }

    for (size_t index = 0; index < program->count; index += 1) {
        const AvaAstLet *binding = &program->items[index];
        if (binding->span.start > binding->span.end || binding->span.end > source->length ||
            fprintf(stream, "let [%zu,%zu)\n", binding->span.start, binding->span.end) < 0 ||
            !write_field(stream, source, "name", binding->name)) {
            return false;
        }
        if (binding->has_type && !write_field(stream, source, "type", binding->type)) {
            return false;
        }
        if (!write_field(stream, source, "integer", binding->value)) {
            return false;
        }
    }
    return ferror(stream) == 0;
}
