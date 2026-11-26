# 42sh

Un shell POSIX écrit en C99, réalisé dans le cadre du projet **42sh** à l'EPITA.

`42sh` lit des commandes depuis l'entrée standard, une chaîne (`-c`) ou un fichier de script, puis les fait passer par un pipeline classique :
**lexer → parser (AST) → expansion → exécution**.

---

## Sommaire

- [Fonctionnalités](#fonctionnalités)
- [Compilation](#compilation)
- [Utilisation](#utilisation)
- [Tests](#tests)
- [Architecture](#architecture)
- [Roadmap](#roadmap)

---

## Fonctionnalités

### Modes d'exécution

| Mode        | Commande                    |
|-------------|-----------------------------|
| Interactif  | `./42sh` (prompt `42sh>`)   |
| Chaîne      | `./42sh -c "echo hello"`    |
| Script      | `./42sh script.sh`          |

### Grammaire

- Listes de commandes : `;` et retour à la ligne
- Opérateurs logiques : `&&`, `||`
- Pipelines : `cmd1 | cmd2`
- Sous-shells : `( ... )`
- Conditions : `if / then / elif / else / fi`
- Quotes simples `'...'`, doubles `"..."` et échappement `\`
- Commentaires `#`

### Redirections

| Opérateur | Effet                                   |
|-----------|-----------------------------------------|
| `>`       | Redirige la sortie (écrase le fichier)  |
| `>>`      | Redirige la sortie (ajoute à la fin)    |
| `<`       | Redirige l'entrée                       |
| `>&`      | Duplique un descripteur de sortie       |
| `<&`      | Duplique un descripteur d'entrée        |
| `<>`      | Ouvre un fichier en lecture/écriture    |

### Expansion de variables

- `$VAR` et `${VAR}`
- Variables spéciales : `$?`, `$$`, `$#`, `$@`, `$*`, `$RANDOM`, `$UID`

### Builtins

| Builtin  | Détails                                           |
|----------|---------------------------------------------------|
| `echo`   | Options `-n`, `-e`, `-E`                          |
| `cd`     | `cd`, `cd ~`, `cd -`, `cd ..`, met à jour `PWD` / `OLDPWD` |
| `export` | `export VAR=valeur`, `export -p`                  |
| `exit`   | Quitte le shell avec un code de retour            |
| `true`   | Retourne 0                                        |
| `false`  | Retourne 1                                        |

Toute autre commande est lancée via `fork` + `execvp`.

### Gestion mémoire

Un garbage collector maison (`src/include/garbage/`) libère toutes les allocations temporaires à la fin de chaque commande, pour éviter les fuites mémoire entre deux lignes.

---

## Compilation

### Prérequis

- `gcc` ou `clang` (C99)
- `autoconf`, `automake`
- `make`

### Build

```bash
autoreconf --install
```

```bash
./configure
```

```bash
make
```

Le binaire est généré dans `src/42sh`.

Le projet est compilé avec `-std=c99 -Wall -Wextra -Werror -pedantic`.

---

## Utilisation

```bash
./src/42sh
```

```
42sh>echo "Hello, world!"
Hello, world!
42sh>cd /tmp && pwd
/tmp
42sh>echo toto | tr a-z A-Z
TOTO
42sh>if false; then echo nope; elif true; then echo ok; fi
ok
```

En mode non interactif :

```bash
./src/42sh -c 'echo first > out.txt; echo second >> out.txt; cat out.txt'
```

---

## Tests

La suite de tests fonctionnels se trouve dans `tests/test_suite.sh`. Elle compare la sortie de `42sh` au résultat attendu pour les commandes simples, les redirections, les pipes, `cd`/`pwd` et les conditions `if`.

```bash
make check
```

Pour lancer aussi les tests unitaires :

```bash
make check COVERAGE=yes
```

---

## Architecture

```
42sh/
├── configure.ac            # Configuration autotools
├── Makefile.am
├── docs/
│   └── 42sh-subject.pdf    # Sujet du projet
├── src/
│   ├── main.c              # Lecture de l'entrée et boucle principale
│   ├── common.h            # État global du shell (ShellState)
│   └── include/
│       ├── lexer/          # Découpage de l'entrée en tokens
│       ├── parser/         # Construction de l'AST
│       │   └── commands/   # Règles if / for / while
│       ├── ast/            # Définition et libération des nœuds
│       ├── expand/         # Expansion des variables
│       ├── executor/       # Exécution de l'AST (fork, pipe, wait)
│       │   └── redirects/  # Gestion des redirections
│       ├── builtins/       # Commandes internes
│       └── garbage/        # Garbage collector
└── tests/
    └── test_suite.sh       # Tests fonctionnels
```

### Pipeline d'exécution

```
entrée ──► lexer ──► tokens ──► parser ──► AST ──► expand ──► executor
```

1. **Lexer** (`lexer.c`) : transforme la ligne en liste chaînée de tokens (mots, mots-clés, opérateurs, redirections) en gérant quotes, échappements et commentaires.
2. **Parser** (`parser.c`, `commands.c`) : descente récursive qui construit un AST avec les nœuds `COMMAND`, `PIPE`, `AND`, `OR`, `REDIRECT`, `SUBSHELL`, `CHAIN` et `IF`.
3. **Expansion** (`expand.c`) : parcourt l'AST et remplace les variables par leur valeur.
4. **Executor** (`executor.c`) : parcourt l'AST, appelle les builtins ou lance les programmes externes, branche les pipes et applique les redirections.

---

## Roadmap

- [x] Commandes simples et builtins de base
- [x] Pipes, `&&`, `||`, `;`
- [x] Redirections
- [x] Sous-shells
- [x] `if / elif / else`
- [x] Expansion des variables
- [ ] Boucles `while`, `until`, `for`
- [ ] Heredoc `<<`
- [ ] `unset`
- [ ] Fonctions
- [ ] Alias

---

Projet réalisé à l'EPITA (promo 2027).
