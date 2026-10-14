#ifndef DIVIDER_H
#define DIVIDER_H

#include <stdbool.h>

/* Diviseur idéal non chargé : R1 et R2 finies > 0, entrée finie en volts.
 * Retourne false sans modifier la sortie si le résultat est invalide ou
 * si une tension non nulle est arrondie à zéro. */
bool divider_voltage(double input_voltage, double resistance1,
                     double resistance2, double *output_voltage);

#endif
