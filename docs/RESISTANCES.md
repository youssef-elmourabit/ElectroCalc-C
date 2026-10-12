# Résistances en série et en parallèle — conception et validation

## Architecture du module

L'application sépare les responsabilités de l'interface et des calculs :

- `main.c` appelle le menu.
- `cli.c` affiche les questions et résultats.
- `input.c` lit une ligne et valide sa conversion numérique.
- `ohm.c` effectue le calcul électrique sans dépendre du clavier.
- `resistance.c` calcule les résistances équivalentes sans dépendre du clavier.
- `include/` déclare les fonctions ; `tests/` vérifie le calcul et la console.

Les deux montages réutilisent la lecture et la validation communes.

## Fichiers et responsabilités

| Fichier | Responsabilité |
| --- | --- |
| `include/resistance.h` | Déclarations des deux fonctions de calcul |
| `src/resistance.c` | Validation des paramètres, calculs série et parallèle |
| `src/cli.c` | Options 2 et 3 actives, questions R1/R2 et résultat en Ω |
| `Makefile` | Nouveau module inclus dans l'application et les tests C ; cible `clean` limitée aux deux exécutables |
| `tests/test_core.c` | Vérification des calculs, des conversions et des limites numériques |
| `tests/test_cli.py` | Tests console des calculs, de la validation et des options 4 et 5 en attente |
| `README.md` | Présentation, fonctionnalités, structure, commandes et exemples |
| `docs/FORMULES.md` | Formules, raisonnement physique et limites numériques |
| `docs/RESISTANCES.md` | Conception et validation des calculs de résistances |

Le module respecte le contrat commun de validation des entrées. Le diviseur
de tension et la puissance restent « À développer » et ne font pas partie
des calculs disponibles.

## Fonctions et concepts

`resistance_series(R1, R2, &R_eq)` et
`resistance_parallel(R1, R2, &R_eq)` renvoient `true` si le résultat est valide.
En cas de refus, elles renvoient `false` et la variable de sortie garde sa
valeur. Ce contrat reprend le style de `ohm_voltage()` : un booléen pour le
statut, un pointeur pour le résultat. Aucun message console ne figure dans
le module de calcul, ce qui le rend directement testable.

`valid_resistances()` est une fonction `static`, interne à `resistance.c`.
Elle exige deux résistances finies et > 0, ainsi qu'un pointeur de sortie
non nul. La vérification existe dans le module même si l'interface valide
déjà les saisies : un futur appelant doit respecter le même contrat physique.

`calculate_resistance(parallel)` centralise les questions identiques aux deux
montages. Son paramètre booléen choisit la formule ; le menu lui transmet
`choice == 3`. `ask_number()` impose R > 0 et redemande une saisie après erreur.
La fin de saisie quitte proprement, y compris pendant R1 ou R2. Après un
calcul ou une erreur numérique, le programme revient au menu.

## Formules et robustesse numérique

En série : **R_eq = R1 + R2**. Avec R1 = 100 Ω et R2 = 220 Ω, le résultat
est 320 Ω. Le code pose a = min(R1, R2) et b = max(R1, R2), puis compare a
à `DBL_MAX - b` avant l'addition afin de refuser une somme hors plage.
Ordonner les termes assure un refus symétrique, notamment pour `DBL_MAX + 1`.
Un contrôle `isfinite()` complète cette garde.

En parallèle : **R_eq = (R1 × R2) / (R1 + R2)**. Le résultat est 68.75 Ω
pour les mêmes entrées. Calculer directement ce produit et cette somme peut
déborder même si le résultat final est représentable. Le code utilise donc
**a / (1 + a/b)**, avec a la plus petite résistance et b la plus grande.
`fmin()` et `fmax()` proviennent de la bibliothèque mathématique standard,
utilisée par le projet ; aucune dépendance tierce n'est requise.

Le dénominateur est entre 1 et 2 ; b est strictement positif. Il n'y a donc
aucune division par zéro. Un résultat final arrondi à zéro est refusé.
Un résultat positif subnormal est accepté, avec la précision limitée de
`double`. Les erreurs de conversion `ERANGE` restent refusées à la saisie.
Les détails et exemples sont dans [les formules électriques](FORMULES.md).

## Validation automatisée

Commande depuis le dossier `ElectroCalc-c` :

```sh
make clean
make
make test
```

`make clean` supprime uniquement `build/electrocalc` et `build/test_core` ;
les sources et journaux sont conservés. `make` reconstruit l'application,
puis `make test` compile les tests C et exécute les deux suites.

Compilation : C11 avec `-Wall -Wextra -Wpedantic`, sans dépendance tierce.
Résultat : **93/93 vérifications C** et **35 méthodes console réussies**.
Les méthodes avec `subTest` couvrent plusieurs valeurs et champs de saisie.

| Scénario | Résultat attendu et vérifié |
| --- | --- |
| Série : 100 Ω, 220 Ω | 320 Ω |
| Parallèle : 100 Ω, 220 Ω | 68.75 Ω |
| Deux résistances de 100 Ω | 200 Ω en série, 50 Ω en parallèle |
| Résistances décimales 0.1 Ω, 0.2 Ω | ≈ 0.3 Ω en série, ≈ 0.06666666667 Ω en parallèle |
| R1/R2 échangées | Même résultat |
| Zéro, négatif, NaN, infini | Refus dans chacun des deux arguments |
| Texte, virgule, suffixe, octet nul, longue ligne | Erreur puis récupération à la saisie |
| Série : 1e308 Ω, 1e308 Ω | Dépassement refusé, retour au menu et calcul suivant possible |
| Série : DBL_MAX Ω et 1 Ω, dans les deux ordres | Dépassement refusé sans dépendre de l'ordre |
| Parallèle : 1e308 Ω, 1e308 Ω | 5e307 Ω |
| Parallèle : 1e-200 Ω, 1e-200 Ω | 5e-201 Ω |
| Parallèle : 1e-200 Ω, 1e200 Ω | ≈ 1e-200 Ω, dans les deux ordres |
| Résultat subnormal positif | Accepté, notamment 4e-308 Ω en parallèle avec 4e-308 Ω |
| Deux DBL_TRUE_MIN en parallèle (test C) | Refus du résultat arrondi à zéro |
| Pointeur de sortie nul ou calcul invalide | Refus ; sortie inchangée si fournie |
| Fin de saisie à R1 ou R2 | Sortie propre sans résultat incomplet |
| Série, parallèle, loi d'Ohm dans une session | Trois résultats puis retour au menu |
| Options 4 et 5 | Toujours « À développer » |

La suite couvre la loi d'Ohm, les deux montages de résistances et la saisie
commune. Les tests des options en attente vérifient qu'aucun résultat n'est
produit pour le diviseur de tension ou la puissance.

## Exercices d'application

1. **Calcul et unités** : pour R1 = 1 kΩ et R2 = 2.2 kΩ, calculer les deux
   équivalents à la main, convertir les entrées en Ω puis vérifier les options
   2 et 3. Ajouter un test unitaire pour la série (3200 Ω) et un test console
   pour le parallèle (≈ 687.5 Ω).
2. **Propriétés physiques** : choisir trois paires de résistances positives et
   ajouter des tests vérifiant que la série est au moins égale à la plus grande,
   le parallèle au plus égal à la plus petite, et que permuter R1/R2 conserve
   le résultat. Utiliser une tolérance pour les comparaisons décimales.
3. **Limites numériques** : expliquer pourquoi la formule directe en parallèle
   échoue pour deux valeurs `1e308`, alors que la forme stable donne `5e307`.
   Faire le même raisonnement pour deux valeurs `1e-200`, puis ajouter un test
   avec deux valeurs `1e150` attendant `5e149` (avec tolérance relative).

Après chaque modification, lancer `make test`. Consulter également
[la conception du module de loi d'Ohm](LOI_OHM.md) pour comparer les contrats
des fonctions de calcul.
