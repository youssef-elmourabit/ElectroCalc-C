# ElectroCalc-C

Application console pédagogique de calculs électriques, écrite en **C11**.
Projet de portfolio de Licence Électronique–Télécommunications, Faculté des
Sciences de Rabat.

## Objectifs

Appliquer les notions d'électricité avec du code simple, séparer calcul et
interface, apprendre à valider des saisies et vérifier les résultats par des
tests reproductibles.

## Fonctionnalités disponibles — Jour 1

- Menu interactif et sortie avec `0`.
- Tension par la loi d'Ohm : **U = R × I**.
- Résistance strictement positive, courant nul ou signé.
- Affichage en volts (V), ohms (Ω) et ampères (A).
- Nouvelle demande après une saisie incorrecte ; retour automatique au menu
  après un calcul, puis possibilité de recalculer ou quitter.
- Fermeture propre en fin de saisie (Ctrl+D sur une ligne vide sous macOS).

## Fonctionnalités prévues

Résistances en série, résistances en parallèle, diviseur de tension et puissance
électrique. Les options 2 à 5 affichent **« À développer »** et n'effectuent
aucun calcul. Calculer R ou I à partir des deux autres grandeurs est également
hors du périmètre du Jour 1.

## Technologies et prérequis

C11, bibliothèque standard C et bibliothèque mathématique standard, Clang ou
GCC, Make. Python 3 sert uniquement aux tests de l'interface ; aucune dépendance
Python supplémentaire n'est requise. Aucun paquet n'a été installé.

Environnement vérifié : Apple Clang 14.0.3, GNU Make 3.81, Git 2.39.2 et
Python 3.12.7. Sur cette machine, `gcc` est aussi Apple Clang.

## Structure

```text
ElectroCalc-c/
├── main.c                 # Point d'entrée existant
├── include/
│   ├── cli.h              # Déclaration du menu
│   ├── input.h            # Statuts de lecture et fonctions de conversion
│   └── ohm.h              # Contrat du calcul électrique
├── src/
│   ├── cli.c              # Menu, questions et résultats
│   ├── input.c            # Lecture bornée, strtod et strtol
│   └── ohm.c              # Calcul U = R × I et validation
├── tests/
│   ├── test_core.c        # Tests des fonctions C
│   └── test_cli.py        # Tests du véritable programme console
├── docs/
│   ├── FORMULES.md        # Formules, unités et hypothèses
│   └── JOUR_1.md          # Explications pédagogiques et exercices
├── build/                 # Exécutables générés, ignorés par Git
├── Makefile
├── .gitignore
└── README.md
```

## Compilation

Depuis ce dossier :

```sh
make
```

La commande utilise `-std=c11 -Wall -Wextra -Wpedantic` et crée
`build/electrocalc`. Les en-têtes sont recherchés avec `-Iinclude`. Pour choisir
un autre compilateur, utiliser `make -B CC=gcc`. Aucun fichier source n'est
supprimé et aucune installation globale n'est nécessaire.

## Exécution

```sh
make run
# Ou directement :
./build/electrocalc
```

Saisir uniquement les nombres, sans unité. Utiliser un point décimal : `0.02`,
pas `0,02`. La notation scientifique (`2e-2`) et les espaces autour d'un nombre
sont acceptés. Une ligne contient au maximum 255 caractères, hors saut de ligne.

Les valeurs non numériques, les caractères supplémentaires, NaN, l'infini et
les conversions hors de la plage du type `double` sont refusés. La résistance
nulle ou négative est refusée. Un produit devenu infini est signalé sans afficher
de résultat. Les nombres flottants ont une précision limitée ; il ne s'agit
pas d'un outil de métrologie. Aucune division n'est réalisée au Jour 1.

## Tests

```sh
make test
```

La cible compile le programme et `build/test_core`, exécute les tests C,
puis lance `tests/test_cli.py` contre le véritable exécutable. Elle retourne
un code non nul en cas d'échec.

Résultat de référence du Jour 1 : **44/44 vérifications C**,
**22 méthodes de test console réussies**, dont plusieurs testent différentes
entrées avec `subTest`. Les cas imposés 2 V, 11 V, 0 V, R négative, R nulle,
saisie non numérique et menu inexistant sont couverts. Sont aussi vérifiés :
fin de saisie, récupération après erreur, calculs successifs, saisies trop longues,
valeurs non finies et dépassement lors du produit.

## Exemples

```text
Votre choix : 1
Résistance R (Ω) : 100
Courant I (A) : 0.02
Résultat : U = 2 V (R = 100 Ω, I = 0.02 A)
Retour au menu.
```

| R (Ω) | I (A) | Résultat |
| --- | --- | --- |
| 100 | 0.02 | U = 2 V |
| 220 | 0.05 | U = 11 V |
| 100 | 0 | U = 0 V |
| -100 | 0.02 | Résistance refusée, nouvelle saisie demandée |
| 0 | 0.02 | Résistance refusée, nouvelle saisie demandée |

## Git

Le projet peut constituer un dépôt autonome : seuls les sources, tests,
documents et fichiers de configuration sont à versionner. `.gitignore` exclut
`build/`, les objets, caches et fichiers temporaires courants. Aucune publication
ni aucun push n'a été effectué. Les fichiers du workspace parent ne sont pas
nécessaires à la compilation de ce projet.
