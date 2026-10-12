#ifndef RESISTANCE_H
#define RESISTANCE_H

#include <stdbool.h>

/* Deux résistances finies > 0, en ohms. La sortie reste inchangée en cas d'erreur. */
bool resistance_series(double resistance1, double resistance2, double *equivalent);
bool resistance_parallel(double resistance1, double resistance2, double *equivalent);

#endif
