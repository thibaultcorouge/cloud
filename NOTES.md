# Fiche de lecture — Projet Cloud (C)

> Carnet d'apprentissage du C, construit au fil du projet.
> **À alimenter à chaque nouvelle étape** : nouveaux concepts, nouveaux pièges rencontrés.
>
> 📘 Pour le **cours** sur le langage lui-même (progressif, indépendant du projet) : **[`COURS-C.md`](COURS-C.md)**.
> Ici, c'est le journal : ce qu'on a fait, et les erreurs réellement commises.

---

## Sommaire

- [0. Le projet](#0-le-projet)
- [1. Pourquoi C et pas C++](#1-pourquoi-c-et-pas-c)
- [2. Le modèle de compilation](#2-le-modèle-de-compilation)
- [3. Header `.h` vs source `.c`](#3-header-h-vs-source-c)
- [4. Les types](#4-les-types)
- [5. Les pointeurs](#5-les-pointeurs)
- [6. Les chaînes de caractères](#6-les-chaînes-de-caractères)
- [7. Les fichiers](#7-les-fichiers)
- [8. L'affichage](#8-laffichage)
- [9. Déclarer / définir / appeler](#9-déclarer--définir--appeler)
- [10. Les fonctions](#10-les-fonctions)
- [11. La gestion d'erreur](#11-la-gestion-derreur)
- [12. Les codes de sortie](#12-les-codes-de-sortie)
- [13. libcurl](#13-libcurl)
- [14. Le format GRIB2](#14-le-format-grib2)
- [15. Dictionnaire C++ → C](#15-dictionnaire-c--c)
- [16. Journal des pièges rencontrés](#16-journal-des-pièges-rencontrés)
- [17. Commandes utiles](#17-commandes-utiles)
- [18. État d'avancement](#18-état-davancement)

---

## 0. Le projet

**But final :** un outil de modélisation de nuages à partir de données météo réelles.

| Étape | Contenu | Concepts |
|---|---|---|
| **A** ✅ | Télécharger les données | libcurl, RAII manuel, pointeurs, callbacks C |
| **B** 🚧 | Parser le GRIB2, extraire l'humidité | binaire, `uint8_t`, endianness, bits |
| **C** | Stocker la grille 3D | structures, layout mémoire, tableaux dynamiques |
| **D** | Interpolation / krigeage | maths, algèbre linéaire, performance |
| **E** | Marching Cubes | tables de lookup, géométrie, maillage |
| **F** | Affichage 3D navigable | OpenGL, shaders, matrices |

**Note :** les algos D et E sont à écrire soi-même, sans aide.

---

## 1. Pourquoi C et pas C++

Toute la pile du projet est **nativement en C** : libcurl, eccodes, OpenGL, GLFW, LAPACK.

**Ce que C coûte :**
- pas de `std::vector` → écrire son propre tableau dynamique (`malloc`/`realloc`/`free`)
- pas de `std::string` → `char *` + `snprintf`, gestion manuelle des tampons
- pas de surcharge d'opérateurs → `v3_add(a, v3_scale(b, t))` au lieu de `a + b * t`
- pas de destructeur → nettoyer à la main sur **chaque** chemin de sortie

**Ce que C rapporte :**
- le langage est **fini** : ~200 pages, on peut le terminer
- **rien ne se passe dans ton dos** : pas de copie implicite, pas de constructeur caché
- messages d'erreur courts et lisibles
- l'intuition Go se transfère presque directement

**Ce n'est pas un cul-de-sac :** C est un quasi-sous-ensemble de C++. On peut migrer plus tard, l'inverse n'existe pas.

---

## 2. Le modèle de compilation

La chose qui n'existe ni en Go, ni en Python, ni en JS. **Trois étapes distinctes :**

```
1. PRÉPROCESSEUR  →  2. COMPILATEUR  →  3. LINKER
   (du texte)          (fichier par       (assemble les .o
                        fichier → .o)      + les bibliothèques)
```

**1. Le préprocesseur** est bête. `#include "x.h"` signifie littéralement *« colle le contenu de ce fichier ici »*. C'est du copier-coller textuel. Rien d'autre.

```bash
clang -E -Iinclude src/main.c | wc -l   # voir l'explosion
```

**2. Le compilateur** traite chaque `.c` **isolément** (une *unité de traduction*) et produit un `.o`. Il ne sait **rien** des autres `.c`.

**3. Le linker** recolle les `.o` et vérifie que chaque promesse a sa réalité.

### Savoir lire les erreurs

| Message | Étape | Cause |
|---|---|---|
| `use of undeclared identifier 'foo'` | compilateur | il manque un `#include` |
| `implicit declaration of function 'foo'` | compilateur | il manque un `#include` |
| `Undefined symbols: "_foo"` | **linker** | déclaré mais jamais défini, ou bibliothèque non liée |

### ⚠️ Les erreurs cascadent

Un symbole inconnu fait perdre le fil au compilateur : les erreurs 2 à 20 sont souvent des **conséquences** de l'erreur 1.

> **Règle de travail : lire la PREMIÈRE erreur. La corriger. Recompiler. Recommencer.**
> Ça paraît lent, c'est dix fois plus rapide.

```bash
cmake --build build 2>&1 | head -20
```

---

## 3. Header `.h` vs source `.c`

- **Déclaration** (dans le `.h`) = « je promets que ça existe »
- **Définition** (dans le `.c`) = « voici le corps »

### Include guard

```c
#ifndef GRIB_H
#define GRIB_H
/* ... */
#endif
```

Nécessaire parce que `#include` est un copier-coller : sans ça, deux inclusions = double définition = erreur. (Alternative moderne, non standard mais universelle : `#pragma once`.)

### Règle d'or des includes

> **Un header n'inclut que ce dont ses propres déclarations ont besoin.**

Si le `.h` déclare une fonction qui retourne `bool` → `<stdbool.h>` suffit. S'il ne mentionne pas `FILE *`, il n'a pas besoin de `<stdio.h>` — c'est le `.c` qui en a besoin.

---

## 4. Les types

### Les entiers de taille garantie — `<stdint.h>`

```c
uint8_t    /* non signé, exactement  8 bits (1 octet), 0..255   */
uint16_t   /* non signé, exactement 16 bits (2 octets)          */
uint32_t   /* non signé, exactement 32 bits (4 octets)          */
uint64_t   /* non signé, exactement 64 bits (8 octets)          */
int8_t, int16_t, int32_t, int64_t   /* versions signées         */
```

`u` = unsigned, le nombre = les bits, `_t` = type.

### ⚠️ Jamais `char` pour du binaire

La signitude de `char` n'est **pas spécifiée** par la norme. Sur macOS arm64 il est **signé** :

```c
char    c = 0xFF;   /* → -1   ⚠️ tous les calculs partent en vrille */
uint8_t o = 0xFF;   /* → 255  ✅                                     */
```

> **Règle : `uint8_t` pour les octets bruts. `char` pour le texte, et c'est tout.**

### `size_t`

Entier **non signé**, assez grand pour contenir la taille de n'importe quel objet. Sur ce Mac : 8 octets.

```
sizeof(int)    = 4 octets  →  max  2 147 483 647            (≈ 2,1 Go)
sizeof(size_t) = 8 octets  →  max 18 446 744 073 709 551 615
```

C'est le type correct pour **toute taille, tout compteur d'octets, tout indice de tableau**.

### ⚠️⚠️ LE piège de `size_t` : l'arithmétique non signée

Un `size_t` ne peut pas être négatif. Sous zéro, il **repasse par le maximum** (*wraparound*) :

```
int    : 0 - 1 = -1
size_t : 0 - 1 = 18446744073709551615   ← !!!
```

```c
/* BOUCLE INFINIE : i >= 0 est TOUJOURS vrai pour un size_t */
for (size_t i = len - 1; i >= 0; i--) { ... }
```

Et si `len == 0`, alors `len - 1` = 18 quintillions → lecture hors mémoire.

> **Règle : ne jamais soustraire sur un `size_t` sans vérifier que le résultat reste positif.**

### `bool`

Depuis `<stdbool.h>` : `bool`, `true`, `false`. (En C23 c'est devenu un mot-clé natif.)

---

## 5. Les pointeurs

Un pointeur est une variable qui contient **une adresse mémoire**. C'est tout.

```c
char *p;   /* « p contient l'adresse d'un char » */
```

Sur une machine 64 bits, une adresse tient sur 8 octets — **quelle que soit la taille de ce qui est pointé** :

```
sizeof(char *) = 8        toujours
```

### Les deux sens de `*` (confusion n°1)

```c
char *p = "abc";   /* DÉCLARATION : « p est un pointeur vers char »        */
char  c = *p;      /* EXPRESSION  : « la VALEUR à l'adresse p »   → 'a'    */
```

Et l'opérateur inverse `&` = « l'adresse de » :

```c
int  x = 42;
int *q = &x;    /* q contient l'adresse de x       */
*q = 99;        /* écrit 99 DANS x, via son adresse */
```

### `.` vs `->`

```c
machin.champ      /* via une valeur   */
machin->champ     /* via un pointeur — équivaut à (*machin).champ */
```

### Arithmétique de pointeurs

```c
for (const char *p = s; *p != '\0'; p++) { ... }
/*   *p        = l'octet courant
     p++       = avance d'un char
     p - s     = l'indice courant (distance entre deux pointeurs) */
```

C'est littéralement l'implémentation de toutes les fonctions de `<string.h>`.

### Quand utiliser un pointeur

| Besoin | Forme |
|---|---|
| Passer un tableau | `int *data` **+ un `size_t n` à côté** |
| **Modifier** la variable de l'appelant (*out-param*) | `void f(int *out)` puis `*out = 42;` |
| Éviter de copier une grosse structure | `void f(const Grid *g)` |
| Représenter « rien » / l'échec | retour `NULL` |
| Un handle opaque | `FILE *`, `CURL *` |

### Les out-params

**Une fonction C ne peut retourner qu'UNE valeur.** Pas de `return a, b` comme en Go.
Pour rendre deux choses (« ça a marché » **et** « voici les données »), on retourne l'une et on passe un pointeur pour l'autre.

```c
long code = 0;
curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);  /* écrit dans code */
```

### `void *` — le pointeur sans type

Pointeur vers n'importe quoi. C'est le mécanisme d'effacement de type du C, utilisé pour passer du contexte aux callbacks (C n'a ni generics, ni closures).

**En C, un `void *` se convertit implicitement vers n'importe quel type de pointeur** — pas de cast nécessaire (contrairement à C++ qui exige `static_cast`). Plus simple, et plus dangereux : rien ne t'arrête si tu te trompes de type.

### ⚠️ Pointeur pendouillant (*dangling pointer*)

Utiliser un pointeur vers une ressource déjà libérée. Parfois ça plante, parfois ça écrit n'importe où en silence.

```c
fclose(f);
curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);   /* f ne désigne plus rien ⚠️ */
```

---

## 6. Les chaînes de caractères

**Il n'y a pas de type string en C.** Une « chaîne » est une convention : un pointeur vers le premier octet, et ça s'arrête au premier octet nul `'\0'`.

```
const char *url = "https://exemple.fr";

mémoire :  h  t  t  p  s  :  /  /  ...  .  f  r  \0
           ^
           url pointe ici
```

### Le parallèle Go

```go
// Go : une string est une STRUCTURE de 16 octets
type string struct {
    ptr *byte   // 8 octets : où ça commence
    len int     // 8 octets : combien de temps ça dure
}
```

```c
// C : juste le pointeur. 8 octets. La longueur ? Débrouille-toi.
const char *s;
```

Le C a **jeté le champ `len`** et l'a remplacé par la convention du `\0`. Conséquences :

- `len(s)` en Go est **O(1)**. `strlen(s)` en C est **O(n)** → ne jamais l'appeler dans une condition de boucle.
- Go connaît la taille donc peut vérifier les accès. Le C ne peut rien vérifier → dépassements de tampon.

### ⚠️ Un littéral est en lecture seule

`"https://..."` vit dans un segment mémoire non modifiable. Écrire dedans = crash. D'où le `const char *` : il est **nécessaire**, pas décoratif.

### ⚠️ Pas de concaténation

Pas de `+`. Il faut un tampon qu'on a soi-même alloué, et y écrire avec `snprintf`.

> **Dogme : `snprintf`, jamais `sprintf`. `snprintf`/`strncpy`, jamais `strcpy`.**
> Les versions sans `n` ne connaissent pas la taille de la destination et écrivent au-delà. C'est *la* faille historique du C.

### ⚠️ En binaire, jamais de fonction `str*`

`strcmp`, `strlen` etc. s'arrêtent au premier `\0`. Un fichier binaire est truffé de `\0`.

```c
memcmp(header, "GRIB", 4) == 0   /* ✅ compare exactement 4 octets */
```

`memcmp` retourne **`0` quand c'est identique** (penser « différence nulle »).

---

## 7. Les fichiers

`FILE *` est un **type opaque** : on ne sait pas ce qu'il y a dedans, on ne peut que le passer aux fonctions de la lib. (`CURL *` aussi.)

```c
FILE *f = fopen(chemin, "wb");   /* "rb" = read binary, "wb" = write binary */
if (f == NULL) { ... }           /* TOUJOURS tester */
...
fclose(f);                       /* pas de destructeur : c'est toi qui fermes */
```

- le **`b`** = binaire. Inutile sur macOS/Linux, **vital** sur Windows (sinon `\n` → `\r\n` et le GRIB est corrompu). À mettre par réflexe.
- une fonction C qui rend un pointeur rend **`NULL`** en cas d'échec. Le duo `fopen` + `if (NULL)` est *le* réflexe C.

### Lire / écrire

```c
size_t fread (void *dest,   size_t taille_elem, size_t nb_elem, FILE *f);
size_t fwrite(const void *src, size_t taille_elem, size_t nb_elem, FILE *f);
```

Les deux retournent le **nombre d'ÉLÉMENTS** traités, pas le nombre d'octets.

> **Idiome : avec `taille_elem = 1`, éléments == octets.** C'est pour ça qu'on écrit `fwrite(buf, 1, n, f)`.

⚠️ `fread` peut lire **moins** que demandé (fin de fichier). Toujours comparer le retour :

```c
if (fread(tampon, 1, 16, f) != 16) { /* fichier trop court */ }
```

### Un tampon

```c
uint8_t header[16];   /* 16 octets sur la pile */
```

Le nom d'un tableau se convertit tout seul en pointeur vers son premier élément → on passe juste `header`.

### Autres

- `fseek(f, offset, SEEK_SET)` — se déplacer dans le fichier
- `remove(chemin)` — supprimer un fichier
- `free(NULL)` est **autorisé** et sans effet. `fclose(NULL)` est un **crash**. D'où `if (f != NULL) fclose(f);`

---

## 8. L'affichage

```c
printf("texte %s et %d\n", chaine, nombre);     /* vers stdout */
fprintf(stderr, "erreur : %s\n", message);      /* vers stderr */
perror("mon message");                          /* message + raison système (errno) */
```

Le format d'abord, les valeurs ensuite. Et `\n` à la main (pas de `endl`).

### Spécificateurs — source d'erreur n°1

| Type | Format |
|---|---|
| `int` | `%d` |
| `long` | `%ld` |
| `size_t` | `%zu` ⚠️ pas `%d` |
| `double` / `float` | `%f` ou `%g` |
| `char *` | `%s` |
| pointeur | `%p` |
| un vrai `%` | `%%` |
| hexadécimal | `%x` / `%02x` |

⚠️ `%l` tout seul n'existe pas : `l` est un **modificateur de longueur**, il doit être suivi de la lettre du type.

⚠️ Mauvais spécificateur = ça compile, comportement indéfini. **C'est `-Wall` qui te sauve** : il vérifie la cohérence format/arguments.

### `perror` vs `fprintf`

`perror` ajoute la raison système (`errno`) : « permission refusée », « dossier inexistant ». **Elle n'a de sens qu'après un appel qui positionne `errno`** (`fopen`, `read`…). Après `curl_easy_init`, elle afficherait un message sans rapport.

### ⚠️ Tamponnage (*buffering*)

`stderr` n'est **pas** tamponné (sort immédiatement), `stdout` l'est. Quand la sortie n'est pas un terminal, **les deux se désynchronisent** et les messages apparaissent dans le désordre. Ce n'est pas un bug du programme.

---

## 9. Déclarer / définir / appeler

```c
/* DÉCLARER : « cette fonction existe, voici ses TYPES » */
FILE *fopen(const char *path, const char *mode);

/* APPELER : « exécute-la, voici mes VALEURS » */
FILE *f = fopen(destination, "wb");
```

| | Déclaration | Appel |
|---|---|---|
| dans les parenthèses | des **types** | des **valeurs** |
| finit par | `;` | fait partie d'une expression |
| résultat | aucun | on le **stocke** |

### ⚠️ Les parenthèses font l'appel

```c
fclose;      /* je NOMME la fonction (son adresse). Il ne se passe rien. */
fclose(f);   /* je l'APPELLE. */
```

### ⚠️ Une fonction qui retourne, retourne

```c
curl_easy_strerror(res);                                 /* la chaîne est calculée puis JETÉE */
fprintf(stderr, "%s\n", curl_easy_strerror(res));        /* ✅ */
```

Le compilateur ne dit **rien** : ignorer un retour est légal. Les noms aident : `strerror` = *string* error → elle **fabrique** une chaîne. Si elle affichait, elle s'appellerait `printerror`.

> **Règle : quand une fonction retourne quelque chose, se demander « qu'est-ce que je fais de cette valeur ? »**

---

## 10. Les fonctions

### ⚠️ Pas de fonctions imbriquées

Une définition de fonction ne peut exister qu'au **niveau du fichier** (*file scope*). Pas de fonctions dans les fonctions, pas d'anonymes, pas de closures, pas de lambdas.

Et le compilateur lit de haut en bas : **une fonction doit être définie (ou déclarée) avant d'être utilisée.**

### `static`

⚠️ **`static` a trois sens différents selon le contexte.**

1. Sur une fonction au niveau fichier → **visible uniquement dans cette unité de traduction**. L'équivalent exact de la minuscule en Go.
2. Sur un membre de struct en C++ → partagé entre instances (sans objet en C).
3. Sur une variable locale → survit entre les appels.

### `extern`

Sur une **fonction**, `extern` est le défaut → on ne l'écrit jamais.
Sur une **variable globale**, il est indispensable.

### Paramètre non utilisé

```c
(void)path;   /* « je sais, tais-toi » — silence le warning -Wunused-parameter */
```

### Nommage C

- fonctions = **verbes** en `snake_case` : `grib_inspect`, `cloud_download`
- pas de namespaces → on **préfixe** : `cloud_`, `grib_`
- types (structs) = noms communs, souvent suffixés `_t`
- macros en `MAJUSCULES`

### L'équivalent d'une classe

```
un struct (l'état)  +  des fonctions prenant un pointeur vers ce struct en 1er argument
```

C'est littéralement ce que C++ fait dans ton dos (`this` est un argument caché). Comme le `func (d *Downloader) ...` de Go, mais explicite.

**Corollaire :** une classe **sans attribut** n'est pas un objet, c'est juste un préfixe de nom déguisé → en C, ça devient une simple fonction libre.

### Attribut ≠ paramètre ≠ retour

Le critère est la **durée de vie**. Un attribut appartient à l'objet et **survit entre les appels**. Un paramètre naît à l'appel et meurt au `return`.

---

## 11. La gestion d'erreur

### Le principe fondamental

> **La durée de vie d'une ressource doit correspondre à la durée de vie de son propriétaire.**

Une ressource globale au processus (`curl_global_init`) appartient à `main`, pas au constructeur d'un objet qu'on peut instancier N fois.

### Guard clauses (clauses de garde)

```c
if (erreur_A) { ...; goto cleanup; }
if (erreur_B) { ...; goto cleanup; }

/* si on arrive ici, A et B sont OK. Garanti. */
ok = true;
```

**Chaque erreur sort immédiatement, donc le chemin du succès reste à l'indentation zéro** et se lit de haut en bas comme une recette.

La forme imbriquée (`if/else` emboîtés) s'appelle l'*arrow anti-pattern* : chaque test ajoute un niveau et enterre le code utile tout au fond à droite.

C'est l'*early return* de JS, ou le `if err != nil { return err }` de Go.

### L'idiome `goto cleanup`

**LE** pattern de gestion d'erreur du C. Présent dans le noyau Linux, SQLite, libcurl.

```c
bool exemple(void)
{
    FILE *f    = NULL;      /* 1. ressources EN HAUT, à NULL */
    CURL *curl = NULL;
    bool  ok   = false;     /* 2. on suppose l'ÉCHEC */

    f = fopen("x", "rb");
    if (f == NULL) { fprintf(stderr, "..."); goto cleanup; }   /* 3. saut */

    curl = curl_easy_init();
    if (curl == NULL) { fprintf(stderr, "..."); goto cleanup; }

    /* ... le travail ... */

    ok = true;              /* 4. true UNIQUEMENT à la toute fin */

cleanup:                    /* 5. une étiquette (label) */
    if (curl != NULL) curl_easy_cleanup(curl);   /* ordre INVERSE de l'acquisition */
    if (f    != NULL) fclose(f);
    return ok;              /* un SEUL return */
}
```

**Les cinq règles :**
1. Toutes les ressources déclarées en haut, à `NULL` — pour que `cleanup` puisse toujours les examiner, même si on saute avant leur acquisition.
2. `ok = false` par défaut, `true` seulement à la dernière ligne du travail utile → impossible d'oublier de signaler une erreur.
3. Tous les chemins d'erreur convergent vers l'étiquette via `goto`.
4. Libérer dans l'**ordre inverse** de l'acquisition.
5. Attention aux fonctions de libération qui n'acceptent pas `NULL` (`fclose` non, `free` oui, `curl_easy_cleanup` oui).

**`goto cleanup` REMPLACE le `return`** → un `return` juste après est du code mort.

### ⚠️⚠️ Une étiquette ne bloque pas l'exécution

C'est **juste un nom donné à une position**. Le programme coule de haut en bas et **tombe dedans** tout seul.

```
                  ┌──────────────────────┐
if (erreur A) ────┤                      │
if (erreur B) ────┤  goto cleanup (SAUT) │
if (erreur C) ────┤                      │
                  └──────────┬───────────┘
                             │
ok = true;   ────────────────┤  on TOMBE dedans, PAS de saut
                             ▼
cleanup:
    libérer
    return ok;
```

**Sur le chemin du succès, il n'y a aucun `goto` à écrire.** Un `goto cleanup` juste avant `ok = true` sauterait par-dessus.

### Pourquoi `goto` est légitime ici

La règle « jamais de `goto` » visait les sauts **en arrière** et ceux qui entrelacent la logique. Ici c'est un saut **en avant vers un point de sortie unique** : ça *réduit* la complexité. C'est l'unique usage du `goto` universellement accepté en C.

### Ce que ça remplace

En C++, le **destructeur** est appelé automatiquement à la sortie du scope, sur tous les chemins, même sur exception. Ce pattern (**RAII**) n'existe pas en C : chaque `return` est une porte qu'il faut garder soi-même. D'où le `goto cleanup`.

---

## 12. Les codes de sortie

Ce que `main` retourne devient le **code de sortie du processus** (`echo $?` dans le shell).

```
0             → succès
tout le reste → échec
```

`<stdlib.h>` fournit `EXIT_SUCCESS` (0) et `EXIT_FAILURE` (1), plus lisibles que des chiffres nus.

Asymétrie : **une** façon de réussir, plein de façons d'échouer.

⚠️ Un programme qui affiche « Échec » puis retourne `0` **mente au shell** : `./prog && echo ok` afficherait `ok`.

---

## 13. libcurl

### Cycle de vie

```c
curl_global_init(CURL_GLOBAL_DEFAULT);   /* dans main, EN PREMIER, avant tout appel curl */
  CURL *curl = curl_easy_init();         /* un "easy handle" */
  curl_easy_setopt(curl, OPTION, valeur);
  CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);
curl_global_cleanup();                   /* dans main, EN DERNIER */
```

⚠️ Aucun appel libcurl avant `curl_global_init` ni après `curl_global_cleanup`. Sans le global init, `curl_easy_init` le fait discrètement — mais **de façon non thread-safe**. Piège qui ne se manifeste qu'en multithread, atroce à diagnostiquer.

`curl_global_init` retourne un `CURLcode` → à tester.

### Les options utilisées

| Option | Rôle |
|---|---|
| `CURLOPT_URL` | l'URL (un `const char *`) |
| `CURLOPT_WRITEDATA` | le `FILE *` destination |
| `CURLOPT_FOLLOWLOCATION` | suivre les redirections (`1L`) |
| `CURLOPT_WRITEFUNCTION` | callback d'écriture — **optionnel** |

> 💡 **Sans `CURLOPT_WRITEFUNCTION`, libcurl utilise `fwrite` par défaut.** Donc passer un `FILE *` à `CURLOPT_WRITEDATA` suffit pour télécharger dans un fichier : pas besoin d'écrire de callback.

⚠️ `FILE *f` **est déjà** un pointeur → on passe `f`, pas `&f`.
⚠️ Le fichier doit rester **ouvert pendant tout le `perform`** (c'est là que l'écriture a lieu).

### ⚠️⚠️ Deux erreurs différentes à tester

| Test | Question | Échecs couverts |
|---|---|---|
| `res != CURLE_OK` | *La conversation a-t-elle eu lieu ?* | DNS, connexion refusée, timeout, TLS invalide |
| code HTTP hors 2xx | *Qu'a répondu le serveur ?* | 404, 403, 500 |

**`CURLE_OK` signifie « la conversation HTTP s'est bien passée ». Un 404 est une conversation réussie.** Sans le second test, une page d'erreur HTML est écrite dans le fichier et le programme annonce « Succès ».

```c
long code = 0;
curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);   /* out-param : le & */
if (code < 200 || code >= 300) { ... }
```

`curl_easy_strerror(res)` **retourne** le message, elle ne l'affiche pas.

### Le callback d'écriture (si besoin un jour)

```c
size_t (*callback)(char *ptr, size_t size, size_t nmemb, void *userdata);
```

- doit être au **niveau fichier**, `static`, et **avant** la fonction qui l'utilise
- `void *userdata` = *pointeur utilisateur* / *contexte*. Seul moyen en C de passer de l'état à un callback (une closure JS capture son environnement ; en C on le transporte à la main)
- doit retourner le nombre d'**octets** consommés

### Réutiliser le handle (optimisation à venir)

Un handle contient la **connexion ouverte**. Ouvrir une connexion HTTPS coûte :

```
DNS + handshake TCP (1 RTT) + handshake TLS (1-2 RTT)  ≈ 2 à 3 allers-retours
```

À ~30 ms de RTT : ~100 ms perdus par fichier. Sur 20 altitudes ≈ 2 s de gâchis + 20 handshakes TLS inutiles.

libcurl gère le **keep-alive automatiquement** — mais seulement si on réutilise le même handle. `init`/`cleanup` à chaque fois lui interdit toute réutilisation.

> **TODO :** réutiliser le handle quand on bouclera sur les altitudes → ce sera le moment d'introduire un `struct` (l'état = le `CURL *`) avec des fonctions `init` / `download` / `destroy`.

**Principe de conception :** ne pas mettre la boucle dans la bibliothèque. Donner à l'appelant la plus petite brique utile et le laisser composer — sinon il perd le contrôle (arrêt sur erreur, décision de l'URL suivante…).

---

## 14. Le format GRIB2

Format binaire de l'OMM pour les données météo. **Entièrement big-endian.**

### Un fichier = une suite de messages indépendants

Pas un bloc unique : des messages collés bout à bout, chacun portant **un paramètre à une altitude**. Chaque message **dit lui-même sa longueur**, ce qui permet de sauter au suivant.

```
┌──────────── un message GRIB2 ────────────┐
│ Section 0  (16 octets FIXES)             │
│ Section 1  (longueur variable)           │
│ Sections 2..7 (longueur variable)        │
│ Section 8  "7777"                        │
└──────────────────────────────────────────┘
      puis le message suivant, collé derrière
```

Chaque section (sauf 0 et 8) commence par : **longueur sur 4 octets**, puis **numéro de section sur 1 octet**.

### Section 0 — 16 octets

| Octets | Contenu |
|---|---|
| 1-4 | `"GRIB"` — nombre magique (`47 52 49 42`) |
| 5-6 | réservé (`ffff`) |
| 7 | **discipline** (0 = produits météorologiques) |
| 8 | **édition** (2 = GRIB2) |
| 9-16 | **longueur totale du message** (64 bits big-endian) |

### Section 1 — identification

| Octets | Contenu |
|---|---|
| 1-4 | longueur de la section |
| 5 | numéro de section = `1` |
| 6-7 | **centre émetteur** (85 = Météo-France) |
| 29-30 | année |
| 31 | mois |
| 32 | jour |

### ⚠️ Big-endian — la recette

Le GRIB stocke **l'octet de poids fort d'abord**. Un x86/arm64 est **little-endian**. On ne peut donc **pas** `fread` directement dans un `uint64_t` : il faut recomposer octet par octet.

```c
uint64_t total = 0;
for (int i = 8; i < 16; i++) {
    total = (total << 8) | header[i];
}
```

**L'analogie :** c'est exactement comme lire `347` chiffre par chiffre en base 10 —
`total = total*10 + chiffre`. Ici on est en **base 256**, donc « ×256 » s'écrit `<< 8`,
et « + » s'écrit `|` (les 8 bits de droite viennent d'être libérés, ils sont à zéro).

Trace réelle sur `00 00 00 00 00 3b d3 35` :

```
  i= 8..12  + 0x00  ->             0
  i=13      + 0x3b  ->            59
  i=14      + 0xd3  ->        15 315
  i=15      + 0x35  ->     3 920 693
```

⚠️ **Trois pièges :**
- **l'ordre** : parcourir du poids fort vers le poids faible (indice croissant ici)
- **le type de l'accumulateur** : `uint64_t` pour 8 octets, et **non signé** (décaler du signé est de l'UB)
- **le type du tampon** : `uint8_t`, jamais `char` (un octet ≥ 0x80 deviendrait négatif et ruinerait le `|`)

Affichage d'un `uint64_t` : `%llu` (ou la forme portable `printf("%" PRIu64, x)` avec `<inttypes.h>`).

### Niveaux (code table 4.5)

| Code | Signification |
|---|---|
| 1 | sol / surface |
| **100** | **surface isobare (Pa)** ← ce qu'il faut pour les nuages |
| 103 | hauteur au-dessus du sol (m) |
| 105 | niveau hybride |

### Paramètres (discipline 0)

| Catégorie | Paramètre | Nom |
|---|---|---|
| 0 | 0 | Température |
| **1** | **0** | **Humidité spécifique** (kg/kg) |
| **1** | **1** | **Humidité relative** (%) |
| 2 | 2 / 3 | Vent U / V |
| 3 | 0 / 5 | Pression / Géopotentiel |
| 6 | 1 | Couverture nuageuse |

### ⚠️ Choix du jeu de données

Chez Météo-France, les paquets sont nommés :

| Suffixe | Signification | Utile ici ? |
|---|---|---|
| `SP` | *surface* | non |
| `HP` | *height* — hauteur au-dessus du sol | ❌ 10/20/50/100 m seulement |
| **`IP`** | ***isobaric*** — niveaux de pression | ✅ **c'est ce qu'il faut** |

Correspondance pression → altitude :

```
1000 hPa ≈    100 m        700 hPa ≈  3 000 m
 925 hPa ≈    800 m        500 hPa ≈  5 500 m
 850 hPa ≈  1 500 m        300 hPa ≈  9 000 m
```

Les nuages vivent entre ~500 m et ~12 000 m → il faut un paquet **`IP`**.

---

## 15. Dictionnaire C++ → C

| C++ | C | Note |
|---|---|---|
| `#include <iostream>` | `#include <stdio.h>` | toute l'E/S |
| `#include <string>` | `#include <string.h>` | `strlen`, `memcmp`, `memcpy`… |
| `std::cout << x << "\n"` | `printf("%s\n", x)` | format explicite |
| `std::cerr << x` | `fprintf(stderr, "%s", x)` | |
| `std::endl` | `"\n"` | |
| `std::string` | `char *` / `const char *` | |
| `const std::string& s` | `const char *s` | pas de références en C |
| `std::ofstream f(p, binary)` | `FILE *f = fopen(p, "wb")` | tester `NULL` |
| `f->write(buf, n)` | `fwrite(buf, 1, n, f)` | ordre des args différent ! |
| `f.close()` | `fclose(f)` | obligatoire |
| `url.c_str()` | `url` | c'est déjà un `const char *` |
| `static_cast<T *>(p)` | *rien* | conversion implicite |
| `nullptr` | `NULL` | |
| `new` / `delete` | `malloc` / `free` | |
| `class` | `struct` + fonctions | |
| `namespace` | un préfixe dans le nom | |
| `CURL* curl` | `CURL *curl` | l'étoile colle au nom, en C |
| `if (!curl)` | `if (curl == NULL)` | plus explicite |
| destructeur | `goto cleanup` | |

---

## 16. Journal des pièges rencontrés

> Les erreurs réellement commises dans ce projet. La partie la plus utile de cette fiche.

| # | Erreur | Leçon |
|---|---|---|
| 1 | `curl_global_init` dans un constructeur | La durée de vie d'une ressource doit correspondre à celle de son propriétaire. Une ressource globale appartient à `main`. |
| 2 | Croire que `bool` et les `string` de la signature étaient des attributs | Attribut ≠ paramètre ≠ retour. Le critère est la durée de vie. |
| 3 | `LANGUAGES C` sur sa propre ligne dans le CMakeLists | C'est un **argument** de `project()`, pas une commande. |
| 4 | `return 0;` **avant** `curl_global_cleanup();` | `return` quitte immédiatement. Tout ce qui suit est du code mort. |
| 5 | `curl_global_init()` sans argument | Lire le `man` : `CURLcode curl_global_init(long flags)`. |
| 6 | `extern void downloader(int downloader);` | `extern` inutile sur une fonction. Et un header n'inclut que ce dont **ses** déclarations ont besoin. |
| 7 | **Inversion des arguments** : header `(destination, url)` vs appel `(url, destination)` | Même type des deux côtés → **le compilateur ne peut rien voir**. Il vérifie les *types*, pas le *sens*. |
| 8 | Fonction `WriteCallback` définie **dans** une autre fonction | Pas de fonctions imbriquées en C. Niveau fichier uniquement. |
| 9 | `#include <curl/curl.h>` manquant → **20 erreurs** | Les erreurs cascadent. Lire la **première**, corriger, recompiler. |
| 10 | `FILE *fopen(const char *d, const char *w);` au lieu d'appeler | Déclarer ≠ appeler. Types vs valeurs. |
| 11 | `if (destination == NULL)` au lieu de `if (f == NULL)` | Tester ce que la fonction **a rendu**, pas ce qu'on lui a donné. |
| 12 | `fclose;` sans parenthèses | Les parenthèses font l'appel. Attrapé par `-Wunused-value`. |
| 13 | `fclose(f)` **avant** le `perform`, puis une 2e fois après | Dangling pointer + double `fclose`. Le fichier doit rester ouvert pendant tout le `perform`. |
| 14 | `curl_easy_strerror(res);` seule sur sa ligne | Elle **retourne** la chaîne, elle ne l'affiche pas. Le compilateur ne dit rien. |
| 15 | `return false` sans libérer `f` ni `curl` sur les chemins d'erreur | Inévitable quand on duplique le nettoyage sur N portes → `goto cleanup`. |
| 16 | Code HTTP récupéré **dans** la branche `res != CURLE_OK` | Les deux tests répondent à deux questions différentes, dans cet ordre. |
| 17 | `%l` au lieu de `%ld` | `l` est un modificateur, il faut la lettre du type derrière. Attrapé par `-Wall`. |
| 18 | `goto cleanup;` juste avant `ok = true;` | **Une étiquette ne bloque pas l'exécution.** On tombe dedans. Aucun `goto` sur le chemin du succès. |
| 19 | Mauvais jeu de données (`HP` au lieu de `IP`) | Vérifier le **contenu** des données, pas seulement que le téléchargement marche. |

| 20 | `memcmp(header, 0, 6)` pour comparer **un** octet à une valeur | `memcmp` compare des **suites d'octets** (2 adresses + une longueur). Pour un seul octet, qui est un **nombre**, on utilise `==`. Et le 3ᵉ argument est une **longueur**, pas une position. |
| 21 | `"0"` au lieu de `0` | `'0'` = le caractère (48 en ASCII), `"0"` = une chaîne de 2 octets, `0` = le nombre. En binaire on veut presque toujours le nombre. |
| 22 | `for (i = 8; i < 15; i++)` pour lire `header[8]`..`header[15]` | **Off-by-one.** `for (i = a; i < b; i++)` tourne exactement `b - a` fois. Pour 8 octets à partir de l'indice 8 : `b = 16`. Convention de l'intervalle semi-ouvert `[a, b)`. |
| 23 | `PRIu64` sans `#include <inttypes.h>` | Compilait par chance (macOS le tire indirectement). Casserait ailleurs. |
| 24 | `int fseek(f, offset, SEEK_SET);` | Le **type devant** transforme l'appel en **déclaration**. Repère : un type doit être suivi d'un **nom de variable** et d'un `=`. S'il est suivi du nom de la fonction, tu déclares. Et on ne déclare **jamais** une fonction de la libc : `#include` l'a fait. |
| 25 | `total = offset;` au lieu de `offset += total;` | Inversion. Les deux variables ont le même type → invisible pour le compilateur. Conséquence : `offset` ne bouge jamais → **boucle infinie**. |
| 26 | `goto cleanup` sur la fin de fichier dans une boucle | La **même** condition technique (lecture courte) est une erreur ou un succès **selon le contexte**. En fin de boucle, c'est un `break`. |

### Méta-leçons

1. **Lire la première erreur, pas la liste.** Corriger, recompiler, recommencer.
2. **Lire les warnings.** `-Wall -Wextra` ont attrapé les pièges 12 et 17 tout seuls.
3. **« Ça compile » ≠ « c'est correct ».** Pièges 7, 10, 13, 14, 18 compilent parfaitement.
4. **Avancer par micro-étapes, compiler après chacune.** Un mur de 20 erreurs paralyse ; une erreur à la fois se règle.
5. **Toujours garder un terrain qui compile.** Quand plus rien ne compile, on est perdu.
6. *Make it work, make it right, make it fast* — dans cet ordre.

---

## 17. Commandes utiles

### Build & exécution

```bash
rm -rf build                      # OBLIGATOIRE si on change de langage (le cache CMake garde CXX)
cmake -B build
cmake --build build
./build/cloud ; echo "sortie : $?"

cmake --build build 2>&1 | head -20    # ne voir que les premières erreurs
```

### Inspecter un binaire

```bash
file model.grib2                  # identifier le type
ls -lh model.grib2                # taille lisible
xxd model.grib2 | head -3         # hexdump + ASCII
xxd -s 16 -l 32 model.grib2       # 32 octets à partir de l'offset 16
```

### Comprendre la compilation

```bash
clang -E -Iinclude src/main.c | wc -l    # sortie du préprocesseur
man fwrite                                # LA doc de référence
man curl_easy_setopt
```

### Git

```bash
git show <commit>:src/downloader.c       # récupérer une ancienne version d'un fichier
git log --oneline
```

### Options de compilation recommandées

```cmake
target_compile_options(cloud PRIVATE -Wall -Wextra -Wunreachable-code)
```

---

## 18. État d'avancement

### ✅ Fait

- `CMakeLists.txt` en C17, `-Wall -Wextra`, libcurl liée
- **`cloud_download(destination, url)`** — téléchargement durci :
  - `fopen` + test `NULL`
  - `curl_easy_init` + test `NULL`
  - `curl_easy_perform` + test du `CURLcode`
  - test du **code HTTP** (`CURLINFO_RESPONSE_CODE`)
  - `goto cleanup`, un seul `return`
  - codes de sortie honnêtes dans `main`
- Testé dans les deux sens : URL valide → GRIB2 de 70 Mo ; URL bidon → `404`, code de sortie 1
- Décodage manuel de l'en-tête GRIB2 (magic, discipline, édition, longueur, centre, date)
- Constat : 20 messages dans le fichier, mais **mauvais paquet** (`HP` : 10/20/50/100 m)

- **`grib_inspect(path)`** dans `src/grib.c` — **section 0 entièrement décodée** :
  - `fopen("rb")` + test `NULL`, `goto cleanup`
  - `uint8_t header[16]` + `fread`, retour comparé à 16
  - `memcmp(header, "GRIB", 4)` → nombre magique validé
  - `header[6]` → discipline (0 = météo), `header[7]` → édition (2)
  - `header[8..15]` → **longueur du message en big-endian** = 3 920 693 octets ✅
  - l'affichage est fait par l'appelant (`main`), pas par la fonction → bonne séparation

- **Marcheur de messages** ✅ : boucle `fseek(f, offset, SEEK_SET)` + accumulateur `offset += longueur`.
  - **20 messages trouvés**, et la **somme des longueurs = taille du fichier à l'octet près** (72 091 291) → le parser est validé par le fichier lui-même.
  - Sortie de boucle par `break` quand `fread` rend moins que 16 (fin de fichier = succès, pas erreur).
  - ⚠️ Ne **jamais** utiliser `feof()` comme condition de boucle : il ne devient vrai qu'*après* une lecture échouée → un tour de trop.

### 🚧 En cours

- Trouver l'URL du bon paquet (**`IP`** / niveaux isobares)

### ⏭️ Ensuite

1. Descendre dans les sections 1..7 (chacune : longueur sur 4 octets + numéro de section sur 1 octet)
3. Section 4 → filtrer sur l'humidité (catégorie 1), lire le type et la valeur du niveau
4. Section 7 → extraire les valeurs empaquetées (décalages de bits, masques)
5. Puis : grille 3D, interpolation/krigeage, Marching Cubes, OpenGL

### 🔧 Petites dettes

- Sur échec, un `model.grib2` pourri reste sur le disque (page d'erreur HTML) → `remove(destination)` dans le `cleanup` quand `ok == false`.
  ⚠️ **Fermer le fichier AVANT de le supprimer** (`fclose` puis `remove`) : supprimer un fichier ouvert « marche » sur POSIX mais échoue sur Windows.
- **Écriture atomique** (à faire plus tard) : `fopen(..., "wb")` **tronque le fichier dès l'ouverture** — un échec détruit donc l'ancien fichier valide *avant* de savoir si le nouveau arrivera (c'est comme ça que les 70 Mo ont été perdus). L'idiome correct :
  1. télécharger vers `model.grib2.tmp`
  2. succès → `rename("model.grib2.tmp", "model.grib2")` — **atomique** sur un même système de fichiers
  3. échec → `remove("model.grib2.tmp")`

  Protège aussi du fichier tronqué par un Ctrl-C, qui *ressemble* à un GRIB valide (bons magic bytes, longueur annoncée fausse) et fait croire à un bug du parser.
  Nécessite de construire un nom de fichier → `snprintf` + tampon.
- `write_callback` inutilisé dans `downloader.c` (libcurl fait le travail par défaut) → à supprimer
- Réutiliser le handle `CURL *` quand on bouclera sur les altitudes (→ `struct`)
