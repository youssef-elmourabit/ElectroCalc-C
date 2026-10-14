#include "power.h"

#include <math.h>
#include <stddef.h>

bool electrical_power(double voltage, double current, double *power)
{
    if (power == NULL || !isfinite(voltage) || !isfinite(current)) {
        return false;
    }

    double result = voltage * current;
    if (!isfinite(result) ||
        (result == 0.0 && voltage != 0.0 && current != 0.0)) {
        return false;
    }

    *power = result;
    return true;
}
