# Diviseur de tension — conception et validation

## Modèle électrique

```text
U_entrée ── R1 ──┬── U_sortie (mesurée par rapport à la masse)
                 R2
                  │
                 masse
```

R1 et R2 sont idéales, strictement positives et finies. La sortie est à vide :
aucune charge ne prélève de courant au point milieu. Le même courant traverse
les deux résistances, d'où :

```text
I = U_entrée / (R1 + R2)
U_sortie = I × R2 = U_entrée × R2 / (R1 + R2)
```

Les tensions sont exprimées en volts et les résistances en ohms. La tension
d'entrée est finie et peut être signée ou nulle. Pour 12 V, R1 = 1000 Ω et
R2 = 2000 Ω, la sortie vaut 8 V. Deux résistances égales divisent la tension
par deux. Une entrée négative donne une sortie négative.

Une charge branchée en parallèle avec R2 modifie le rapport du diviseur.
Le modèle non chargé ne doit pas être utilisé tel quel dans ce cas. Il ne
représente pas non plus les tolérances, la température ou les transitoires.

## Architecture et contrat

- `include/divider.h` déclare `divider_voltage(U_entrée, R1, R2, &U_sortie)`.
- `src/divider.c` valide les paramètres et calcule sans interaction console.
- L'option 4 de `src/cli.c` réutilise `ask_number()`, `read_line()` et
  `parse_number()`, puis affiche le résultat en volts.
- Le Makefile inclut le module dans l'application et les tests C.

Un pointeur nul, une tension non finie, ou une résistance non finie, nulle ou
négative provoque un retour `false`. Les résistances sont validées même si
la tension d'entrée est nulle. Une erreur laisse la sortie inchangée ; un
succès écrit la tension et retourne `true`. La fin de saisie quitte proprement,
et une erreur numérique rend la main au menu.

## Calcul stable par mise à l'échelle

L'évaluation directe `U_entrée * R2 / (R1 + R2)` peut déborder alors que la
sortie est représentable. Calculer le rapport avant le produit peut, à
l'inverse, arrondir le rapport à zéro et perdre un résultat récupérable.

`frexp()` décompose chaque nombre non nul sous la forme `m × 2^e`, avec
`0.5 <= |m| < 1`. Poser :

```text
U_entrée = mu × 2^eu
R1 = m1 × 2^e1
R2 = m2 × 2^e2
e = max(e1, e2)
d = m1 × 2^(e1-e) + m2 × 2^(e2-e)
U_sortie = [mu × (m2/d)] × 2^[eu + (e2-e)]
```

`ldexpl()` applique une puissance de deux en précision `long double`.
Les mantisses obtenues en `double` sont converties exactement en `long double`
pour les opérations normalisées ; seule la sortie finale est reconvertie en
`double`. Cette précision intermédiaire évite notamment de perdre le côté
de la frontière d'arrondi dans le cas `U_entrée = DBL_TRUE_MIN`,
`R1 = nextafter(1.0, 0.0)` et `R2 = 1` : la sortie doit s'arrondir à
`DBL_TRUE_MIN`, et non à zéro.

Le dénominateur d est entre 0.5
inclus et 2 exclu, donc fini et strictement positif. Le rapport `m2/d` est
entre 0.25 et 2, sans valeur intermédiaire extrême. Les exposants gardent
l'échelle physique jusqu'à la dernière opération. Le petit terme du
dénominateur peut disparaître par arrondi, mais le numérateur reste présent.
La formule utilise uniquement la bibliothèque mathématique standard C.
Sur l'environnement macOS x86_64 vérifié, `long double` a 64 bits de précision
contre 53 pour `double`. Cette précision supplémentaire n'est pas garantie
par C11 sur toutes les plateformes ; les cas de frontière doivent être
revalidés lors d'un portage.

Exemples vérifiés :

| Entrée (V) | R1 (Ω) | R2 (Ω) | Sortie attendue (V) |
| --- | --- | --- | --- |
| 12 | 1000 | 2000 | 8 |
| 10 | 1000 | 1000 | 5 |
| 0 | 1000 | 2000 | 0 |
| -12 | 1000 | 2000 | -8 |
| 12 | DBL_MAX | DBL_MAX | 6 |
| DBL_MAX | DBL_MAX | DBL_MAX | DBL_MAX / 2 |
| 1e308 | 1e308 | 1e-200 | ≈ 1e-200 |
| 1e308 | 1e308 | 4e-308 | ≈ 4e-308 |
| DBL_MAX | DBL_MAX | DBL_TRUE_MIN | DBL_TRUE_MIN |
| DBL_TRUE_MIN | nextafter(1.0, 0.0) | 1 | DBL_TRUE_MIN |
| DBL_TRUE_MIN | 1 | 1 | Refus du résultat arrondi à zéro |

`DBL_MAX` et `DBL_TRUE_MIN` viennent de `<float.h>` ; ils désignent le plus
grand `double` fini et le plus petit `double` strictement positif. Les tests
C peuvent transmettre ces constantes directement. La console refuse les
conversions pour lesquelles `strtod()` signale `ERANGE`, dont certaines
entrées subnormales.

Un résultat non fini est refusé. Une entrée non nulle ne peut physiquement
donner zéro avec deux résistances positives ; un résultat arrondi à zéro
est donc refusé. Les résultats subnormaux non nuls sont acceptés. Les arrondis
restent ceux de `double` et la console affiche 10 chiffres significatifs.
Pour un rapport proche de 1, la sortie peut s'arrondir à la tension d'entrée.
Le module ne fournit pas de borne certifiée sur l'erreur d'arrondi.

## Validation

`make test` contrôle les résultats usuels, les tensions négatives et nulles,
les résistances invalides dans chaque argument, les pointeurs nuls et la
conservation de la sortie en cas d'échec. Les cas extrêmes vérifient la somme
et le produit intermédiaires hors plage, un rapport minuscule récupérable,
un résultat subnormal et les deux côtés de la frontière d'arrondi à zéro.

Les tests console couvrent chaque champ : texte, suffixe, virgule décimale,
NaN, infini, conversion hors plage, octet nul et ligne trop longue. Ils
vérifient également la fin de saisie à chaque question, une dernière ligne
sans saut de ligne et la récupération après erreur.

Consulter [les formules électriques](FORMULES.md),
[le module de puissance](PUISSANCE.md) et [le README](../README.md)
pour les commandes et le bilan de la suite complète.
