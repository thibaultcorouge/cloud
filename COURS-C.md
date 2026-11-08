# Le C — cours progressif

> Pour quelqu'un qui connaît Python / JS / TS / Go et découvre le C.
> À lire dans l'ordre. Chaque chapitre suppose les précédents.
>
> Compagnon : `NOTES.md` (journal du projet, pièges réellement rencontrés).

---

## Sommaire

| | Chapitre | Pourquoi c'est là |
|---|---|---|
| **0** | [Le modèle mental](#chapitre-0--le-modèle-mental) | Comprendre *ce qu'est* le C avant sa syntaxe |
| **1** | [La chaîne de compilation](#chapitre-1--la-chaîne-de-compilation) | La chose qui n'existe dans aucun de tes langages |
| **2** | [La mémoire](#chapitre-2--la-mémoire) | **Le chapitre central.** Tout le C en découle |
| **3** | [Les types et leur représentation](#chapitre-3--les-types-et-leur-représentation) | Les octets, vraiment |
| **4** | [Les pointeurs](#chapitre-4--les-pointeurs) | Le cœur du langage |
| **5** | [Les tableaux](#chapitre-5--les-tableaux) | Et leur relation troublante avec les pointeurs |
| **6** | [Les chaînes de caractères](#chapitre-6--les-chaînes-de-caractères) | Il n'y en a pas |
| **7** | [La mémoire dynamique](#chapitre-7--la-mémoire-dynamique) | `malloc` / `free` — pas de GC |
| **8** | [Structurer ses données](#chapitre-8--structurer-ses-données) | `struct`, `typedef`, `enum`, `union` |
| **9** | [Fonctions, portées, visibilité](#chapitre-9--fonctions-portées-visibilité) | |
| **10** | [Le préprocesseur](#chapitre-10--le-préprocesseur) | Un langage dans le langage |
| **11** | [Manipuler les bits](#chapitre-11--manipuler-les-bits) | Indispensable pour le binaire |
| **12** | [La bibliothèque standard](#chapitre-12--la-bibliothèque-standard) | Petite, et c'est tout ce qu'il y a |
| **13** | [Les idiomes du C](#chapitre-13--les-idiomes-du-c) | Comment on écrit *vraiment* |
| **14** | [Le comportement indéfini](#chapitre-14--le-comportement-indéfini) | Le concept le plus important, et le plus ignoré |
| **15** | [Les outils](#chapitre-15--les-outils) | Compilateur, débogueur, sanitizers |

---
---

# Chapitre 0 — Le modèle mental

## Ce que le C est vraiment

Le C est **un assembleur portable**. Il a été conçu en 1972 pour écrire un système d'exploitation, et c'est ce qui explique tout le reste.

Quand tu écris du Python ou du JS, tu décris *ce que tu veux*. Une machine virtuelle s'occupe du comment : elle gère la mémoire, vérifie les accès, convertit les types, lève des exceptions.

Quand tu écris du C, **tu décris des opérations sur une machine**. Il n'y a pas de machine virtuelle. Ton code devient directement des instructions processeur.

## Les cinq conséquences

**1. Rien ne se passe dans ton dos.** Une ligne de C correspond à un petit nombre d'instructions machine prévisibles. Pas de copie implicite, pas d'allocation cachée, pas de ramasse-miettes qui se déclenche au mauvais moment. C'est pour ça que le C sert à écrire des noyaux et des moteurs temps réel.

**2. Il n'y a aucune vérification à l'exécution.** Pas de bornes de tableau vérifiées, pas de `nil pointer exception`, pas de `TypeError`. Si tu lis la case 1000 d'un tableau de 10 éléments, le C lit les octets qui s'y trouvent et te les rend. Ça peut être n'importe quoi.

**3. La mémoire est ton problème.** Tu demandes, tu libères. Si tu oublies, ça fuit. Si tu libères deux fois, ça casse.

**4. Le langage est minuscule.** ~32 mots-clés. Pas de classes, pas de modules, pas d'exceptions, pas de generics, pas de closures, pas de ramasse-miettes, pas de chaînes, pas de tableaux dynamiques, pas de tables de hachage. **Tout ça, tu l'écris ou tu l'importes.**

**5. Il est partout.** Linux, Windows, macOS, les navigateurs, Python lui-même, PostgreSQL, Redis, git, ffmpeg, OpenGL, ta box internet, le calculateur de ta voiture. Apprendre le C, c'est apprendre la couche sur laquelle tout repose.

## La bonne mentalité

> **Ce que tu écris est ce qui s'exécute. Ni plus, ni moins.**

Le C ne t'aide pas, mais il ne te trahit pas non plus. C'est un outil honnête : brutal et prévisible. La difficulté du C n'est pas sa syntaxe — tu l'auras apprise en deux semaines. C'est la **rigueur** qu'il exige : il ne rattrape aucune de tes erreurs.

## Le piège mental à désamorcer tout de suite

Tes langages t'ont habitué à une idée fausse pour le C : *« si mon programme tourne, c'est qu'il est correct »*.

En C, **un programme peut avoir un bug grave et fonctionner parfaitement pendant des mois.** Un dépassement de tableau qui n'écrase rien d'important. Un pointeur libéré deux fois qui tombe sur une case réutilisée. Ça marche, jusqu'au jour où on change une ligne ailleurs et où tout explose.

D'où la règle de survie n°1 : **faire confiance aux outils, pas à l'observation**. Les warnings, les sanitizers, le débogueur. On en parle au chapitre 15.

---
---

# Chapitre 1 — La chaîne de compilation

## Trois étapes, trois familles d'erreurs

```
  tes .c, tes .h
        │
        ▼
┌──────────────────┐
│ 1. PRÉPROCESSEUR │   manipulation de TEXTE
└────────┬─────────┘   #include, #define, #ifdef
         │
         ▼  un gros fichier de texte par .c
┌──────────────────┐
│ 2. COMPILATEUR   │   un .c → un .o, ISOLÉMENT
└────────┬─────────┘   analyse, vérification des types, génération de code
         │
         ▼  des .o (fichiers objets)
┌──────────────────┐
│ 3. LINKER        │   assemble tous les .o + les bibliothèques
└────────┬─────────┘   résout les symboles
         │
         ▼
    l'exécutable
```

## Étape 1 — le préprocesseur

Il ne comprend **rien** au C. Il fait de la manipulation de texte, avant que le compilateur ne voie quoi que ce soit.

```c
#include <stdio.h>     /* → « colle ici tout le contenu de stdio.h »     */
#define MAX 100        /* → remplace partout le texte MAX par le texte 100 */
#ifdef DEBUG           /* → garde ou jette ce bloc de texte               */
```

Vois-le de tes yeux :

```bash
clang -E mon_fichier.c | less
```

Tu verras des milliers de lignes. `#include <stdio.h>` a littéralement collé tout `stdio.h`, qui lui-même en inclut d'autres.

## Étape 2 — le compilateur

Il traite **chaque `.c` séparément**. On appelle ça une **unité de traduction** (*translation unit*).

> ⚠️ **Il ne sait RIEN des autres `.c`.** C'est le point que les gens venant de Go ou Python ratent systématiquement.

Quand il compile `main.c` et qu'il voit un appel à `grib_inspect()`, il n'a aucun moyen d'aller regarder dans `grib.c`. Il a besoin qu'on lui **promette** la signature. C'est le rôle du `.h`.

```c
/* grib.h — la PROMESSE (déclaration) */
bool grib_inspect(const char *path);

/* grib.c — la RÉALITÉ (définition) */
bool grib_inspect(const char *path) { ... }
```

Le compilateur produit un `.o` contenant du code machine, avec des **trous** : « ici il y a un appel à `grib_inspect`, je ne sais pas où elle est, le linker remplira ».

## Étape 3 — le linker

Il prend tous les `.o`, et pour chaque trou il cherche la définition correspondante. Il regarde aussi dans les bibliothèques (`-lcurl`, `-lm`…).

S'il ne trouve pas : `Undefined symbols`.
S'il trouve deux fois : `duplicate symbol`.

## Savoir diagnostiquer

| Message | Étape | Cause typique |
|---|---|---|
| `fatal error: 'x.h' file not found` | préprocesseur | chemin d'include manquant (`-I`) |
| `use of undeclared identifier 'X'` | compilateur | il manque un `#include` |
| `implicit declaration of function 'f'` | compilateur | il manque un `#include` |
| `incompatible pointer types` | compilateur | erreur de type |
| `Undefined symbols: "_f"` | **linker** | `.c` pas compilé, ou bibliothèque pas liée (`-lcurl`) |
| `duplicate symbol '_f'` | **linker** | la même fonction définie dans deux `.c` |

> **Le réflexe :** le message dit-il « undeclared » (compilateur, problème de `#include`) ou « undefined symbol » (linker, problème de `.c` ou de `-l`) ? Ces deux mots se ressemblent et désignent deux mondes différents.

## ⚠️ Les erreurs cascadent

Un seul symbole inconnu fait perdre le fil au compilateur, et tout ce qui suit devient suspect.

> **Lire la PREMIÈRE erreur. La corriger. Recompiler.** Jamais la liste entière.

```bash
clang -ferror-limit=1 ...      # clang
gcc  -fmax-errors=1   ...      # gcc
cmake --build build 2>&1 | head -20
```

---
---

# Chapitre 2 — La mémoire

**C'est le chapitre le plus important du cours.** Tout le C découle de là : les pointeurs, les chaînes, les tableaux, `malloc`, les bugs.

## Le modèle

La mémoire est **un immense tableau d'octets numérotés**. Le numéro d'un octet s'appelle son **adresse**. Sur une machine 64 bits, une adresse tient sur 8 octets.

```
adresse :  ... 0x7f1000  0x7f1001  0x7f1002  0x7f1003 ...
contenu :  ...   0x2A      0x00      0x00      0x00   ...
                 └──────── un int valant 42 ────────┘
```

C'est tout. Il n'y a pas d'« objets » en mémoire, pas de types, pas de métadonnées. **Juste des octets.** Le type n'existe qu'à la compilation : c'est l'information qui dit au compilateur *comment interpréter* ces octets.

## Les trois zones

```
  adresses hautes
┌─────────────────────┐
│       PILE          │  variables locales, arguments
│         │           │  grandit vers le bas
│         ▼           │  automatique : libéré à la sortie du scope
├─────────────────────┤
│                     │
│    (espace libre)   │
│                     │
├─────────────────────┤
│         ▲           │
│         │           │  grandit vers le haut
│        TAS          │  malloc / free
│      (heap)         │  manuel : libéré quand TU le dis
├─────────────────────┤
│  STATIQUE / GLOBAL  │  variables globales, static, littéraux
│                     │  vit toute la durée du programme
├─────────────────────┤
│   CODE (read-only)  │  tes instructions
└─────────────────────┘
  adresses basses
```

### 1. La pile (*stack*)

```c
void f(void) {
    int x = 42;          /* sur la pile */
    char buf[256];       /* sur la pile, 256 octets */
}                        /* ← x et buf DISPARAISSENT ici, automatiquement */
```

**Rapide** (un simple déplacement de pointeur) et **automatique**. Mais :

- **Taille limitée** — ~8 Mo par défaut sur macOS. Un `double grille[1000][1000]` = 8 Mo → *stack overflow*, ça plante.
- **Durée de vie liée au scope.** À la sortie de la fonction, c'est fini.

> ⚠️ **L'erreur classique absolue :**
> ```c
> int *mauvais(void) {
>     int x = 42;
>     return &x;      /* ⚠️ l'adresse de quelque chose qui meurt à la ligne suivante */
> }
> ```
> L'appelant reçoit une adresse vers une case de pile qui va être réutilisée. Ça « marche » parfois, ce qui est le pire des cas.

### 2. Le tas (*heap*)

```c
double *grille = malloc(1000 * 1000 * sizeof(double));   /* 8 Mo, sur le tas */
...
free(grille);                                            /* à TOI de le rendre */
```

**Grand** (limité par la RAM) et **à durée de vie libre** : ça survit à la sortie de la fonction, jusqu'à ton `free`. Mais plus lent, et **entièrement ta responsabilité**.

C'est ici que vivront tes grilles 3D et tes maillages.

### 3. La zone statique

```c
int compteur = 0;                      /* globale */
static int interne = 0;                /* globale privée au fichier */
const char *s = "bonjour";             /* le littéral "bonjour" est ici */

void f(void) {
    static int appels = 0;             /* survit entre les appels ! */
    appels++;
}
```

Vit du début à la fin du programme. Les **littéraux de chaîne** vivent ici, en **lecture seule** — d'où le `const char *`.

## Le tableau récapitulatif

| | Pile | Tas | Statique |
|---|---|---|---|
| Déclaration | `int x;` | `malloc()` | `static` / global |
| Taille | ~8 Mo | la RAM | fixe, à la compilation |
| Libération | automatique | **`free()` manuel** | fin du programme |
| Vitesse | très rapide | plus lent | — |
| Pour quoi | petites variables locales | grosses données, taille dynamique | constantes, état global |

## Le parallèle avec tes langages

En Go et en Python, cette distinction existe aussi — mais **le langage décide pour toi**. Go fait de l'*escape analysis* : si une variable locale s'échappe de la fonction, il la met automatiquement sur le tas. Python met tout sur le tas et compte les références.

**En C, c'est toi qui décides, et tu peux te tromper.** C'est exactement la contrepartie du contrôle.

---
---

# Chapitre 3 — Les types et leur représentation

## Les types de base

```c
char        /* 1 octet.  ⚠️ signé ou non : NON SPÉCIFIÉ par la norme */
short       /* ≥ 2 octets */
int         /* ≥ 2 octets par la norme, 4 en pratique partout       */
long        /* ≥ 4.  8 sur macOS/Linux, 4 sur Windows (!)           */
long long   /* ≥ 8 octets */
float       /* 4 octets, flottant simple précision                  */
double      /* 8 octets, flottant double précision                  */
void        /* « rien » / « type inconnu »                          */
```

Chacun a sa version `unsigned` (non signée, uniquement positive).

> ⚠️ **La norme ne fixe que des minimums.** `long` fait 8 octets sur ton Mac et 4 sur Windows. Du code qui suppose une taille n'est pas portable.

## La solution : `<stdint.h>`

```c
#include <stdint.h>

int8_t  int16_t  int32_t  int64_t      /* signés,     taille GARANTIE */
uint8_t uint16_t uint32_t uint64_t     /* non signés, taille GARANTIE */
```

> **Règle : dès que la taille compte — format binaire, protocole réseau, structure sur disque — utiliser `<stdint.h>`.**

## `size_t`

Entier **non signé** dimensionné pour contenir la taille de n'importe quel objet. 8 octets sur une machine 64 bits.

C'est le type de `sizeof`, de `strlen`, et le type correct pour **toute taille, tout compteur d'octets, tout indice de tableau**.

### ⚠️ Le piège du non-signé

Un non-signé ne peut pas être négatif. Sous zéro, il **repasse par le maximum** (*wraparound*) :

```c
size_t z = 0;
z - 1;                /* = 18 446 744 073 709 551 615, pas -1 */
```

```c
/* BOUCLE INFINIE : i >= 0 est toujours vrai */
for (size_t i = len - 1; i >= 0; i--) { ... }

/* et si len == 0, len - 1 = 18 quintillions → lecture hors mémoire */
```

> **Règle : ne jamais soustraire sur un non-signé sans vérifier que le résultat reste positif.**
> Pour boucler à l'envers, une forme sûre : `for (size_t i = len; i-- > 0; )`.

## `sizeof`

Opérateur (pas une fonction) évalué **à la compilation**. Rend un `size_t` = le nombre d'octets.

```c
sizeof(int)        /* 4  */
sizeof(double)     /* 8  */
sizeof(char *)     /* 8  — une adresse, quelle que soit la cible */
sizeof(x)          /* la taille du TYPE de x */
```

> **Idiome important :** écrire `malloc(n * sizeof(double))` et jamais `malloc(n * 8)`. Le premier reste correct si on change de type.

## Comment les octets représentent les nombres

### Les entiers non signés — en base 2

```
uint8_t  →  8 bits  →  0 à 255

  0000 0000 = 0
  0000 0001 = 1
  0000 1010 = 10
  1111 1111 = 255
```

### Les entiers signés — complément à deux

Le bit de poids fort porte le signe, mais pas naïvement :

```
int8_t  →  -128 à 127

  0000 0000 =    0
  0111 1111 =  127
  1000 0000 = -128      ← ça déborde ici
  1111 1111 =   -1
```

Pour obtenir `-x` : on inverse tous les bits et on ajoute 1. L'intérêt : l'addition fonctionne sans cas particulier pour le signe. C'est ce qu'utilisent tous les processeurs modernes.

### ⚠️ `char` est un piège

```c
char    c = 0xFF;   /* → -1   sur macOS arm64 (char signé) */
uint8_t o = 0xFF;   /* → 255  toujours                     */
```

Lire un octet `0xFF` dans un `char` signé donne `-1`. Tous les calculs partent en vrille, silencieusement.

> **Règle absolue : `uint8_t` pour les octets bruts. `char` uniquement pour du texte.**

## L'endianness (*boutisme*)

Un entier de plusieurs octets peut être rangé dans deux ordres. Prenons `0x12345678` (4 octets) :

```
BIG-ENDIAN     (poids fort d'abord)    12 34 56 78
LITTLE-ENDIAN  (poids faible d'abord)  78 56 34 12
```

- **Ton Mac** (arm64), et tous les x86 : **little-endian**
- **Les formats réseau et beaucoup de formats de fichiers** (dont GRIB) : **big-endian**. On dit *network byte order*.

> ⚠️ **Conséquence :** tu ne peux **pas** lire 4 octets d'un fichier big-endian directement dans un `uint32_t`. Il faut recomposer la valeur à la main, avec des décalages de bits. Voir le chapitre 11.

Pourquoi deux conventions ? Un hasard historique. Le nom vient des *Voyages de Gulliver*, où deux peuples se font la guerre pour savoir par quel bout casser un œuf à la coque.

## Conversions implicites

Le C convertit les types silencieusement, et ça mord.

```c
int    i = 3;
double d = i / 2;        /* = 1.0 !  3/2 est une division ENTIÈRE  */
double e = i / 2.0;      /* = 1.5    l'un des deux est flottant    */

int petit = 300;
uint8_t o = petit;       /* = 44 (300 % 256). Tronqué, en silence. */

unsigned u = 1;
if (u > -1) { ... }      /* FAUX ! -1 devient 4294967295           */
```

> `-Wall -Wextra -Wconversion` signale une bonne partie de ces cas. **Lis tes warnings.**

---
---

# Chapitre 4 — Les pointeurs

Tu connais les pointeurs en Go. En C, c'est la même idée, avec plus de pouvoir et aucune protection.

## La définition

**Un pointeur est une variable qui contient une adresse mémoire.** Point final.

```c
int  x = 42;      /* une case contenant 42                          */
int *p = &x;      /* une case contenant l'ADRESSE de la case de x    */
```

```
        p                      x
   ┌──────────┐           ┌────────┐
   │ 0x7f1000 │ ────────> │   42   │
   └──────────┘           └────────┘
   à l'adresse            à l'adresse
   0x7f0f00               0x7f1000
```

## Les deux opérateurs

```c
&x     /* « l'ADRESSE de x »                   → address-of  */
*p     /* « la VALEUR à l'adresse p »          → déréférencement */
```

Ils sont inverses l'un de l'autre : `*(&x)` vaut `x`.

## ⚠️ La confusion n°1 : les deux sens de `*`

```c
int *p;          /* DÉCLARATION : « p est un pointeur vers int »      */
*p = 42;         /* EXPRESSION  : « écris 42 À l'adresse p »          */
int y = *p;      /* EXPRESSION  : « lis la valeur À l'adresse p »     */
```

Dans une **déclaration**, `*` fait partie du type. Dans une **expression**, c'est un opérateur qui va chercher le contenu. Même symbole, sens opposés. C'est un défaut de conception du C, et on s'y habitue.

> **Astuce de lecture :** on écrit `int *p` et non `int* p` précisément pour cette raison. Le `*` appartient au **nom**, pas au type. Ça se voit dans :
> ```c
> int *a, b;      /* a est un POINTEUR, b est un simple int ! */
> ```

## Le type du pointeur compte

Un pointeur ne contient qu'une adresse — mais son **type** dit au compilateur deux choses :

1. **Comment interpréter** les octets à cette adresse
2. **De combien avancer** quand on fait `p + 1`

```c
uint8_t  *p8;    p8 + 1   /* avance de 1 octet  */
uint32_t *p32;   p32 + 1  /* avance de 4 octets */
double   *pd;    pd + 1   /* avance de 8 octets */
```

C'est l'**arithmétique de pointeurs** : `p + n` avance de `n * sizeof(*p)` octets. Très pratique, et très surprenant quand on l'oublie.

## `void *` — le pointeur sans type

```c
void *p;    /* une adresse, sans information sur ce qu'il y a au bout */
```

- On ne peut **pas** le déréférencer (`*p` est une erreur : déréférencer quoi ?)
- On ne peut **pas** faire d'arithmétique dessus (avancer de combien ?)
- Mais **il se convertit implicitement vers et depuis n'importe quel type de pointeur**

```c
FILE *f = userdata;         /* userdata est un void * — aucun cast nécessaire en C */
```

> C'est le mécanisme d'**effacement de type** du C. Il sert à écrire du code générique (`malloc` rend un `void *`, `memcpy` prend des `void *`) et à passer du contexte aux callbacks — le C n'ayant ni generics ni closures.
>
> ⚠️ C'est aussi un trou dans le système de types : le compilateur ne peut plus rien vérifier. Si tu te trompes de type en sortant d'un `void *`, **rien ne te le dira**.

## `NULL`

L'adresse « nulle », qui ne pointe sur rien. C'est la convention C pour « pas de valeur » / « échec ».

```c
FILE *f = fopen("x", "rb");
if (f == NULL) { /* échec */ }
```

⚠️ Déréférencer `NULL` plante le programme (*segmentation fault*). C'est d'ailleurs le cas **le plus gentil** : au moins ça s'arrête net.

> **Règle : toute fonction qui rend un pointeur peut rendre `NULL`. Toujours tester.**

## Les out-params

**Une fonction C ne peut retourner qu'une seule valeur.** Pas de `return a, b` comme en Go.

Pour rendre deux choses — par exemple « ça a marché » *et* « voilà le résultat » — on retourne l'une et on passe un **pointeur** pour l'autre.

```c
/* « rends-moi un bool, et écris le résultat dans *out » */
bool parse_int(const char *texte, int *out);

int valeur;
if (parse_int("42", &valeur)) {
    /* valeur contient 42 */
}
```

C'est l'idiome C pour le `(résultat, erreur)` de Go. Tu vas l'utiliser massivement.

## Les bugs classiques de pointeurs

| Bug | Nom | Description |
|---|---|---|
| Utiliser un pointeur vers de la mémoire libérée | *use-after-free* / *dangling pointer* | parfois ça marche — le pire des cas |
| `free` deux fois la même adresse | *double free* | corrompt les structures internes de l'allocateur |
| Lire un pointeur jamais initialisé | *uninitialized read* | contient une adresse au hasard |
| Oublier `free` | *memory leak* | la mémoire grandit indéfiniment |
| Écrire au-delà d'un tampon | *buffer overflow* | écrase les données voisines. **LA** faille historique |

> **Tous ces bugs sont détectables automatiquement.** Voir le chapitre 15 (`-fsanitize=address`). Ne cherche jamais un bug mémoire à l'œil nu — lance le sanitizer.

## `const` avec les pointeurs

L'emplacement du `const` change tout :

```c
const char *p;          /* p peut changer, *p est en lecture seule   */
char *const p;          /* p est fixe, *p est modifiable             */
const char *const p;    /* les deux sont fixes                       */
```

Dans 95 % des cas, c'est le premier qu'on veut : « je te prête ces données, tu les lis, tu n'y touches pas ».

> **Lis de droite à gauche en partant du nom :** `const char *p` → « p est un pointeur vers un char constant ».

---
---

# Chapitre 5 — Les tableaux

## Déclaration

```c
int t[5];                        /* 5 ints, non initialisés (contenu = n'importe quoi) */
int u[5] = {1, 2, 3, 4, 5};      /* initialisé                                         */
int v[5] = {0};                  /* tout à zéro — idiome courant                        */
int w[]  = {1, 2, 3};            /* taille déduite : 3                                  */
```

Un tableau est un **bloc contigu**. `int t[5]` = 20 octets d'affilée.

```
t[0]  t[1]  t[2]  t[3]  t[4]
┌────┬────┬────┬────┬────┐
│    │    │    │    │    │   20 octets contigus
└────┴────┴────┴────┴────┘
▲
t
```

## ⚠️ Aucune vérification de bornes

```c
int t[5];
t[10] = 42;        /* compile sans un mot. Écrit 20 octets après le tableau. */
t[-1] = 42;        /* pareil, 4 octets avant. */
```

Le C **ne vérifie jamais** les indices. Il calcule `adresse_de_t + 10 * 4` et écrit là. Tu viens d'écraser une autre variable, ou l'adresse de retour de la fonction.

> C'est la différence la plus brutale avec tous tes langages. En Python tu aurais un `IndexError`, en Go un `panic`. En C, **rien**.

## La relation tableau / pointeur

C'est le point le plus troublant du C. Deux faits à tenir ensemble :

### Fait 1 — un tableau n'est pas un pointeur

```c
int t[5];
sizeof(t);      /* 20 — la taille du TABLEAU */

int *p = t;
sizeof(p);      /* 8  — la taille d'une ADRESSE */
```

### Fait 2 — mais il se « dégrade » en pointeur presque partout

Dans presque tout contexte d'expression, **le nom d'un tableau devient automatiquement un pointeur vers son premier élément**. On appelle ça la *décomposition* (*array-to-pointer decay*).

```c
int t[5];
int *p = t;         /* équivaut à : int *p = &t[0]; */
```

Conséquence : l'indexation **est** de l'arithmétique de pointeurs.

```c
t[i]    /* est DÉFINI comme  *(t + i)  */
```

Ce qui donne cette curiosité, parfaitement légale :

```c
t[2] == *(t + 2) == *(2 + t) == 2[t]     /* oui, 2[t] compile et marche 😄 */
```

### ⚠️ La conséquence qui fait mal

**Quand tu passes un tableau à une fonction, la taille est perdue.**

```c
void f(int t[]) {       /* ⚠️ ceci est MENSONGER : c'est en réalité int *t */
    sizeof(t);          /* 8, pas 20 ! t est un pointeur ici. */
}
```

> **Règle d'or : un tableau se passe TOUJOURS en deux arguments — le pointeur et la taille.**
> ```c
> void f(const int *t, size_t n);
> ```
> C'est exactement ce que fait une *slice* Go, sauf qu'en Go les deux voyagent ensemble automatiquement. En C, tu les transportes à la main, et si tu te trompes de taille, c'est un dépassement.

## Tableaux multidimensionnels

```c
double g[10][20];       /* 10 lignes de 20 → 200 doubles, contigus */
g[3][7] = 1.0;
```

En mémoire, c'est **une seule séquence**, ligne par ligne (*row-major*) :

```
g[0][0] g[0][1] ... g[0][19] g[1][0] g[1][1] ... 
```

### L'indexation à plat

Pour une grille allouée dynamiquement, on gère l'indice soi-même :

```c
double *g = malloc(nx * ny * nz * sizeof(double));

/* accès à (x, y, z) */
g[(z * ny + y) * nx + x]
```

> ⚠️ **L'ordre des dimensions a un impact énorme sur les performances.** Le processeur charge la mémoire par **lignes de cache** (64 octets). Parcourir dans l'ordre mémoire est jusqu'à **10× plus rapide** que sauter.
>
> Autrement dit : la boucle sur l'indice **qui varie le plus vite en mémoire** doit être la boucle **la plus interne**. C'est le genre de chose qui comptera énormément pour tes grilles 3D et tes Marching Cubes.

---
---

# Chapitre 6 — Les chaînes de caractères

## Il n'y a pas de type chaîne

Une « chaîne » en C est **une convention** : un pointeur vers un tableau de `char`, terminé par un octet nul `'\0'`.

```
const char *s = "salut";

     s
     ▼
   ┌───┬───┬───┬───┬───┬────┐
   │ s │ a │ l │ u │ t │ \0 │   6 octets pour 5 caractères
   └───┴───┴───┴───┴───┴────┘
```

> Le `\0` s'appelle le **terminateur nul** (*NUL terminator*). C'est lui, et lui seul, qui dit où la chaîne s'arrête.

## Le parallèle Go, qui éclaire tout

```go
// Go : une string est une STRUCTURE de 16 octets
type string struct {
    ptr *byte   // 8 octets : où ça commence
    len int     // 8 octets : combien de temps ça dure
}
```

```c
// C : juste le pointeur. 8 octets. La longueur ? Tu la cherches.
const char *s;
```

**Le C a supprimé le champ `len`** et l'a remplacé par la convention du `\0`. Toutes les conséquences découlent de là :

| | Go | C |
|---|---|---|
| `len(s)` / `strlen(s)` | **O(1)** — lit un champ | **O(n)** — parcourt jusqu'au `\0` |
| Vérification des accès | possible | impossible |
| Sous-chaîne | gratuite (même tableau) | nécessite une copie |
| Peut contenir un `\0` | oui | **non** |

> ⚠️ **Ne jamais appeler `strlen` dans une condition de boucle** — tu re-parcours toute la chaîne à chaque tour, et ta boucle devient O(n²).

## Les trois formes à distinguer

```c
const char *p = "salut";    /* 1. pointeur vers un LITTÉRAL, en lecture seule */
char t[] = "salut";         /* 2. un TABLEAU de 6 octets, copie modifiable    */
char *m = malloc(6);        /* 3. un tampon sur le TAS, à libérer             */
```

```c
p[0] = 'S';     /* ⚠️ CRASH — un littéral vit en mémoire read-only */
t[0] = 'S';     /* ✅ OK — t est une copie sur la pile              */
```

> C'est pour ça qu'on écrit `const char *` pour les littéraux : le `const` est **nécessaire**, pas décoratif.

## Les fonctions de `<string.h>`

```c
size_t strlen(const char *s);                          /* longueur, O(n) */
int    strcmp(const char *a, const char *b);           /* 0 si égales    */
char  *strcpy(char *dst, const char *src);             /* ⚠️ DANGEREUX   */
char  *strncpy(char *dst, const char *src, size_t n);  /* borné           */
char  *strchr(const char *s, int c);                   /* trouve un char  */
char  *strstr(const char *h, const char *n);           /* trouve une sous-chaîne */

void  *memcpy(void *dst, const void *src, size_t n);   /* copie n octets  */
void  *memmove(void *dst, const void *src, size_t n);  /* idem, zones chevauchantes OK */
int    memcmp(const void *a, const void *b, size_t n); /* compare n octets */
void  *memset(void *s, int c, size_t n);               /* remplit de c     */
```

## ⚠️ Le danger des fonctions non bornées

```c
char buf[10];
strcpy(buf, "une chaîne beaucoup trop longue");   /* ⚠️ écrit 32 octets dans 10 */
```

`strcpy` **ne connaît pas la taille de la destination**. Elle copie jusqu'au `\0` de la source, où que ça la mène. Elle vient d'écraser 22 octets au-delà de `buf` : d'autres variables, l'adresse de retour de la fonction.

C'est le **buffer overflow**, la faille de sécurité la plus exploitée de l'histoire de l'informatique.

> ## 🔴 Dogme
> **`snprintf`, jamais `sprintf`. `strncpy`/`snprintf`, jamais `strcpy`. `strncat`, jamais `strcat`.**
> Les versions sans `n` ne connaissent pas la taille de la destination.

## Construire une chaîne — `snprintf`

Pas de `+`. Il te faut un tampon que tu as toi-même dimensionné.

```c
char url[256];
snprintf(url, sizeof(url), "https://exemple.fr/data?niveau=%d", 850);
```

- 2e argument = **la taille totale du tampon**, terminateur inclus. `sizeof(url)` est l'idiome (il reste correct si on change la taille).
- Elle **tronque** au lieu de déborder, et garantit le `\0`.
- Elle **retourne** le nombre de caractères qui *auraient* été écrits → si c'est `>= sizeof(url)`, il y a eu troncature. À tester quand ça compte.

## ⚠️ En binaire, jamais de `str*`

Les fonctions `str*` s'arrêtent au premier `\0`. Un fichier binaire est **truffé** de `\0`.

```c
memcmp(header, "GRIB", 4) == 0      /* ✅ compare exactement 4 octets */
strcmp(header, "GRIB") == 0         /* ❌ s'arrêtera n'importe où     */
```

> `memcmp` retourne **`0` quand c'est identique**. Penser « différence nulle ».

---
---

# Chapitre 7 — La mémoire dynamique

Pas de ramasse-miettes. Tu demandes, tu rends.

## Les quatre fonctions — `<stdlib.h>`

```c
void *malloc(size_t taille);                   /* alloue, contenu NON initialisé */
void *calloc(size_t n, size_t taille);         /* alloue n*taille, mis à ZÉRO    */
void *realloc(void *p, size_t nouvelle);       /* redimensionne                  */
void  free(void *p);                           /* libère                         */
```

## `malloc`

```c
double *g = malloc(n * sizeof(double));
if (g == NULL) { /* plus de mémoire */ }
...
free(g);
g = NULL;        /* 💡 bonne habitude : évite un use-after-free plus tard */
```

- **Rend `NULL` en cas d'échec** → à tester (même si sur un Mac avec de la swap, ça n'arrive quasiment jamais).
- **Le contenu n'est pas initialisé** : c'est de la mémoire recyclée, elle contient des ordures. Lire avant d'écrire = comportement indéfini.
- Rend un `void *` → **aucun cast nécessaire en C** (contrairement à C++).
- **Idiome : `malloc(n * sizeof(*g))`** plutôt que de répéter le type. Reste correct si on change `double` en `float`.

## `calloc` — alloue et met à zéro

```c
double *g = calloc(n, sizeof(double));     /* tout à 0.0 */
```

Pour une grille numérique, c'est souvent ce que tu veux. En bonus, `calloc` vérifie le débordement de `n * taille`, ce que `malloc(n * taille)` ne fait pas.

## `realloc` — le cœur du tableau dynamique

```c
double *tmp = realloc(g, nouvelle_taille * sizeof(double));
if (tmp == NULL) { /* échec : g est TOUJOURS valide */ }
g = tmp;
```

> ⚠️ **Toujours passer par une variable temporaire.** Si tu écris `g = realloc(g, ...)` et que ça échoue, `realloc` rend `NULL`, tu écrases `g`, et **tu as perdu l'adresse de ton bloc** → fuite garantie.

⚠️ `realloc` peut **déplacer** le bloc. Tous les pointeurs qui visaient l'ancienne adresse deviennent pendouillants.

## `free`

```c
free(p);        /* rend le bloc à l'allocateur */
```

- **`free(NULL)` est explicitement autorisé** et sans effet → pas besoin de tester avant.
- ⚠️ **Double `free`** = corruption de l'allocateur.
- ⚠️ `free` ne met **pas** ton pointeur à `NULL`. Il reste pendouillant. D'où l'habitude `free(p); p = NULL;`

## Les règles de survie

> **1. À chaque `malloc` correspond exactement un `free`.**
>
> **2. Un bloc a un seul PROPRIÉTAIRE** — la fonction ou la structure responsable de le libérer. Écris-le dans un commentaire : *« l'appelant doit free() le résultat »*.
>
> **3. Documente la propriété dans le nom.** `grid_create` / `grid_destroy` qui vont par paire, c'est explicite et dur à oublier.

C'est la discipline que C++ automatise avec RAII et que Go automatise avec son GC. En C, c'est une convention humaine — et c'est pour ça qu'on utilise des outils pour la vérifier (chapitre 15).

## Écrire son tableau dynamique

Tu n'as pas `std::vector`. Tu vas l'écrire. Le principe (c'est exactement une slice Go, de l'intérieur) :

```c
typedef struct {
    double *data;      /* le bloc                        */
    size_t  len;       /* nombre d'éléments utilisés      */
    size_t  cap;       /* nombre d'éléments alloués       */
} vec;
```

Et à l'ajout : si `len == cap`, on `realloc` avec une capacité **doublée**.

> **Pourquoi doubler, et pas ajouter 1 ?** Avec un doublement, le coût des réallocations est *amorti* : N ajouts coûtent O(N) au total. En ajoutant 1 à chaque fois, chaque `realloc` recopie tout → O(N²).
>
> C'est exactement la stratégie de `append` en Go et de `std::vector` en C++. Tu vas en avoir besoin pour tes Marching Cubes : tu ne sais pas combien de triangles tu vas générer.

---
---

# Chapitre 8 — Structurer ses données

## `struct` — regrouper

```c
struct Point {
    double x;
    double y;
    double z;
};

struct Point p;         /* ⚠️ en C, le mot-clé "struct" est OBLIGATOIRE */
p.x = 1.0;
```

## `typedef` — nommer un type

Écrire `struct Point` partout est pénible. D'où l'idiome universel :

```c
typedef struct {
    double x, y, z;
} Vec3;

Vec3 v;                 /* ✅ plus besoin de "struct" */
v.x = 1.0;
```

Et pour une structure qui se référence elle-même (liste, arbre), il faut la nommer :

```c
typedef struct Node {
    int          valeur;
    struct Node *suivant;      /* le nom est nécessaire ICI */
} Node;
```

## Accès : `.` ou `->`

```c
Vec3  v;    v.x = 1.0;          /* via une valeur   */
Vec3 *p;    p->x = 1.0;         /* via un pointeur — équivaut à (*p).x */
```

## Passer une struct à une fonction

```c
void f(Vec3 v);              /* par VALEUR : copie les 24 octets        */
void f(const Vec3 *v);       /* par pointeur const : 8 octets, lecture  */
void f(Vec3 *v);             /* par pointeur : modifiable par la fonction */
```

> **Règle pratique : petite struct (≤ 16 octets) → par valeur, c'est plus simple et souvent plus rapide. Grosse struct → par pointeur `const`.**

## ⚠️ Le padding (*rembourrage*)

Le compilateur insère des octets vides pour aligner les champs sur des adresses multiples de leur taille — le processeur lit beaucoup plus vite comme ça.

```c
struct A {       struct B {
    char  c;         double d;    /* 8 */
    double d;        char   c;    /* 1 */
    char  e;         char   e;    /* 1 */
};               };               /* + 6 de padding */

sizeof(struct A) = 24            sizeof(struct B) = 16
```

Même contenu, 50 % de différence. **Déclarer les champs du plus grand au plus petit** réduit le padding.

> ⚠️⚠️ **Conséquence critique pour toi : on ne peut JAMAIS lire une struct directement depuis un fichier binaire.**
> ```c
> struct GribHeader h;
> fread(&h, sizeof(h), 1, f);     /* ❌ NE FAIS JAMAIS ÇA */
> ```
> Le padding, l'endianness et les tailles de types varient selon la plateforme et le compilateur. **Il faut toujours lire octet par octet et recomposer les champs à la main.**

## `enum` — des constantes nommées

```c
typedef enum {
    LEVEL_SURFACE  = 1,
    LEVEL_ISOBARIC = 100,
    LEVEL_HEIGHT   = 103
} LevelType;
```

Sans valeur explicite, ça démarre à 0 et incrémente. C'est juste un `int` déguisé — le C ne vérifie pas qu'une variable `enum` contient une valeur valide.

> Bien plus lisible que des nombres nus dans le code. Les *magic numbers* sont l'ennemi.

## `union` — plusieurs vues du même espace

Tous les membres partagent **la même mémoire**. Taille = celle du plus grand.

```c
union Bits {
    uint32_t entier;
    uint8_t  octets[4];
    float    flottant;
};
```

Utile pour réinterpréter des octets — par exemple décoder un flottant IEEE-754 depuis 4 octets bruts, ce qui t'arrivera avec le GRIB.

> ⚠️ Écrire dans un membre et lire dans un autre est techniquement du *type punning*. En C c'est **autorisé** via une union (contrairement à C++). Mais c'est un outil à manier en connaissance de cause.

## Membres de taille binaire — *bitfields*

```c
struct Flags {
    unsigned actif    : 1;     /* 1 bit  */
    unsigned priorite : 3;     /* 3 bits */
};
```

Compact, mais l'ordre et l'alignement ne sont **pas portables**. À éviter pour parser des formats binaires — fais-le à la main avec des masques (chapitre 11).

---
---

# Chapitre 9 — Fonctions, portées, visibilité

## Déclarer, définir, appeler

```c
/* DÉCLARATION (prototype) : des TYPES, finit par ;  → dans le .h */
bool grib_inspect(const char *path);

/* DÉFINITION : le corps                             → dans le .c */
bool grib_inspect(const char *path) { ... }

/* APPEL : des VALEURS, le résultat se stocke */
bool ok = grib_inspect("model.grib2");
```

| | Déclaration | Appel |
|---|---|---|
| dans les parenthèses | des **types** | des **valeurs** |
| finit par | `;` | fait partie d'une expression |
| résultat | aucun | on le **stocke** |

> ⚠️ **Les parenthèses font l'appel.** `fclose;` nomme la fonction (son adresse) et ne fait rien. `fclose(f);` l'appelle.

## `void` dans les paramètres

```c
int main(void);     /* ✅ « prend ZÉRO argument »             */
int main();         /* ⚠️ « nombre d'arguments non spécifié » */
```

La seconde forme est un vestige du C d'avant 1989 qui désactive la vérification. **Toujours écrire `void`.**

## Visibilité (*linkage*)

```c
bool f(void) { ... }           /* EXTERNE : visible par les autres .c       */
static bool g(void) { ... }    /* INTERNE : visible seulement dans ce .c    */
```

> `static` sur une fonction = l'équivalent exact de la **minuscule en Go**. C'est le mécanisme d'encapsulation du C : tout ce qui n'est pas dans le `.h` et qui est `static` est privé.

> **Bonne pratique : `static` par défaut.** N'expose dans le `.h` que ce qui doit être public. Ça réduit les collisions de noms et ça documente l'interface.

## ⚠️ `static` a trois sens différents

| Contexte | Sens |
|---|---|
| fonction au niveau fichier | visible seulement dans ce `.c` |
| variable au niveau fichier | idem |
| **variable locale** | **survit entre les appels** (vit en zone statique) |

```c
void f(void) {
    static int appels = 0;      /* initialisé UNE fois, conserve sa valeur */
    appels++;
}
```

C'est une bizarrerie historique du C. Trois sens, un mot-clé.

## `extern`

Sur une **fonction**, c'est le défaut → **on ne l'écrit jamais**.
Sur une **variable globale** partagée entre fichiers, c'est indispensable : `extern` dans le `.h`, la vraie définition dans un seul `.c`.

## ⚠️ Pas de fonctions imbriquées

Une définition de fonction ne peut exister qu'au **niveau fichier**. Pas de fonctions dans les fonctions, pas d'anonymes, pas de closures, pas de lambdas.

Si tu veux du comportement paramétrable, tu utilises un **pointeur de fonction**.

## Les pointeurs de fonction

Une fonction a une adresse, donc on peut la stocker.

```c
/* « cb est un pointeur vers une fonction prenant (char*, size_t) et rendant size_t » */
size_t (*cb)(char *, size_t);

cb = write_callback;         /* on assigne : juste le nom, sans parenthèses */
size_t n = cb(buf, 10);      /* on appelle                                  */
```

La syntaxe est horrible. Un `typedef` la rend lisible :

```c
typedef size_t (*WriteFn)(char *, size_t);
WriteFn cb = write_callback;
```

C'est le mécanisme des callbacks (`CURLOPT_WRITEFUNCTION`) et du polymorphisme en C.

> ⚠️ **Un pointeur de fonction n'a pas d'environnement capturé.** Une closure JS emporte ses variables ; un pointeur de fonction C n'emporte rien. D'où l'invariable `void *userdata` en argument supplémentaire : c'est **le contexte transporté à la main**.

## Paramètre non utilisé

```c
(void)path;     /* « je sais, tais-toi » — silence -Wunused-parameter */
```

## Nommage

- fonctions = **verbes** en `snake_case` : `grib_inspect`, `vec3_add`
- pas de namespaces → on **préfixe par module** : `grib_`, `cloud_`, `vec3_`
- types = noms communs, souvent suffixés `_t`
- macros et constantes en `MAJUSCULES`

---
---

# Chapitre 10 — Le préprocesseur

Un langage dans le langage, qui ne manipule que du **texte**.

## `#include`

```c
#include <stdio.h>      /* chevrons : chercher dans les répertoires système */
#include "grib.h"       /* guillemets : chercher d'abord à côté du fichier  */
```

C'est un **copier-coller textuel**, rien de plus.

### Include guard

```c
#ifndef GRIB_H
#define GRIB_H
/* ... */
#endif
```

Indispensable : sans ça, deux inclusions = double définition = erreur de compilation.
Alternative moderne, non standard mais universellement supportée : `#pragma once`.

### Règle d'or

> **Un header n'inclut que ce dont ses propres déclarations ont besoin.**
> Si le `.h` déclare `bool f(void)` → `<stdbool.h>` suffit. S'il ne mentionne pas `FILE *`, il n'a rien à faire d'un `<stdio.h>` — c'est le `.c` qui en a besoin.

## `#define`

```c
#define MAX_NIVEAUX 64
#define PI 3.14159265358979323846
```

Du **remplacement de texte**. Pas une variable, pas de type.

### Les macros-fonctions et leurs pièges

```c
#define CARRE(x) x * x               /* ⚠️ CASSÉ */
CARRE(1 + 2)    /* → 1 + 2 * 1 + 2  =  5, pas 9 ! */

#define CARRE(x) ((x) * (x))        /* ✅ parenthéser TOUT */
CARRE(1 + 2)    /* → ((1 + 2) * (1 + 2)) = 9 */
```

```c
#define MAX(a, b) ((a) > (b) ? (a) : (b))
MAX(i++, j)     /* ⚠️ i++ est évalué DEUX fois */
```

> **Préfère `const` et les fonctions `static inline` aux macros.** Elles sont typées, déboguables et n'ont aucun de ces pièges. Les macros sont utiles pour ce que le langage ne peut pas faire : la compilation conditionnelle, le *stringification*, les gardes d'include.

## Compilation conditionnelle

```c
#ifdef DEBUG
    printf("valeur = %d\n", x);
#endif

#if defined(__APPLE__)
    /* code spécifique macOS */
#elif defined(__linux__)
    /* code spécifique Linux */
#endif
```

Le bloc non retenu **n'existe même pas** pour le compilateur : il est supprimé du texte.

## Macros prédéfinies utiles

```c
__FILE__      /* le nom du fichier courant   */
__LINE__      /* le numéro de ligne          */
__func__      /* le nom de la fonction       */
```

```c
#define LOG(msg) fprintf(stderr, "[%s:%d] %s\n", __FILE__, __LINE__, msg)
```

## `assert`

```c
#include <assert.h>
assert(n > 0);      /* si faux : affiche le fichier/ligne et avorte */
```

Désactivé par `-DNDEBUG` en production. Sert à vérifier les **invariants** — ce qui ne devrait *jamais* arriver si ton code est correct. Pas pour valider les entrées utilisateur (ça, c'est un `if`).

---
---

# Chapitre 11 — Manipuler les bits

Le chapitre dont tu as besoin pour tout format binaire.

## Les six opérateurs

```c
a & b     /* ET      : 1 si les DEUX bits sont à 1          */
a | b     /* OU      : 1 si AU MOINS UN bit est à 1         */
a ^ b     /* XOR     : 1 si les bits DIFFÈRENT              */
~a        /* NON     : inverse tous les bits                */
a << n    /* décalage à GAUCHE  de n bits (×  2^n)          */
a >> n    /* décalage à DROITE  de n bits (÷  2^n)          */
```

```
  0b1100  &  0b1010  =  0b1000
  0b1100  |  0b1010  =  0b1110
  0b1100  ^  0b1010  =  0b0110
         ~  0b1100   =  0b0011   (sur 4 bits)

  0b0011  <<  2      =  0b1100   (3 × 4 = 12)
  0b1100  >>  2      =  0b0011   (12 ÷ 4 = 3)
```

> ⚠️ Ne confonds pas `&` (bits) et `&&` (logique), `|` (bits) et `||` (logique). C'est une erreur qui compile.

## Les quatre recettes fondamentales

```c
/* LIRE le bit n          */  (valeur >> n) & 1
/* METTRE à 1 le bit n    */  valeur |=  (1u << n)
/* METTRE à 0 le bit n    */  valeur &= ~(1u << n)
/* INVERSER le bit n      */  valeur ^=  (1u << n)
```

## Les masques

Un **masque** isole les bits qui t'intéressent.

```c
uint8_t octet = 0b11010110;

octet & 0x0F        /* garde les 4 bits de poids FAIBLE → 0b0110 */
(octet >> 4) & 0x0F /* garde les 4 bits de poids FORT   → 0b1101 */
```

**Fabriquer un masque de n bits à 1 :**

```c
(1u << n) - 1       /* n=4 → 16 - 1 = 15 = 0b1111 */
```

Cette formule est à connaître par cœur : elle sert à extraire un champ de n bits.

## Extraire un champ de n bits

```c
/* « donne-moi les n bits situés à la position p » */
(valeur >> p) & ((1u << n) - 1)
```

C'est l'opération de base pour décoder n'importe quel format compacté — y compris les valeurs GRIB2 empaquetées sur un nombre arbitraire de bits.

## ⚠️ Les pièges du décalage

```c
1 << 31          /* ⚠️ COMPORTEMENT INDÉFINI : 1 est un int signé */
1u << 31         /* ✅ utilise un littéral non signé              */
1ull << 40       /* ✅ pour dépasser 32 bits                       */

x << 32          /* ⚠️ si x est sur 32 bits : INDÉFINI            */
                 /*    décaler de ≥ la largeur du type est interdit */

int n = -8;
n >> 1           /* ⚠️ sur un SIGNÉ, le comportement du décalage   */
                 /*    droit dépend de l'implémentation            */
```

> **Règles :** décale toujours sur des types **non signés** (`1u`, `1ull`, `uint32_t`), et jamais d'un nombre de bits ≥ la largeur du type.

## Lire un entier big-endian — l'opération clé

Ton fichier GRIB stocke les entiers **octet de poids fort d'abord**. Ta machine est little-endian. Tu ne peux donc **pas** faire un `fread` direct dans un `uint32_t`.

Il faut recomposer. Le principe, sur 2 octets :

```
octets lus :   0x07   0xEA
               ↓      ↓
               poids  poids
               fort   faible

valeur = (0x07 << 8) | 0xEA
       = (7 × 256) + 234
       = 2026
```

Généralisation : **chaque octet supplémentaire décale le total de 8 bits de plus vers la gauche**, et on ajoute le nouvel octet avec un `|`.

```c
/* forme générale, pour n octets, big-endian */
uint64_t v = 0;
for (size_t i = 0; i < n; i++) {
    v = (v << 8) | tampon[i];
}
```

> Relis cette boucle jusqu'à ce qu'elle soit évidente. `v << 8` pousse tout ce qu'on a déjà d'un octet vers la gauche, et `| tampon[i]` glisse le nouvel octet dans les 8 bits libérés à droite. C'est exactement comment on lit un nombre en base 256.

⚠️ Veille au **type du tampon** : si c'est un `char` signé, un octet ≥ 0x80 devient négatif et le `|` ruine le résultat. **`uint8_t` obligatoire.**

## Pourquoi pas une conversion automatique ?

Il existe `htonl`/`ntohl` (*host to network long*) dans `<arpa/inet.h>`, mais elles ne gèrent que 16 et 32 bits, et elles ne t'aident pas pour les champs de 3, 5 ou 8 octets fréquents dans les formats météo. La boucle manuelle est plus simple et plus générale.

---
---

# Chapitre 12 — La bibliothèque standard

Elle est **petite**. C'est tout ce que le C te donne — le reste, tu l'écris ou tu l'importes.

| Header | Contenu |
|---|---|
| `<stdio.h>` | fichiers et E/S : `printf`, `fopen`, `fread`, `fwrite`, `fseek` |
| `<stdlib.h>` | `malloc`, `free`, `exit`, `atoi`, `strtod`, `qsort`, `abs`, `rand` |
| `<string.h>` | `strlen`, `strcmp`, `memcpy`, `memcmp`, `memset` |
| `<math.h>` | `sqrt`, `sin`, `cos`, `pow`, `exp`, `log`, `floor`, `fabs` |
| `<stdint.h>` | `uint8_t`, `int32_t`… |
| `<stdbool.h>` | `bool`, `true`, `false` |
| `<stddef.h>` | `size_t`, `NULL`, `ptrdiff_t` |
| `<assert.h>` | `assert` |
| `<errno.h>` | `errno` et les codes d'erreur système |
| `<limits.h>` | `INT_MAX`, `CHAR_BIT`… |
| `<float.h>` | `DBL_MAX`, `DBL_EPSILON`… |
| `<time.h>` | `time`, `clock`, `strftime` |

> ⚠️ `<math.h>` nécessite souvent de lier la bibliothèque mathématique : **`-lm`** à la compilation. Sinon tu auras une erreur du *linker* sur `sqrt`. (Sur macOS c'est en général automatique.)

## Les fichiers — `<stdio.h>`

```c
FILE *f = fopen(chemin, mode);     /* "rb" lecture binaire, "wb" écriture binaire */
if (f == NULL) { ... }             /* TOUJOURS tester */

size_t fread (void *dst, size_t taille_elem, size_t nb, FILE *f);
size_t fwrite(const void *src, size_t taille_elem, size_t nb, FILE *f);

int fseek(FILE *f, long offset, int origine);   /* SEEK_SET / SEEK_CUR / SEEK_END */
long ftell(FILE *f);                            /* position courante               */
int  feof(FILE *f);                             /* fin de fichier atteinte ?       */
int  fclose(FILE *f);
int  remove(const char *chemin);
```

`FILE *` est un **type opaque** : on ne sait pas ce qu'il y a dedans, on ne peut que le passer aux fonctions de la lib. (`CURL *` aussi.)

- le **`b`** du mode = binaire. Inutile sur macOS/Linux, **vital** sur Windows (sinon `\n` → `\r\n` et un fichier binaire est corrompu). À mettre par réflexe.
- `fread`/`fwrite` retournent le **nombre d'ÉLÉMENTS** traités, pas d'octets. **Idiome : avec `taille_elem = 1`, éléments == octets.**
- ⚠️ `fread` peut lire **moins** que demandé. Toujours comparer le retour à ce qu'on attendait.
- ⚠️ `fclose(NULL)` plante. `free(NULL)` est autorisé.

## `printf` et ses spécificateurs

```c
printf("texte %s et %d\n", chaine, nombre);
fprintf(stderr, "erreur : %s\n", message);
snprintf(buf, sizeof(buf), "format %d", x);
perror("mon message");                        /* message + raison système (errno) */
```

| Type | Format | | Type | Format |
|---|---|---|---|---|
| `int` | `%d` | | `char *` | `%s` |
| `unsigned` | `%u` | | `char` | `%c` |
| `long` | `%ld` | | pointeur | `%p` |
| `long long` | `%lld` | | hexa | `%x` / `%02x` |
| `size_t` | `%zu` | | un vrai `%` | `%%` |
| `double` | `%f` / `%g` / `%e` | | | |

Largeur et précision : `%5d` (aligné à droite sur 5), `%-5d` (à gauche), `%.3f` (3 décimales), `%08.3f`.

> ⚠️ `%l` tout seul n'existe pas : `l` est un **modificateur de longueur**, il doit être suivi de la lettre du type.
>
> ⚠️ Un mauvais spécificateur **compile** et produit un comportement indéfini. **C'est `-Wall` qui te sauve** : il vérifie la cohérence entre le format et les arguments. C'est l'une des meilleures raisons de l'activer.

### ⚠️ Le tamponnage

`stderr` n'est **pas** tamponné (sort immédiatement). `stdout` l'est (il attend un lot, sauf sur un terminal interactif où il sort à chaque `\n`). Quand la sortie est redirigée, **les deux se désynchronisent** et les messages apparaissent dans le désordre. Ce n'est pas un bug de ton programme.

`fflush(stdout)` force la vidange.

## `errno` et `perror`

Beaucoup de fonctions système positionnent la variable globale `errno` en cas d'échec.

```c
#include <errno.h>
#include <string.h>

FILE *f = fopen(chemin, "rb");
if (f == NULL) {
    perror("ouverture");                            /* "ouverture: No such file..." */
    fprintf(stderr, "%s\n", strerror(errno));       /* juste le texte               */
}
```

> ⚠️ `perror` n'a de sens qu'**immédiatement après** un appel qui positionne `errno`. Après une fonction qui n'y touche pas (`curl_easy_init`, par exemple), tu afficherais un message d'erreur sans aucun rapport.

## Convertir du texte en nombre

```c
int    atoi(const char *s);                         /* ⚠️ pas de détection d'erreur */
long   strtol(const char *s, char **fin, int base); /* ✅ robuste                    */
double strtod(const char *s, char **fin);           /* ✅ robuste                    */
```

> **Préfère les `strto*`.** `atoi("abc")` rend `0` sans rien signaler — impossible de distinguer une erreur d'un vrai zéro. Les `strto*` te rendent via `fin` où elles se sont arrêtées.

## `qsort` — le tri générique

```c
int compare(const void *a, const void *b) {
    double x = *(const double *)a;
    double y = *(const double *)b;
    return (x > y) - (x < y);       /* -1, 0 ou 1 — sans soustraction, pour éviter les débordements */
}

qsort(tableau, n, sizeof(double), compare);
```

Note comment le C fait du **générique sans generics** : `void *` + la taille d'un élément + un pointeur de fonction de comparaison. C'est le patron du polymorphisme en C.

---
---

# Chapitre 13 — Les idiomes du C

La syntaxe s'apprend en deux semaines. Les idiomes, c'est ce qui distingue du C lisible d'un champ de mines.

## 1. Tester chaque retour

```c
FILE *f = fopen(path, "rb");
if (f == NULL) { ... }
```

En C, **rien n'est automatique**. Pas d'exception qui remonte, pas de `panic`. Une fonction qui peut échouer te le dit par son retour, et si tu ne regardes pas, tu continues avec des données invalides.

Les conventions de retour :

| Forme | Échec signalé par |
|---|---|
| rend un pointeur | `NULL` |
| rend un `int` « code » | `< 0` (souvent `-1`), ou `!= 0` |
| rend un `bool` | `false` |
| rend un compte | une valeur < ce qui était demandé |

## 2. Les guard clauses

```c
if (erreur_A) { ...; goto cleanup; }
if (erreur_B) { ...; goto cleanup; }

/* si on arrive ici, A et B sont OK. Garanti. */
ok = true;
```

Chaque erreur sort immédiatement, donc **le chemin du succès reste à l'indentation zéro** et se lit de haut en bas comme une recette.

La forme imbriquée (`if/else` emboîtés) s'appelle l'*arrow anti-pattern* : chaque test ajoute un niveau et enterre le code utile tout au fond à droite.

C'est l'*early return* de JS, ou le `if err != nil { return err }` de Go.

## 3. `goto cleanup` — **LE** pattern de gestion d'erreur

Présent dans le noyau Linux, SQLite, libcurl, FFmpeg.

```c
bool traiter(const char *chemin)
{
    FILE   *f   = NULL;      /* 1. ressources EN HAUT, à NULL */
    double *buf = NULL;
    bool    ok  = false;     /* 2. on suppose l'ÉCHEC */

    f = fopen(chemin, "rb");
    if (f == NULL) { perror("fopen"); goto cleanup; }          /* 3. saut */

    buf = malloc(1024 * sizeof(double));
    if (buf == NULL) { fprintf(stderr, "oom\n"); goto cleanup; }

    /* ... le travail utile ... */

    ok = true;               /* 4. true UNIQUEMENT à la dernière ligne */

cleanup:                     /* 5. une étiquette */
    free(buf);                            /* ordre INVERSE de l'acquisition */
    if (f != NULL) fclose(f);
    return ok;               /* un SEUL return */
}
```

**Les cinq règles :**

1. **Ressources déclarées en haut, à `NULL`** — pour que `cleanup` puisse toujours les examiner, même si on saute avant leur acquisition.
2. **`ok = false` par défaut**, `true` seulement à la dernière ligne du travail → impossible d'oublier de signaler une erreur.
3. Tous les chemins d'erreur convergent vers l'étiquette.
4. **Libérer dans l'ordre inverse** de l'acquisition.
5. Attention aux fonctions qui n'acceptent pas `NULL` (`fclose` non, `free` oui).

> ⚠️⚠️ **Une étiquette ne bloque pas l'exécution.** C'est juste un **nom donné à une position**. Le programme coule de haut en bas et **tombe dedans** tout seul.
>
> ```
>                   ┌──────────────────────┐
> if (erreur A) ────┤                      │
> if (erreur B) ────┤  goto cleanup (SAUT) │
> if (erreur C) ────┤                      │
>                   └──────────┬───────────┘
>                              │
> ok = true;   ────────────────┤  on TOMBE dedans, PAS de saut
>                              ▼
> cleanup:
>     libérer
>     return ok;
> ```
>
> **Sur le chemin du succès, il n'y a aucun `goto` à écrire.** Un `goto cleanup` juste avant `ok = true` sauterait par-dessus.

> **Et `goto` n'est pas « sale » ici.** La règle « jamais de `goto` » visait les sauts **en arrière** et ceux qui entrelacent la logique. Ici c'est un saut **en avant vers un point de sortie unique** : ça *réduit* la complexité. C'est l'unique usage du `goto` universellement accepté en C.

> **Ce que ça remplace :** en C++, le **destructeur** est appelé automatiquement à la sortie du scope, sur tous les chemins. Ce pattern s'appelle **RAII** et n'existe pas en C. Chaque `return` est une porte à garder soi-même, d'où le `goto cleanup`.

## 4. Tableau = pointeur + taille

```c
void traiter(const double *data, size_t n);
```

**Toujours les deux.** C'est une slice Go démontée en ses deux morceaux.

## 5. Les out-params

```c
bool parse(const char *texte, double *out);
```

Le retour porte le succès, le pointeur porte la valeur. L'équivalent C du `(valeur, err)` de Go.

## 6. `create` / `destroy` par paires

```c
Grid *grid_create(size_t nx, size_t ny, size_t nz);   /* l'appelant doit grid_destroy() */
void  grid_destroy(Grid *g);
```

Nommer les paires rend la propriété évidente et les fuites visibles à la relecture.

## 7. `sizeof(*p)` plutôt que le type

```c
double *g = malloc(n * sizeof(*g));     /* reste correct si on passe en float */
```

## 8. `static` par défaut

Tout ce qui n'a pas besoin d'être public est `static`. Le `.h` est ton interface, et elle doit être minuscule.

## 9. Pas de *magic numbers*

```c
if (type == 100) { ... }                  /* ❌ 100 c'est quoi ? */
if (type == LEVEL_ISOBARIC) { ... }       /* ✅ */
```

## 10. Initialiser à la déclaration

```c
int    n = 0;
double *p = NULL;
Vec3   v = {0};        /* struct entièrement à zéro */
```

Une variable non initialisée en C contient des **ordures**, et la lire est un comportement indéfini. `= {0}` sur une struct ou un tableau met tout à zéro — idiome très courant.

---
---

# Chapitre 14 — Le comportement indéfini

**Le concept le plus important du C, et le plus ignoré.** Si tu ne retiens qu'un chapitre, prends celui-là.

## La définition

La norme C définit ce que ton programme doit faire pour un ensemble de situations. Pour tout le reste, elle dit : **comportement indéfini** (*undefined behavior*, « UB »).

**« Indéfini » ne veut pas dire « ça plante » ni « ça donne une valeur bizarre ».** Ça veut dire : *la norme ne garantit absolument rien*, et le compilateur est **autorisé à faire n'importe quoi**.

## Pourquoi c'est pire que ça n'en a l'air

Le compilateur **suppose que ton programme ne contient pas d'UB**, et il optimise en conséquence. Il peut donc **supprimer ton code**.

```c
int f(int *p) {
    int x = *p;              /* déréférencement : donc p n'est pas NULL */
    if (p == NULL) return 0; /* ⚠️ le compilateur SUPPRIME ce test :     */
    return x;                /*    si p était NULL, la ligne 2 serait   */
}                            /*    de l'UB, donc p n'est pas NULL.      */
```

C'est logique de son point de vue, et dévastateur du tien. Un UB peut faire disparaître une vérification de sécurité située **ailleurs** dans la fonction.

## Et le pire symptôme : ça marche

Un UB peut produire exactement le résultat attendu pendant des mois. Puis tu changes le niveau d'optimisation, tu ajoutes une variable, tu mets à jour le compilateur — et tout explose, à un endroit sans rapport.

> **Corollaire : en C, « ça marche » n'est PAS une preuve de correction.**

## Le catalogue des UB courants

| UB | Exemple |
|---|---|
| Accès hors bornes | `int t[5]; t[5] = 0;` |
| Déréférencer `NULL` | `int *p = NULL; *p = 1;` |
| Use-after-free | `free(p); *p = 1;` |
| Double `free` | `free(p); free(p);` |
| Lire une variable non initialisée | `int x; printf("%d", x);` |
| Débordement d'entier **signé** | `INT_MAX + 1` |
| Décalage ≥ la largeur du type | `uint32_t x; x << 32` |
| Décalage d'un négatif | `1 << 31` sur un `int` |
| Mauvais spécificateur `printf` | `printf("%d", 3.14)` |
| Division par zéro | `x / 0` |
| Modifier un littéral de chaîne | `char *s = "a"; s[0] = 'b';` |
| Violation du *strict aliasing* | lire un `float` à travers un `int *` |
| `memcpy` sur des zones qui se chevauchent | utiliser `memmove` |

> 💡 **Noter l'asymétrie :** le débordement d'un entier **signé** est de l'UB, celui d'un **non signé** est parfaitement défini (il repasse par zéro). C'est contre-intuitif, et c'est une raison de plus d'utiliser des types non signés quand on fait de l'arithmétique sur des bits.

## Comment s'en protéger

Tu ne peux **pas** détecter l'UB à l'œil nu. Il faut des outils :

1. **`-Wall -Wextra`** — attrape une partie des cas à la compilation
2. **`-fsanitize=undefined`** (UBSan) — détecte l'UB **à l'exécution** et affiche le fichier, la ligne et la nature du problème
3. **`-fsanitize=address`** (ASan) — détecte tous les bugs mémoire : hors bornes, use-after-free, double free, fuites

> **Règle de travail : développe toujours avec les sanitizers activés.** Ils coûtent ~2× en vitesse d'exécution, et ils te font gagner des jours.

---
---

# Chapitre 15 — Les outils

En C, les outils ne sont pas un luxe : **ils compensent l'absence de garde-fous du langage.**

## Les options de compilation

```bash
-Wall -Wextra              # les avertissements de base. NON NÉGOCIABLE.
-Wpedantic                 # strict respect de la norme
-Wshadow                   # une variable locale masque une autre
-Wconversion               # conversions implicites qui perdent de l'information
-Wunreachable-code         # code mort
-Werror                    # transforme les warnings en erreurs (pour la CI)

-g                         # symboles de débogage (nécessaire pour lldb)
-O0                        # pas d'optimisation (développement)
-O2                        # optimisation (production)

-fsanitize=address         # ASan  — bugs mémoire
-fsanitize=undefined       # UBSan — comportement indéfini
-fno-omit-frame-pointer    # piles d'appel lisibles dans les rapports
```

> ⚠️ **Un warning n'est pas une suggestion.** En C, un avertissement signale presque toujours un vrai bug, actuel ou futur. Vise **zéro warning**.

## Dans CMake

```cmake
target_compile_options(cloud PRIVATE -Wall -Wextra -Wunreachable-code)

# profil de développement avec sanitizers
target_compile_options(cloud PRIVATE
    $<$<CONFIG:Debug>:-g -O0 -fsanitize=address,undefined -fno-omit-frame-pointer>)
target_link_options(cloud PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=address,undefined>)
```

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
```

> ⚠️ Les sanitizers doivent être passés **à la compilation ET au link**. C'est l'oubli classique.

## Les sanitizers en action

```
=================================================================
==12345==ERROR: AddressSanitizer: heap-buffer-overflow
WRITE of size 8 at 0x6020000000f8 thread T0
    #0 0x10a3f in grid_set grid.c:42
    #1 0x10b22 in main main.c:17

0x6020000000f8 is located 0 bytes to the right of 8-byte region
allocated by thread T0 here:
    #0 0x1096a in malloc
    #1 0x10a01 in grid_create grid.c:20
```

Il te donne : le **type** de bug, la **ligne** du bug, et la **ligne de l'allocation** concernée. C'est incomparablement plus efficace que de chercher à la main.

> 💡 Sur Mac Apple Silicon, **Valgrind ne fonctionne pas**. ASan/UBSan sont le bon outil, et ils sont intégrés à clang — rien à installer.

## Le débogueur : `lldb`

Sur macOS c'est `lldb` (pas `gdb`). Il faut avoir compilé avec `-g`.

```bash
lldb ./build/cloud
```

```
(lldb) b grib.c:42          # breakpoint à la ligne 42
(lldb) b grib_inspect       # breakpoint sur une fonction
(lldb) run                  # lancer
(lldb) n                    # next — ligne suivante, sans entrer dans les appels
(lldb) s                    # step — entrer dans l'appel
(lldb) c                    # continue
(lldb) p variable           # afficher une variable
(lldb) p *pointeur          # déréférencer
(lldb) x/16xb pointeur      # dump hexa de 16 octets
(lldb) bt                   # backtrace — la pile d'appels
(lldb) frame variable       # toutes les locales
(lldb) q                    # quitter
```

Après un crash, `run` puis `bt` te dit **exactement** où ça a pété et par quel chemin on y est arrivé.

> **Apprends `b`, `run`, `n`, `p`, `bt`.** Ces cinq commandes couvrent 90 % des besoins, et une heure investie là te fera gagner des semaines de `printf`.

## Inspecter des fichiers binaires

```bash
file fichier                   # identifier le type
ls -lh fichier                 # taille lisible
xxd fichier | head -5          # hexdump + ASCII
xxd -s 16 -l 32 fichier        # 32 octets à partir de l'offset 16
xxd -b fichier | head          # en BINAIRE (pratique pour les bits)
```

`xxd` sera ton meilleur ami pour le GRIB : il te montre la réalité, pas ton interprétation de la réalité.

## Autres outils utiles

```bash
clang-format -i src/*.c        # formatage automatique
clang-tidy src/*.c             # analyse statique (bugs probables)
nm build/cloud | grep mon_sym  # lister les symboles d'un binaire
otool -L build/cloud           # bibliothèques dynamiques liées (macOS)
man fwrite                     # LA doc de référence. Lis les man pages.
```

## La méthode de travail

> **1. Une erreur à la fois.** Lire la première, corriger, recompiler.
>
> **2. Toujours garder un terrain qui compile.** Quand plus rien ne compile, on est perdu. Avance par micro-étapes et compile après chacune.
>
> **3. Zéro warning.**
>
> **4. Développe avec les sanitizers.** Toujours.
>
> **5. Ne cherche pas un bug mémoire à l'œil nu.** Lance ASan.
>
> **6. « Ça marche » n'est pas une preuve.** Teste aussi les chemins d'erreur.
>
> **7. Lis les `man`.** La doc Unix est excellente et fait autorité.
>
> **8. *Make it work, make it right, make it fast* — dans cet ordre.**

---
---

# Annexe — Les 32 mots-clés du C

C'est tout le langage. Vraiment.

```
Types           char  short  int  long  float  double  signed  unsigned  void
Structuration   struct  union  enum  typedef
Stockage        auto  register  static  extern  const  volatile
Contrôle        if  else  switch  case  default  for  while  do  break  continue  goto  return
Autre           sizeof
```

(Plus quelques ajouts C99/C11 : `inline`, `restrict`, `_Bool`, `_Atomic`, `_Generic`, `_Static_assert`…)

Compare avec les ~90 mots-clés de C++, ou l'immensité de Python avec sa bibliothèque standard. **Le C tient dans ta tête.** C'est sa principale qualité.
