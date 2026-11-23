# CompilX

## Compiler Design & Construction Toolkit

CompilX is a menu-driven educational compiler design toolkit developed using C++17. It demonstrates fundamental compiler concepts through 13 integrated modules.

## Features

1. **Comment Removal** — Removes comments from source code.
2. **Lexical Analyzer** — Identifies keywords, identifiers, constants, operators, and special symbols.
3. **Formal Language & Automata** — Implements DFA and NFA, including epsilon transitions.
4. **Regular Expression to NFA/DFA** — Converts regular expressions into NFA and equivalent DFA.
5. **Left Factoring** — Extracts common prefixes from grammar productions.
6. **Left Recursion Elimination** — Eliminates immediate and indirect left recursion.
7. **FIRST Calculation** — Calculates FIRST sets.
8. **FOLLOW Calculation** — Calculates FOLLOW sets.
9. **LL(1) Parsing Table** — Constructs parsing tables and detects conflicts.
10. **Predictive Parsing** — Implements table-driven predictive parsing.
11. **Three Address Code** — Generates three-address code.
12. **Quadruple Generation** — Generates quadruple representations.
13. **Code Optimization** — Implements constant folding, constant propagation, algebraic simplification, copy propagation, common subexpression elimination, and dead code elimination.

## Technologies Used

- C++17
- GNU G++ (MinGW)
- C++ Standard Library
- Visual Studio Code / Code::Blocks
- PowerShell

## Project Structure

```text
CompilX/
├── main.cpp
├── build.ps1
├── run.ps1
├── README.md
├── .gitignore
├── include/
│   └── 13 module header files
└── src/
    └── 13 module implementation files
```

## Build Instructions

Open the project directory in the terminal and run:

```powershell
.\build.ps1
```

## Run Instructions

```powershell
.\run.ps1
```

Select a module from the main menu. Enter `0` to exit.

## Input Conventions

- Enter `END` on a separate line to finish source-code or grammar input.
- Grammar alternatives are separated by `|`.
- Use `epsilon` for an empty grammar production.
- Grammar non-terminals use uppercase letters and may have apostrophe suffixes.
- Regular expressions support union (`|`), explicit concatenation (`.`), Kleene star (`*`), one-or-more (`+`), and parentheses.

## Scope

CompilX is an educational project for demonstrating Compiler Design algorithms. It is not a complete production compiler and does not implement the full C++ language specification.

## Project Purpose

Developed to apply theoretical concepts from the Compiler Design course through practical C++ implementations.