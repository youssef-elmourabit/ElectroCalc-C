# ElectroCalc-C

Application console de calculs électriques en **C11**, conçue pour l'analyse
élémentaire de circuits résistifs. ElectroCalc calcule une tension par la loi
d'Ohm, la résistance équivalente de deux résistances en série ou en parallèle,
la tension de sortie d'un diviseur idéal non chargé et la puissance électrique.

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
| 4 | Diviseur de tension idéal non chargé | U_sortie = U_entrée × R2 / (R1 + R2) |
| 5 | Puissance électrique | P = U × I |
| 0 | Quitter l'application | — |

- Menu interactif, résultat avec unités et retour au menu après chaque calcul.
- Résistances strictement positives ; tensions et courants nuls ou signés.
- Refus des saisies invalides, des valeurs non finies et des calculs hors plage.
- Fermeture propre à la fin de saisie, notamment avec Ctrl+D sur une ligne vide
  sous macOS.

## Fonctionnalités à venir

Pistes d'évolution : calcul inverse de R ou I à partir des deux autres grandeurs
et traitement de réseaux de plus de deux résistances. Ces calculs ne sont pas
disponibles dans la version actuelle.

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
│   ├── resistance.h       # Contrats des calculs série et parallèle
│   ├── divider.h          # Contrat du diviseur idéal non chargé
│   └── power.h            # Contrat du calcul de puissance
├── src/
│   ├── cli.c              # Menu, questions et affichage des résultats
│   ├── input.c            # Lecture bornée, strtod et strtol
│   ├── ohm.c              # Loi d'Ohm et validation des paramètres
│   ├── resistance.c       # Résistances équivalentes et limites numériques
│   ├── divider.c          # Diviseur avec mise à l'échelle numérique
│   └── power.c            # Produit U × I et validation du résultat
├── tests/
│   ├── test_core.c        # Tests unitaires C
│   └── test_cli.py        # Tests du véritable programme console
├── docs/
│   ├── FORMULES.md        # Modèles, unités, hypothèses et limites
│   ├── LOI_OHM.md         # Conception et validation du module de tension
│   ├── RESISTANCES.md     # Conception et validation des deux montages
│   ├── DIVISEUR_TENSION.md # Hypothèses, algorithme et tests du diviseur
│   └── PUISSANCE.md       # Convention de signe et tests de puissance
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

Saisir les valeurs **sans unité**, en volts (V), en ohms (Ω) et en ampères (A). Les préfixes
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

Le diviseur décompose les nombres en mantisses et exposants binaires avec
`frexp()` et `ldexpl()`. Les opérations normalisées sont effectuées en
`long double`, puis converties en `double` pour le résultat. Il évite la somme
R1 + R2, le produit U_entrée × R2
et un rapport prématurément arrondi à zéro. Une entrée nulle est acceptée.
Pour le diviseur et la puissance, un résultat subnormal non nul est accepté ;
un résultat non nul mathématiquement mais arrondi à zéro est refusé. La
puissance refuse aussi les produits devenus infinis. L'affichage utilise
10 chiffres significatifs, avec la précision interne du type `double`.

Le diviseur suppose R1 entre l'entrée et la sortie, R2 entre la sortie et la
masse, et **aucune charge** en sortie. Pour la puissance, la **convention
récepteur** est utilisée : le courant de référence entre par la borne de
référence positive de la tension. P > 0 correspond à une puissance absorbée,
P < 0 à une puissance fournie. U et I doivent décrire le même dipôle selon
ces références ; le programme ne déduit pas le câblage à partir des signes.

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

Résultat vérifié : **183/183 vérifications C et 47 méthodes de test console
réussies**. Plusieurs méthodes couvrent différentes entrées avec `subTest`.

La suite contrôle notamment les résultats usuels, la symétrie R1/R2, les
résistances égales, les décimales, les pointeurs nuls, les paramètres non finis,
les dépassements, les résultats subnormaux, les saisies invalides ou trop longues,
la fin de saisie et les cinq calculs successifs. Les tests existants de la loi
d'Ohm, des résistances et de la saisie restent inclus. Les anciennes attentes
des options en préparation ont été adaptées à leurs calculs désormais actifs.
Les nouveaux tests vérifient aussi les tensions signées du diviseur, les quatre
combinaisons de signes de U et I, et les résultats arrondis à zéro.

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

### Diviseur de tension

```text
Votre choix : 4
Tension d'entrée U_entrée (V) : 12
Résistance R1 (Ω) : 1000
Résistance R2 (Ω) : 2000
Résultat : U_sortie = 8 V (U_entrée = 12 V, R1 = 1000 Ω, R2 = 2000 Ω)
Retour au menu.
```

Avec R1 = R2 et U_entrée = 10 V, U_sortie = 5 V. Une entrée de -12 V dans
le premier exemple donne -8 V ; une entrée nulle donne 0 V.

### Puissance électrique

```text
Votre choix : 5
Tension U (V) : 12
Courant I (A) : 2
Résultat : P = 24 W (U = 12 V, I = 2 A)
Retour au menu.
```

| U (V) | I (A) | P (W) | Interprétation en convention récepteur |
| --- | --- | --- | --- |
| 12 | 2 | 24 | Puissance absorbée |
| 5 | 0.5 | 2.5 | Puissance absorbée |
| 0 | 2 | 0 | Puissance nulle |
| -12 | 2 | -24 | Puissance fournie |
| 12 | -2 | -24 | Puissance fournie |
| -12 | -2 | 24 | Puissance absorbée |

## Documentation technique

- [Formules électriques, unités et limites](docs/FORMULES.md)
- [Loi d'Ohm : architecture, validation et exercices](docs/LOI_OHM.md)
- [Résistances : algorithmes, stabilité numérique et exercices](docs/RESISTANCES.md)
- [Diviseur de tension : modèle non chargé et calcul stable](docs/DIVISEUR_TENSION.md)
- [Puissance : convention récepteur et limites numériques](docs/PUISSANCE.md)
