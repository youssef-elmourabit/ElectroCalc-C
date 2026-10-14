#include "divider.h"

#include <math.h>
#include <stddef.h>

bool divider_voltage(double input_voltage, double resistance1,
                     double resistance2, double *output_voltage)
{
    if (output_voltage == NULL || !isfinite(input_voltage) ||
        !isfinite(resistance1) || resistance1 <= 0.0 ||
        !isfinite(resistance2) || resistance2 <= 0.0) {
        return false;
    }
    if (input_voltage == 0.0) {
        *output_voltage = input_voltage;
        return true;
    }

    int exponent1;
    int exponent2;
    int input_exponent;
    double mantissa1 = frexp(resistance1, &exponent1);
    double mantissa2 = frexp(resistance2, &exponent2);
    double input_mantissa = frexp(input_voltage, &input_exponent);
    int scale = exponent1 > exponent2 ? exponent1 : exponent2;

    /* R1 + R2 = denominator * 2^scale, sans former la somme originale.
     * Le petit terme peut disparaître par arrondi, sans annuler le numérateur. */
    long double denominator = ldexpl((long double)mantissa1, exponent1 - scale) +
                              ldexpl((long double)mantissa2, exponent2 - scale);
    /* Garder l'exposant de R2 séparé évite de perdre un rapport minuscule
     * avant de le multiplier par une grande tension d'entrée. */
    /* Conserver la précision étendue jusqu'à l'arrondi final en double évite
     * un faux zéro à la frontière des résultats subnormaux. */
    double result = (double)ldexpl((long double)input_mantissa *
                                  ((long double)mantissa2 / denominator),
                                  input_exponent + (exponent2 - scale));
    if (!isfinite(result) || result == 0.0) {
        return false;
    }

    *output_voltage = result;
    return true;
}
