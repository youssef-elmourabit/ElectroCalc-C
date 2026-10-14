# Formules électriques et unités

## Loi d'Ohm — disponible

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

## Deux résistances en série — disponible

**R_eq = R1 + R2**, avec R1 et R2 strictement positives, en ohms (Ω).

Le même courant traverse les deux résistances. Leurs tensions s'additionnent :
U_total = R1 × I + R2 × I = (R1 + R2) × I.
Exemple : 100 Ω + 220 Ω = **320 Ω**.

Le code ordonne les termes (`a = min`, `b = max`) et vérifie
`a > DBL_MAX - b` avant d'additionner, puis contrôle que la somme est finie.
L'ordre rend le refus indépendant de la permutation de R1 et R2. Par exemple,
`DBL_MAX + 1` est refusé dans les deux ordres, même si un arrondi de l'addition
seule aurait masqué le dépassement. La variable de sortie reste inchangée en
cas d'erreur. `DBL_MAX`, fourni par `<float.h>`, est le plus grand `double` fini.

## Deux résistances en parallèle — disponible

**1 / R_eq = 1 / R1 + 1 / R2**, soit
**R_eq = (R1 × R2) / (R1 + R2)**.

Les deux résistances ont la même tension U. Les courants s'additionnent :
I_total = U/R1 + U/R2. Exemple : 100 Ω et 220 Ω donnent **68.75 Ω**.
Deux résistances égales R donnent **R/2**. Le résultat exact est positif et
inférieur à chacune des deux résistances ; un arrondi peut le rendre égal
à la plus petite quand leurs valeurs sont très éloignées.

### Forme utilisée dans le code

Poser `a = min(R1, R2)` et `b = max(R1, R2)` :

```text
R_eq = (a × b) / (a + b)
     = a / (1 + a/b)
```

Comme `0 < a <= b`, le rapport est entre 0 et 1 et le dénominateur entre 1 et 2.
La forme évite de calculer le produit et la somme des résistances. Elle évite
aussi de former les inverses `1/R1` et `1/R2`, qui peuvent dépasser la plage
pour des valeurs très petites. Les divisions sont sûres : b > 0, puis
1 + a/b >= 1.

- R1 = R2 = `1e308` : R_eq = `5e307`, malgré un produit et une somme directs
  hors plage.
- R1 = R2 = `1e-200` : R_eq = `5e-201`, malgré un produit direct arrondi à zéro.
- R1 = `1e-200`, R2 = `1e200` : R_eq ≈ `1e-200`. Le rapport peut s'arrondir
  à zéro ; son effet est alors inférieur à la précision utile du résultat.

Le résultat doit être fini et strictement positif. Un résultat subnormal
positif est accepté ; un résultat final arrondi à zéro est refusé. Par exemple,
deux valeurs `DBL_TRUE_MIN` en parallèle donnent un résultat exact égal à la
moitié du plus petit `double` positif, non représentable : le test C exige
un refus. Les tests C peuvent fournir ces valeurs directement ; la lecture
console refuse les conversions pour lesquelles `strtod()` signale `ERANGE`.

Les mêmes contrôles d'entrée s'appliquent aux deux montages : pointeur de
sortie non nul, valeurs finies et strictement positives. Le résultat de sortie
n'est écrit qu'après validation complète. Chaque calcul traite exactement
**deux résistances**, sans généralisation à un réseau.

## Diviseur de tension — disponible

**U_sortie = U_entrée × R2 / (R1 + R2)**, tensions en volts (V).

R1 relie l'entrée à la sortie, R2 relie la sortie à la masse. Pour un diviseur
idéal **non chargé**, le même courant traverse R1 et R2 :
I = U_entrée / (R1 + R2), puis U_sortie = I × R2.

Les deux résistances doivent être finies et strictement positives. La tension
d'entrée doit être finie ; elle peut être positive, négative ou nulle. Avec
12 V, 1000 Ω et 2000 Ω, la sortie vaut **8 V**. Avec deux résistances égales,
elle vaut U_entrée / 2. Son signe suit celui de l'entrée et sa valeur absolue
ne dépasse pas celle de l'entrée, à l'arrondi près.

Le code met les résistances à une échelle commune et garde séparés les
exposants de la tension et de R2. Il évite ainsi un débordement de R1 + R2 ou
de U_entrée × R2, et préserve certains résultats dont le rapport seul serait
arrondi à zéro. Exemple : U_entrée = `1e308`, R1 = `1e308`, R2 = `1e-200`
donne U_sortie ≈ `1e-200` V. Une entrée nulle produit zéro après validation
des résistances. Un résultat non nul mathématiquement mais arrondi à zéro
est refusé ; un résultat subnormal non nul est accepté.

Une charge modifie la résistance de la branche R2 et donc le résultat.
Son effet n'est pas calculé par cette fonction. Voir
[la conception du diviseur](DIVISEUR_TENSION.md) pour l'algorithme détaillé.

## Puissance électrique — disponible

**P = U × I**, avec P en watts (W), U en volts (V), I en ampères (A).

Les deux entrées sont finies, nulles ou signées. Avec 12 V et 2 A,
P = **24 W** ; avec 5 V et 0.5 A, P = **2.5 W**.

La **convention récepteur** définit le courant de référence comme entrant par
la borne de référence positive de la tension. Avec ces références :

- P > 0 : le dipôle absorbe de l'énergie.
- P < 0 : le dipôle fournit de l'énergie.
- P = 0 : la puissance est nulle ; U ou I peut être nul.

Deux valeurs négatives donnent un produit positif. Le signe seul de U ou de I
ne suffit donc pas à déterminer si le dipôle reçoit ou fournit de l'énergie.

Le produit est contrôlé avec `isfinite()` ; un produit devenu infini est refusé.
Si les deux facteurs sont non nuls mais que leur produit s'arrondit à zéro,
le calcul est également refusé. Un produit subnormal non nul reste accepté.
Le résultat de sortie reste inchangé en cas de refus.

Pour une résistance avec convention récepteur : P = R × I² = U² / R,
avec R > 0. Ces deux formes ne sont pas proposées par l'interface, qui utilise
exclusivement U et I. Pour des signaux variables, P = U × I décrit la puissance
instantanée si U et I sont des valeurs simultanées ; le programme ne calcule
pas les puissances active, réactive ou apparente à partir de valeurs efficaces
et d'un déphasage. Voir [le module de puissance](PUISSANCE.md).

Conversions utiles : 1 kΩ = 1000 Ω ; 1 mA = 0.001 A ; 1 mV = 0.001 V.
Le programme attend les unités de base ; il ne convertit pas les préfixes.
