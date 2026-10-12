# ElectroCalc-C

Application console de calculs électriques en **C11**, conçue pour l'analyse
élémentaire de circuits résistifs. ElectroCalc calcule une tension par la loi
d'Ohm et la résistance équivalente de deux résistances en série ou en parallèle.

Le projet associe les modèles d'électricité à une architecture modulaire,
une validation stricte des entrées et des tests automatisés. Il s'inscrit dans
une démarche académique d'ingénierie électronique et de développement logiciel.

[Dépôt GitHub](https://github.com/youssef-elmourabit/ElectroCalc-C)

## Fonctionnalités disponibles

| Option | Calcul | Relation |
| --- | --- | --- |
| 1 | Tension aux bornes d'une résistance | U = R × I |
| 2 | Deux résistances en série | R_eq = R1 + R2 |
| 3 | Deux résistances en parallèle | R_eq = (R1 × R2) / (R1 + R2) |
| 0 | Quitter l'application | — |

- Menu interactif, résultat avec unités et retour au menu après chaque calcul.
- Résistances strictement positives ; courant nul ou signé pour la loi d'Ohm.
- Refus des saisies invalides, des valeurs non finies et des calculs hors plage.
- Fermeture propre à la fin de saisie, notamment avec Ctrl+D sur une ligne vide
  sous macOS.

## Fonctionnalités à venir

Le **diviseur de tension** (option 4) et la **puissance électrique** (option 5)
sont affichés comme « À développer » et ne réalisent aucun calcul.

Le calcul inverse de R ou I à partir des deux autres grandeurs et le traitement
de réseaux de plus de deux résistances ne sont pas disponibles.

## Architecture du code

Les modules de calcul sont indépendants de l'interface console. Leurs fonctions
renvoient un booléen de succès et transmettent le résultat par pointeur ;
une erreur laisse la variable de sortie inchangée.

```text
ElectroCalc-c/
├── main.c                 # Point d'entrée de l'application
├── include/
│   ├── cli.h              # Contrat de l'interface console
│   ├── input.h            # Lecture et conversion des saisies
│   ├── ohm.h              # Contrat du calcul de tension
│   └── resistance.h       # Contrats des calculs série et parallèle
├── src/
│   ├── cli.c              # Menu, questions et affichage des résultats
│   ├── input.c            # Lecture bornée, strtod et strtol
│   ├── ohm.c              # Loi d'Ohm et validation des paramètres
│   └── resistance.c       # Résistances équivalentes et limites numériques
├── tests/
│   ├── test_core.c        # Tests unitaires C
│   └── test_cli.py        # Tests du véritable programme console
├── docs/
│   ├── FORMULES.md        # Modèles, unités, hypothèses et limites
│   ├── LOI_OHM.md         # Conception et validation du module de tension
│   └── RESISTANCES.md     # Conception et validation des deux montages
├── build/                 # Exécutables et journaux locaux ignorés par Git
├── Makefile
├── .gitignore
└── README.md
```

## Technologies utilisées

- **C11** et bibliothèque standard C, avec la bibliothèque mathématique standard.
- **Clang ou GCC** pour la compilation et **Make** pour les commandes du projet.
- **Python 3**, avec sa bibliothèque standard uniquement, pour les tests console.

Aucune bibliothèque tierce n'est nécessaire. Environnement vérifié : Apple
Clang 14.0.3, GNU Make 3.81 et Python 3.12.7 sous macOS. Sur cette machine,
la commande `gcc` utilise également Apple Clang.

## Compilation et exécution

Depuis le dossier du projet :

```sh
make
make run
```

L'exécutable est `build/electrocalc` ; il peut aussi être lancé directement :

```sh
./build/electrocalc
```

La compilation active `-std=c11 -Wall -Wextra -Wpedantic`. Les en-têtes sont
recherchés avec `-Iinclude`. Pour utiliser un autre compilateur :

```sh
make -B CC=gcc
```

Pour reconstruire entièrement l'application et les tests :

```sh
make clean
make
make test
```

`make clean` supprime uniquement `build/electrocalc` et `build/test_core`.
Les sources, documents et journaux de tests sont conservés.

### Saisie et limites numériques

Saisir les valeurs **sans unité**, en ohms (Ω) et en ampères (A). Les préfixes
ne sont pas convertis : 20 mA doivent être saisis comme `0.02`. Utiliser le
point décimal ; la notation scientifique et les espaces autour d'un nombre
sont acceptés. Une ligne contient au maximum 255 caractères, hors saut de ligne.

Le programme refuse le texte résiduel, la virgule décimale, NaN, l'infini,
les résistances nulles ou négatives et les conversions signalant `ERANGE`.
Certaines entrées subnormales très petites peuvent donc être refusées.

La somme en série est contrôlée avant l'addition. Le parallèle utilise la forme
stable `min / (1 + min / max)`, évitant le produit et la somme intermédiaires.
Les dénominateurs sont strictement positifs. Un résultat de résistance positif
subnormal est accepté ; un résultat arrondi à zéro est refusé. Un produit de
la loi d'Ohm devenu infini est refusé ; son éventuel arrondi à zéro pour des
valeurs extrêmement petites n'est pas détecté séparément.

Le type `double` impose une précision et une plage limitées ; une contribution
très petite peut disparaître par arrondi. Le programme suppose des résistances
idéales et ne modélise ni tolérance, ni température, ni comportement transitoire.
Il ne constitue pas un outil de métrologie. Voir [les formules](docs/FORMULES.md).

## Tests

```sh
make test
```

La commande compile et exécute les tests unitaires C, puis les tests console
contre le véritable exécutable. Un échec produit un code de retour non nul.

Résultat vérifié : **93/93 vérifications C et 35 méthodes de test console
réussies**. Plusieurs méthodes couvrent différentes entrées avec `subTest`.

La suite contrôle notamment les résultats usuels, la symétrie R1/R2, les
résistances égales, les décimales, les pointeurs nuls, les paramètres non finis,
les dépassements, les résultats subnormaux, les saisies invalides ou trop longues,
la fin de saisie et les calculs successifs. Les tests existants de la loi d'Ohm
et de la saisie restent inclus.

Les tests ont été exécutés sous macOS ; la validation sur Windows et Linux
n'a pas été réalisée.

## Exemples d'utilisation

### Loi d'Ohm

```text
Votre choix : 1
Résistance R (Ω) : 100
Courant I (A) : 0.02
Résultat : U = 2 V (R = 100 Ω, I = 0.02 A)
Retour au menu.
```

| R (Ω) | I (A) | Résultat |
| --- | --- | --- |
| 100 | 0.02 | 2 V |
| 220 | 0.05 | 11 V |
| 100 | 0 | 0 V |
| 100 | -0.02 | -2 V, selon la convention de signe choisie |
| 0 ou valeur négative | 0.02 | Résistance refusée, nouvelle saisie demandée |

### Résistances équivalentes

```text
Votre choix : 3
Résistance R1 (Ω) : 100
Résistance R2 (Ω) : 220
Résultat : R_eq = 68.75 Ω (R1 = 100 Ω, R2 = 220 Ω)
Retour au menu.
```

| R1 (Ω) | R2 (Ω) | Série (Ω) | Parallèle (Ω) |
| --- | --- | --- | --- |
| 100 | 220 | 320 | 68.75 |
| 100 | 100 | 200 | 50 |
| 0.1 | 0.2 | ≈ 0.3 | ≈ 0.06666666667 |
| 1e308 | 1e308 | Refus : dépassement | 5e307 |

## Documentation technique

- [Formules électriques, unités et limites](docs/FORMULES.md)
- [Loi d'Ohm : architecture, validation et exercices](docs/LOI_OHM.md)
- [Résistances : algorithmes, stabilité numérique et exercices](docs/RESISTANCES.md)
