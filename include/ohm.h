#ifndef OHM_H
#define OHM_H

#include <stdbool.h>

/* R > 0, I signé, valeurs finies. Retourne false si le calcul est invalide. */
bool ohm_voltage(double resistance, double current, double *voltage);

#endif
