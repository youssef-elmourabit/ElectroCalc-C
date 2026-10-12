# Loi d'Ohm — conception et validation

## Architecture du module

`main.c`, conservé à sa place, appelle `run_menu()`. `src/` contient les
implémentations et `include/` leurs déclarations. Un en-tête décrit ce qu'un
autre fichier peut appeler ; les gardes `#ifndef` évitent les inclusions multiples.
Le Makefile compile les sources ensemble et reconstruit les exécutables si
un source, un en-tête ou le Makefile change.

Exercice rapide : lire une déclaration dans `ohm.h`, puis retrouver son corps
dans `ohm.c`. Repérer le type du retour et le rôle de chaque paramètre.

## Calcul électrique et contrat de la fonction

`ohm_voltage(R, I, &U)` vérifie les paramètres, calcule `R * I`, puis écrit le
résultat dans U. Le retour `bool` indique succès ou refus. Le pointeur `&U`
permet de transmettre un résultat tout en conservant ce retour de statut.
Un pointeur nul est refusé et une erreur ne modifie pas la valeur de sortie.
`isfinite()` évite de traiter NaN ou l'infini comme des mesures valides.

Le type `double` représente des nombres réels avec une précision finie.
Les tests comparent donc 2 V et 11 V avec une tolérance `1e-12`, plutôt qu'en
supposant que tout calcul décimal est exact en mémoire.

Exercice rapide : calculer à la main U pour R = 470 Ω et I = 0.01 A.

## Lecture et validation des entrées

`read_line()` lit une ligne dans un tableau de taille fixe et consomme tout
excédent. Son `enum InputStatus` distingue une ligne valide, une ligne refusée
et la fin du flux. Une ligne trop longue ne pollue donc pas la question suivante.

`parse_number()` utilise `strtod()` : le pointeur de fin permet de refuser
`100abc` au lieu de n'accepter que son début numérique. `errno` signale les
conversions hors plage. `parse_menu_choice()` utilise `strtol()` pour exiger
un entier. Les espaces de fin sont autorisés, les caractères restants refusés.

`ask_number()` répète une question jusqu'à recevoir une valeur valide ou une
fin de saisie. Elle ajoute la règle R > 0 pour la résistance. Les fonctions de
conversion restent indépendantes de cette règle physique.

Exercice rapide : prévoir le comportement de `abc`, `0,02`, `2e-2`, `nan` et `0`.

## Interface console et tests

`run_menu()` utilise une boucle et un `switch`. `calculate_ohm()` coordonne
les questions et le résultat. Les fonctions internes marquées `static` sont
visibles uniquement dans leur fichier. `fflush(stdout)` affiche l'invite même
sans saut de ligne. `0` quitte depuis le menu ; pendant un calcul, la fin de
saisie permet de quitter (Ctrl+D sur une ligne vide).

`test_core.c` contrôle directement les fonctions C et retourne un échec si
une vérification échoue. `test_cli.py` envoie des saisies au véritable programme,
inspecte sa sortie et impose un délai pour détecter les boucles bloquées.
Le script utilise uniquement la bibliothèque standard Python.

## Exercices d'application

1. **Unités et résultat** : vérifier à la main R = 470 Ω, I = 10 mA, convertir
   le courant en A, puis ajouter un cas console attendant 4.7 V et lancer
   `make test`.
2. **Présentation** : remplacer `%.10g` par `%.3f` dans l'affichage du résultat,
   prévoir la différence entre `2` et `2.000`, puis adapter les attentes des
   tests concernés. Observer pourquoi le calcul reste identique.
3. **Validation** : ajouter un test console avec R = `100abc`, puis une R
   correcte. Expliquer le rôle du pointeur `end` et vérifier qu'un seul message
   d'erreur précède le résultat final.

Après chaque modification, exécuter `make test` pour vérifier le calcul et
le comportement de l'interface. Consulter [les formules électriques](FORMULES.md)
et [le module des résistances](RESISTANCES.md) pour compléter l'étude.
