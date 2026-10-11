#include "ohm.h"

#include <math.h>
#include <stddef.h>

bool ohm_voltage(double resistance, double current, double *voltage)
{
    if (voltage == NULL || !isfinite(resistance) || resistance <= 0.0 ||
        !isfinite(current)) {
        return false;
    }

    double result = resistance * current;
    if (!isfinite(result)) {
        return false;
    }

    *voltage = result;
    return true;
}
