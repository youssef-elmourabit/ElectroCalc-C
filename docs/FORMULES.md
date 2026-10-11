# Formules électriques et unités

## Implémentée au Jour 1 : loi d'Ohm

**U = R × I**

- U : tension aux bornes de la résistance, en volts (V).
- R : résistance, en ohms (Ω).
- I : courant traversant la résistance, en ampères (A).

Exemple : 20 mA = 0.02 A ; 100 Ω × 0.02 A = 2 V.
Le calcul suppose un composant ohmique. Cette version impose R > 0.
Un courant négatif indique le sens opposé à la référence choisie : U suit
la même convention de signe. Un courant nul donne une tension nulle.

Les relations I = U / R et R = U / I ne sont pas implémentées. Leur utilisation
future nécessitera de vérifier que le dénominateur n'est pas nul.

## Référence pour les jours futurs — aucun code correspondant au Jour 1

| Calcul | Formule | Conditions / unités |
| --- | --- | --- |
| Résistances en série | R_eq = R1 + R2 + … | Résistances en Ω |
| Résistances en parallèle | 1/R_eq = 1/R1 + 1/R2 + … | Résistances strictement positives en Ω |
| Diviseur de tension | U_sortie = U_entrée × R2 / (R1 + R2) | R2 côté masse, sortie à vide, somme non nulle |
| Puissance électrique | P = U × I | P en watts (W), U en V, I en A |

Pour une résistance avec convention récepteur : P = R × I² = U² / R,
avec R > 0 pour la forme comportant une division.

Conversions utiles : 1 kΩ = 1000 Ω ; 1 mA = 0.001 A ; 1 mV = 0.001 V.
Le programme attend les unités de base ; il ne convertit pas les préfixes.
