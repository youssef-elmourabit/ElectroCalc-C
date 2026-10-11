#include "input.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>

InputStatus read_line(FILE *stream, char *buffer, size_t capacity)
{
    size_t length = 0;
    bool invalid = false;
    int character;

    if (stream == NULL || buffer == NULL || capacity < 2) {
        return INPUT_INVALID;
    }

    while ((character = fgetc(stream)) != '\n' && character != EOF) {
        if (character == '\0') {
            invalid = true;
        }
        if (length < capacity - 1) {
            buffer[length++] = (char)character;
        } else {
            invalid = true;
        }
    }
    buffer[length] = '\0';

    if (ferror(stream)) {
        return INPUT_END;
    }
    if (character == EOF && length == 0 && !invalid) {
        return INPUT_END;
    }
    return invalid ? INPUT_INVALID : INPUT_OK;
}

static bool ends_with_spaces(const char *text)
{
    while (isspace((unsigned char)*text)) {
        ++text;
    }
    return *text == '\0';
}

bool parse_number(const char *text, double *value)
{
    char *end;

    if (text == NULL || value == NULL) {
        return false;
    }

    errno = 0;
    double result = strtod(text, &end);
    if (end == text || errno == ERANGE || !isfinite(result) ||
        !ends_with_spaces(end)) {
        return false;
    }

    *value = result;
    return true;
}

bool parse_menu_choice(const char *text, int *choice)
{
    char *end;

    if (text == NULL || choice == NULL) {
        return false;
    }

    errno = 0;
    long result = strtol(text, &end, 10);
    if (end == text || errno == ERANGE || result < INT_MIN || result > INT_MAX ||
        !ends_with_spaces(end)) {
        return false;
    }

    *choice = (int)result;
    return true;
}
