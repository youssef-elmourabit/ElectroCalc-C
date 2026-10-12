#include "resistance.h"

#include <float.h>
#include <math.h>
#include <stddef.h>

static bool valid_resistances(double resistance1, double resistance2,
                              const double *equivalent)
{
    return equivalent != NULL && isfinite(resistance1) && resistance1 > 0.0 &&
           isfinite(resistance2) && resistance2 > 0.0;
}

bool resistance_series(double resistance1, double resistance2, double *equivalent)
{
    if (!valid_resistances(resistance1, resistance2, equivalent)) {
        return false;
    }

    double smaller = fmin(resistance1, resistance2);
    double larger = fmax(resistance1, resistance2);
    /* Ordonner les termes rend la garde symétrique, même près de DBL_MAX. */
    if (smaller > DBL_MAX - larger) {
        return false;
    }
    double result = resistance1 + resistance2;
    if (!isfinite(result)) {
        return false;
    }

    *equivalent = result;
    return true;
}

bool resistance_parallel(double resistance1, double resistance2, double *equivalent)
{
    if (!valid_resistances(resistance1, resistance2, equivalent)) {
        return false;
    }

    double smaller = fmin(resistance1, resistance2);
    double larger = fmax(resistance1, resistance2);
    /* Forme de (R1 * R2) / (R1 + R2) évitant le produit et la somme intermédiaires. */
    double result = smaller / (1.0 + smaller / larger);
    if (!isfinite(result) || result <= 0.0) {
        return false;
    }

    *equivalent = result;
    return true;
}
