# Puissance électrique — conception et validation

## Formule et unités

**P = U × I**, avec U en volts, I en ampères et P en watts.

- 12 V × 2 A = 24 W.
- 5 V × 0.5 A = 2.5 W.
- Une tension ou un courant nul donne une puissance nulle.

Pour un régime continu, cette relation donne la puissance du dipôle. Pour
des valeurs instantanées simultanées, elle donne la puissance instantanée.
Le programme ne calcule pas de puissance moyenne ou réactive à partir
de valeurs efficaces et d'un déphasage.

## Convention de signe

Le programme utilise la **convention récepteur** : `U = V(+) - V(-)`,
et le courant de référence I entre par la borne de référence positive (+).
Une valeur négative indique le sens opposé à cette référence.

| Signe de U | Signe de I | Signe de P | Échange d'énergie du dipôle |
| --- | --- | --- | --- |
| + | + | + | Absorption |
| - | - | + | Absorption |
| + | - | - | Fourniture |
| - | + | - | Fourniture |

Le signe d'une seule grandeur ne permet pas de conclure. Une puissance
négative n'est pas une saisie invalide ; elle décrit une fourniture d'énergie
avec ces références. Le programme ne vérifie pas le câblage réel. Pour une
résistance ohmique passive, U = R × I avec R > 0 impose P = R × I² >= 0.

## Architecture et contrat

`include/power.h` déclare `electrical_power(U, I, &P)` ; `src/power.c` ne
dépend pas de la console. L'option 5 de `src/cli.c` rappelle la convention,
réutilise la lecture et la conversion communes, puis affiche P en watts.
Le Makefile compile ce module dans l'application et les tests unitaires.

Les deux valeurs doivent être finies, sans contrainte de positivité. Un
pointeur de sortie nul est refusé. La fonction retourne `false` sans écrire
la sortie si le calcul est invalide ; sinon elle écrit P et retourne `true`.

## Limites numériques

Le produit est effectué en `double` puis contrôlé par `isfinite()`. Un
débordement produisant l'infini est refusé dans les deux ordres et pour tous
les signes. `DBL_MAX × 1` reste accepté.

Un produit de deux nombres non nuls arrondi à zéro est refusé, afin de ne pas
présenter une perte numérique comme une puissance physique nulle. Un produit
subnormal non nul est accepté, avec la précision réduite de cette plage.
Par exemple, `DBL_MIN × 0.5` est accepté, mais `DBL_TRUE_MIN × 0.5` est refusé
sur l'environnement vérifié avec l'arrondi usuel au plus proche.

Les produits sont arrondis selon le type `double` ; aucune arithmétique exacte
ni borne certifiée d'erreur n'est fournie. Les tests visent l'environnement
macOS documenté dans le README. Les conversions console signalant `ERANGE`
restent refusées, même si la fonction C pourrait traiter directement certaines
valeurs subnormales. La console affiche 10 chiffres significatifs.

## Validation

Les tests C couvrent les exemples de référence, les quatre combinaisons de
signes, les deux facteurs nuls, les pointeurs nuls, les valeurs non finies
dans les deux arguments, les produits trop grands et ceux arrondis à zéro.
Ils vérifient aussi les résultats subnormaux et la conservation de la sortie
après un refus.

Les tests console contrôlent les deux champs, les unités affichées, les
saisies invalides, les dépassements positifs et négatifs, les sous-dépassements,
la récupération après erreur, la fin de saisie et les calculs successifs.
La suite conserve les contrôles de la loi d'Ohm, des résistances et de la saisie.

Consulter [les formules électriques](FORMULES.md),
[le diviseur de tension](DIVISEUR_TENSION.md) et [le README](../README.md).
