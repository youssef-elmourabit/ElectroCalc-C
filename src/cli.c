#include "cli.h"
#include "input.h"
#include "ohm.h"
#include "resistance.h"

#include <stdbool.h>
#include <stdio.h>

static bool ask_number(const char *prompt, bool positive, double *value)
{
    char line[256];

    for (;;) {
        printf("%s", prompt);
        fflush(stdout);
        InputStatus status = read_line(stdin, line, sizeof line);
        if (status == INPUT_END) {
            return false;
        }
        if (status != INPUT_OK || !parse_number(line, value)) {
            puts("Erreur : saisissez un nombre fini valide (point décimal, ex. 0.02).");
        } else if (positive && *value <= 0.0) {
            puts("Erreur : la résistance doit être strictement positive (R > 0 Ω).");
        } else {
            return true;
        }
    }
}

static bool calculate_ohm(void)
{
    double resistance;
    double current;
    double voltage;

    puts("\nLoi d'Ohm : U = R × I");
    if (!ask_number("Résistance R (Ω) : ", true, &resistance) ||
        !ask_number("Courant I (A) : ", false, &current)) {
        return false;
    }

    if (!ohm_voltage(resistance, current, &voltage)) {
        puts("Erreur : résultat hors de la plage numérique représentable.");
    } else {
        printf("Résultat : U = %.10g V (R = %.10g Ω, I = %.10g A)\n",
               voltage, resistance, current);
    }
    puts("Retour au menu.");
    return true;
}

static bool calculate_resistance(bool parallel)
{
    double resistance1;
    double resistance2;
    double equivalent;

    if (parallel) {
        puts("\nDeux résistances en parallèle : R_eq = (R1 × R2) / (R1 + R2)");
    } else {
        puts("\nDeux résistances en série : R_eq = R1 + R2");
    }
    if (!ask_number("Résistance R1 (Ω) : ", true, &resistance1) ||
        !ask_number("Résistance R2 (Ω) : ", true, &resistance2)) {
        return false;
    }

    bool success = parallel
        ? resistance_parallel(resistance1, resistance2, &equivalent)
        : resistance_series(resistance1, resistance2, &equivalent);
    if (!success) {
        puts("Erreur : résistance équivalente hors de la plage numérique représentable.");
    } else {
        printf("Résultat : R_eq = %.10g Ω (R1 = %.10g Ω, R2 = %.10g Ω)\n",
               equivalent, resistance1, resistance2);
    }
    puts("Retour au menu.");
    return true;
}

int run_menu(void)
{
    char line[256];
    int choice;

    for (;;) {
        puts("\n=== ElectroCalc ===");
        puts("1. Loi d'Ohm : U = R × I");
        puts("2. Deux résistances en série");
        puts("3. Deux résistances en parallèle");
        puts("4. Diviseur de tension — À développer");
        puts("5. Puissance électrique — À développer");
        puts("0. Quitter");
        printf("Votre choix : ");
        fflush(stdout);

        InputStatus status = read_line(stdin, line, sizeof line);
        if (status == INPUT_END) {
            puts("\nFin de saisie. Au revoir !");
            return 0;
        }
        if (status != INPUT_OK || !parse_menu_choice(line, &choice) ||
            choice < 0 || choice > 5) {
            puts("Erreur : choix de menu invalide. Entrez un entier de 0 à 5.");
            continue;
        }

        switch (choice) {
        case 0:
            puts("Au revoir !");
            return 0;
        case 1:
            if (!calculate_ohm()) {
                puts("\nFin de saisie. Au revoir !");
                return 0;
            }
            break;
        case 2:
        case 3:
            if (!calculate_resistance(choice == 3)) {
                puts("\nFin de saisie. Au revoir !");
                return 0;
            }
            break;
        default:
            puts("À développer : fonctionnalité prévue pour une prochaine version.");
            break;
        }
    }
}
