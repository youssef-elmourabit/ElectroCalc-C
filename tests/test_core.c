#include "input.h"
#include "ohm.h"
#include "resistance.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks = 0;
static int failures = 0;

#define CHECK(condition) do { \
    ++checks; \
    if (!(condition)) { \
        fprintf(stderr, "ÉCHEC ligne %d : %s\n", __LINE__, #condition); \
        ++failures; \
    } \
} while (0)

static void test_voltage(void)
{
    double voltage = 42.0;

    CHECK(ohm_voltage(100.0, 0.02, &voltage) && fabs(voltage - 2.0) < 1e-12);
    CHECK(ohm_voltage(220.0, 0.05, &voltage) && fabs(voltage - 11.0) < 1e-12);
    CHECK(ohm_voltage(100.0, 0.0, &voltage) && voltage == 0.0);
    CHECK(ohm_voltage(100.0, -0.02, &voltage) && fabs(voltage + 2.0) < 1e-12);
    voltage = 42.0;
    CHECK(!ohm_voltage(-100.0, 0.02, &voltage) && voltage == 42.0);
    CHECK(!ohm_voltage(0.0, 0.02, &voltage));
    CHECK(!ohm_voltage(NAN, 0.02, &voltage));
    CHECK(!ohm_voltage(INFINITY, 0.02, &voltage));
    CHECK(!ohm_voltage(100.0, NAN, &voltage));
    CHECK(!ohm_voltage(100.0, INFINITY, &voltage));
    CHECK(!ohm_voltage(DBL_MAX, 2.0, &voltage));
    CHECK(!ohm_voltage(100.0, 0.02, NULL));
}

static void test_resistances(void)
{
    double equivalent = 42.0;

    CHECK(resistance_series(100.0, 220.0, &equivalent) && equivalent == 320.0);
    CHECK(resistance_series(220.0, 100.0, &equivalent) && equivalent == 320.0);
    CHECK(resistance_series(100.0, 100.0, &equivalent) && equivalent == 200.0);
    CHECK(resistance_series(0.1, 0.2, &equivalent) && fabs(equivalent - 0.3) < 1e-15);
    CHECK(resistance_parallel(100.0, 220.0, &equivalent) && fabs(equivalent - 68.75) < 1e-12);
    CHECK(resistance_parallel(220.0, 100.0, &equivalent) && fabs(equivalent - 68.75) < 1e-12);
    CHECK(resistance_parallel(100.0, 100.0, &equivalent) && equivalent == 50.0);
    CHECK(resistance_parallel(0.1, 0.2, &equivalent) && fabs(equivalent - 1.0 / 15.0) < 1e-15);

    /* Les limites s'appliquent aux deux arguments, dans les deux montages. */
    const double invalid[] = {0.0, -0.0, -100.0, NAN, INFINITY, -INFINITY};
    for (size_t index = 0; index < sizeof invalid / sizeof invalid[0]; ++index) {
        equivalent = 42.0;
        CHECK(!resistance_series(invalid[index], 100.0, &equivalent) && equivalent == 42.0);
        CHECK(!resistance_series(100.0, invalid[index], &equivalent) && equivalent == 42.0);
        CHECK(!resistance_parallel(invalid[index], 100.0, &equivalent) && equivalent == 42.0);
        CHECK(!resistance_parallel(100.0, invalid[index], &equivalent) && equivalent == 42.0);
    }
    CHECK(!resistance_series(100.0, 220.0, NULL));
    CHECK(!resistance_parallel(100.0, 220.0, NULL));

    /* Une somme trop grande est refusée ; la sortie conserve sa valeur. */
    equivalent = 42.0;
    CHECK(!resistance_series(DBL_MAX, DBL_MAX, &equivalent) && equivalent == 42.0);
    CHECK(!resistance_series(1e308, 1e308, &equivalent) && equivalent == 42.0);
    CHECK(!resistance_series(DBL_MAX, 1.0, &equivalent) && equivalent == 42.0);
    CHECK(!resistance_series(1.0, DBL_MAX, &equivalent) && equivalent == 42.0);
    CHECK(resistance_series(DBL_MAX / 2.0, DBL_MAX / 2.0, &equivalent) && equivalent == DBL_MAX);
    CHECK(resistance_series(DBL_MIN, DBL_MIN, &equivalent) && equivalent == 2.0 * DBL_MIN);
    CHECK(resistance_series(DBL_TRUE_MIN, DBL_TRUE_MIN, &equivalent) && equivalent == 2.0 * DBL_TRUE_MIN);

    /* La forme stable traite les cas où le produit, la somme ou les inverses déborderaient. */
    CHECK(resistance_parallel(DBL_MAX, DBL_MAX, &equivalent) && equivalent == DBL_MAX / 2.0);
    CHECK(resistance_parallel(1e308, 1e308, &equivalent) && equivalent == 5e307);
    CHECK(resistance_parallel(1e-200, 1e-200, &equivalent) && equivalent == 5e-201);
    CHECK(resistance_parallel(1e-200, 1e200, &equivalent) && equivalent == 1e-200);
    CHECK(resistance_parallel(1e200, 1e-200, &equivalent) && equivalent == 1e-200);
    CHECK(resistance_parallel(DBL_MIN, DBL_MIN, &equivalent) && equivalent == DBL_MIN / 2.0);
    CHECK(resistance_parallel(DBL_TRUE_MIN, DBL_MAX, &equivalent) && equivalent == DBL_TRUE_MIN);
    equivalent = 42.0;
    CHECK(!resistance_parallel(DBL_TRUE_MIN, DBL_TRUE_MIN, &equivalent) && equivalent == 42.0);
}

static void test_parsers(void)
{
    double value;
    int choice;

    CHECK(parse_number(" 0.02 \t", &value) && value == 0.02);
    CHECK(parse_number("2e-2", &value) && value == 0.02);
    CHECK(parse_number("-0.02", &value) && value == -0.02);
    CHECK(!parse_number("abc", &value));
    CHECK(!parse_number("", &value));
    CHECK(!parse_number(" \t", &value));
    CHECK(!parse_number("100 ohms", &value));
    CHECK(!parse_number("0,02", &value));
    CHECK(!parse_number("nan", &value));
    CHECK(!parse_number("inf", &value));
    CHECK(!parse_number("1e9999", &value));
    CHECK(!parse_number("1e-9999", &value));
    CHECK(!parse_number(NULL, &value));
    CHECK(!parse_number("1", NULL));
    CHECK(parse_menu_choice(" 1 \t", &choice) && choice == 1);
    CHECK(parse_menu_choice("0", &choice) && choice == 0);
    CHECK(!parse_menu_choice("1.0", &choice));
    CHECK(!parse_menu_choice("1x", &choice));
    CHECK(!parse_menu_choice("", &choice));
    CHECK(!parse_menu_choice("999999999999999999999999", &choice));
    CHECK(!parse_menu_choice("2147483648", &choice));
    CHECK(!parse_menu_choice(NULL, &choice));
    CHECK(!parse_menu_choice("1", NULL));
}

static void test_lines(void)
{
    char buffer[8];
    FILE *stream = tmpfile();
    CHECK(stream != NULL);
    if (stream == NULL) {
        return;
    }

    fputs("12345678\nOK\n\n1234567\nlast", stream);
    rewind(stream);
    CHECK(read_line(stream, buffer, sizeof buffer) == INPUT_INVALID);
    CHECK(read_line(stream, buffer, sizeof buffer) == INPUT_OK && strcmp(buffer, "OK") == 0);
    CHECK(read_line(stream, buffer, sizeof buffer) == INPUT_OK && buffer[0] == '\0');
    CHECK(read_line(stream, buffer, sizeof buffer) == INPUT_OK && strcmp(buffer, "1234567") == 0);
    CHECK(read_line(stream, buffer, sizeof buffer) == INPUT_OK && strcmp(buffer, "last") == 0);
    CHECK(read_line(stream, buffer, sizeof buffer) == INPUT_END);
    CHECK(fclose(stream) == 0);
    CHECK(read_line(NULL, buffer, sizeof buffer) == INPUT_INVALID);
}

int main(void)
{
    test_voltage();
    test_resistances();
    test_parsers();
    test_lines();
    printf("Tests C : %d/%d vérifications réussies.\n", checks - failures, checks);
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
