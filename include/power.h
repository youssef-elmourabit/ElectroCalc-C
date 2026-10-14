#ifndef POWER_H
#define POWER_H

#include <stdbool.h>

/* P = U * I, valeurs finies en V et A ; résultat en W, convention récepteur.
 * Retourne false sans modifier la sortie si le résultat n'est pas fini ou
 * si un produit de deux valeurs non nulles s'arrondit à zéro. */
bool electrical_power(double voltage, double current, double *power);

#endif
