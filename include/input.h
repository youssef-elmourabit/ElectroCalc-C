#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

typedef enum {
    INPUT_OK,
    INPUT_INVALID,
    INPUT_END
} InputStatus;

/* Une ligne trop longue est consommée entièrement, puis refusée. */
InputStatus read_line(FILE *stream, char *buffer, size_t capacity);
bool parse_number(const char *text, double *value);
bool parse_menu_choice(const char *text, int *choice);

#endif
