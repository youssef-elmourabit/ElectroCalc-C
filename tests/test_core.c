#include "divider.h"
#include "input.h"
#include "ohm.h"
#include "power.h"
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

static void test_divider(void)
{
    double voltage = 42.0;

    CHECK(divider_voltage(12.0, 1000.0, 2000.0, &voltage) && fabs(voltage - 8.0) < 1e-12);
    CHECK(divider_voltage(10.0, 1000.0, 1000.0, &voltage) && voltage == 5.0);
    CHECK(divider_voltage(0.0, 1000.0, 2000.0, &voltage) && voltage == 0.0);
    CHECK(divider_voltage(-12.0, 1000.0, 2000.0, &voltage) && fabs(voltage + 8.0) < 1e-12);
    CHECK(divider_voltage(12.0, 2000.0, 1000.0, &voltage) && fabs(voltage - 4.0) < 1e-12);
    CHECK(divider_voltage(12.0, 1.0, 2.0, &voltage) && fabs(voltage - 8.0) < 1e-12);
    CHECK(divider_voltage(-0.0, 1.0, 1.0, &voltage) && voltage == 0.0 && signbit(voltage));

    const double invalid_resistances[] = {0.0, -0.0, -1.0, NAN, INFINITY, -INFINITY};
    for (size_t index = 0; index < sizeof invalid_resistances / sizeof invalid_resistances[0]; ++index) {
        voltage = 42.0;
        CHECK(!divider_voltage(12.0, invalid_resistances[index], 1000.0, &voltage) && voltage == 42.0);
        CHECK(!divider_voltage(12.0, 1000.0, invalid_resistances[index], &voltage) && voltage == 42.0);
        /* Une entrée nulle ne doit pas court-circuiter la validation de R. */
        CHECK(!divider_voltage(0.0, invalid_resistances[index], 1000.0, &voltage) && voltage == 42.0);
        CHECK(!divider_voltage(0.0, 1000.0, invalid_resistances[index], &voltage) && voltage == 42.0);
    }
    const double nonfinite[] = {NAN, INFINITY, -INFINITY};
    for (size_t index = 0; index < sizeof nonfinite / sizeof nonfinite[0]; ++index) {
        CHECK(!divider_voltage(nonfinite[index], 1000.0, 2000.0, &voltage) && voltage == 42.0);
    }
    CHECK(!divider_voltage(12.0, 1000.0, 2000.0, NULL));

    /* Ni la somme R1+R2 ni le produit U*R2 n'ont besoin d'être représentables. */
    CHECK(divider_voltage(12.0, DBL_MAX, DBL_MAX, &voltage) && voltage == 6.0);
    CHECK(divider_voltage(DBL_MAX, DBL_MAX, DBL_MAX, &voltage) && voltage == DBL_MAX / 2.0);
    CHECK(divider_voltage(-DBL_MAX, DBL_MAX, DBL_MAX, &voltage) && voltage == -DBL_MAX / 2.0);
    CHECK(divider_voltage(DBL_MIN, DBL_MIN, DBL_MIN, &voltage) && voltage == DBL_MIN / 2.0);
    CHECK(divider_voltage(DBL_MAX, DBL_TRUE_MIN, DBL_MAX, &voltage) && voltage == DBL_MAX);
    /* Le rapport R2/R1 sous-déborde, mais U * R2/R1 est représentable. */
    CHECK(divider_voltage(DBL_MAX, DBL_MAX, DBL_TRUE_MIN, &voltage) && voltage == DBL_TRUE_MIN);
    CHECK(divider_voltage(-DBL_MAX, DBL_MAX, DBL_TRUE_MIN, &voltage) && voltage == -DBL_TRUE_MIN);
    CHECK(divider_voltage(1e308, 1e308, 1e-200, &voltage) && fabs(voltage / 1e-200 - 1.0) < 1e-12);
    CHECK(divider_voltage(1e308, 1e308, 4e-308, &voltage) && fabs(voltage / 4e-308 - 1.0) < 1e-12);
    CHECK(divider_voltage(1e-200, 1e-200, 1e308, &voltage) && voltage == 1e-200);
    CHECK(divider_voltage(1.0, DBL_TRUE_MIN, DBL_TRUE_MIN, &voltage) && voltage == 0.5);
    /* Juste au-dessus de la moitié de DBL_TRUE_MIN : arrondi non nul attendu.
     * Une somme intermédiaire en double masquerait la différence de R1/R2. */
    CHECK(divider_voltage(DBL_TRUE_MIN, nextafter(1.0, 0.0), 1.0, &voltage) && voltage == DBL_TRUE_MIN);
    CHECK(divider_voltage(-DBL_TRUE_MIN, nextafter(1.0, 0.0), 1.0, &voltage) && voltage == -DBL_TRUE_MIN);
    voltage = 42.0;
    CHECK(!divider_voltage(DBL_TRUE_MIN, 1.0, 1.0, &voltage) && voltage == 42.0);
    CHECK(!divider_voltage(-DBL_TRUE_MIN, 1.0, 1.0, &voltage) && voltage == 42.0);
    CHECK(!divider_voltage(DBL_MIN, DBL_MAX, DBL_MIN, &voltage) && voltage == 42.0);
    CHECK(!divider_voltage(DBL_TRUE_MIN, nextafter(1.0, 2.0), 1.0, &voltage) && voltage == 42.0);
    CHECK(!divider_voltage(-DBL_TRUE_MIN, nextafter(1.0, 2.0), 1.0, &voltage) && voltage == 42.0);
}

static void test_power(void)
{
    double power = 42.0;

    CHECK(electrical_power(12.0, 2.0, &power) && power == 24.0);
    CHECK(electrical_power(5.0, 0.5, &power) && power == 2.5);
    CHECK(electrical_power(0.0, 2.0, &power) && power == 0.0);
    CHECK(electrical_power(12.0, 0.0, &power) && power == 0.0);
    CHECK(electrical_power(0.0, 0.0, &power) && power == 0.0);
    CHECK(electrical_power(-12.0, 2.0, &power) && power == -24.0);
    CHECK(electrical_power(12.0, -2.0, &power) && power == -24.0);
    CHECK(electrical_power(-12.0, -2.0, &power) && power == 24.0);
    CHECK(electrical_power(-0.0, 2.0, &power) && power == 0.0 && signbit(power));
    CHECK(electrical_power(DBL_MAX, 1.0, &power) && power == DBL_MAX);
    CHECK(electrical_power(1.0, DBL_MAX, &power) && power == DBL_MAX);
    CHECK(electrical_power(DBL_MIN, 0.5, &power) && power == DBL_MIN / 2.0);
    CHECK(electrical_power(DBL_TRUE_MIN, 1.0, &power) && power == DBL_TRUE_MIN);
    CHECK(electrical_power(DBL_MAX, 0.0, &power) && power == 0.0);
    CHECK(electrical_power(1e-200, 1e200, &power) && fabs(power - 1.0) < 1e-12);
    power = 42.0;
    const double nonfinite[] = {NAN, INFINITY, -INFINITY};
    for (size_t index = 0; index < sizeof nonfinite / sizeof nonfinite[0]; ++index) {
        CHECK(!electrical_power(nonfinite[index], 2.0, &power) && power == 42.0);
        CHECK(!electrical_power(12.0, nonfinite[index], &power) && power == 42.0);
        CHECK(!electrical_power(nonfinite[index], 0.0, &power) && power == 42.0);
        CHECK(!electrical_power(0.0, nonfinite[index], &power) && power == 42.0);
    }
    CHECK(!electrical_power(12.0, 2.0, NULL));
    CHECK(!electrical_power(DBL_MAX, 2.0, &power) && power == 42.0);
    CHECK(!electrical_power(2.0, DBL_MAX, &power) && power == 42.0);
    CHECK(!electrical_power(-DBL_MAX, 2.0, &power) && power == 42.0);
    CHECK(!electrical_power(DBL_MAX, -2.0, &power) && power == 42.0);
    CHECK(!electrical_power(-DBL_MAX, -2.0, &power) && power == 42.0);
    CHECK(!electrical_power(DBL_TRUE_MIN, 0.5, &power) && power == 42.0);
    CHECK(!electrical_power(0.5, DBL_TRUE_MIN, &power) && power == 42.0);
    CHECK(!electrical_power(-DBL_TRUE_MIN, 0.5, &power) && power == 42.0);
    CHECK(!electrical_power(1e-200, 1e-200, &power) && power == 42.0);
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
    test_divider();
    test_power();
    test_parsers();
    test_lines();
    printf("Tests C : %d/%d vérifications réussies.\n", checks - failures, checks);
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
