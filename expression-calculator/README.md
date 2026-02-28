# Expression Calculator

A mathematical expression evaluator in C++17 using the Shunting-Yard algorithm with proper operator precedence.

## Features

- Operators: `+`, `-`, `*`, `/`, `%` (modulo), `^` (power)
- Parentheses for grouping
- Unary minus (e.g., `-5`, `-(3+2)`)
- Floating point and integer arithmetic
- Interactive REPL mode
- Single-expression evaluation via CLI
- Verbose mode (`-v`) showing tokenization and RPN conversion steps
- Graceful error handling for division by zero, syntax errors, mismatched parentheses

## Build

```bash
make
```

## Usage

Interactive REPL:

```bash
./calc
```

Single expression:

```bash
./calc "2 + 3 * 4"
./calc "(1 + 2) ^ 3 - 10 / 2"
```

Verbose mode (shows tokenization and evaluation steps):

```bash
./calc -v "2 + 3 * 4"
./calc -v
```

## Examples

```
> 2 + 3 * 4
= 14
> (1 + 2) ^ 3
= 27
> -5 + 3
= -2
> 10 % 3
= 1
> 1 / 0
Error: Division by zero
```

## Clean

```bash
make clean
```
