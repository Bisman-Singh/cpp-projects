# Mini Shell

A basic Unix shell implementation in C++ with support for pipes, redirection, and built-in commands.

## Features

- Execute external commands via `fork`/`exec`
- Built-in commands: `cd`, `pwd`, `exit`, `help`, `history`
- Command history (last 20 commands)
- Pipe support (`cmd1 | cmd2 | cmd3`)
- Output redirection (`cmd > file`, `cmd >> file`)
- Input redirection (`cmd < file`)
- Custom prompt showing current working directory
- Cross-platform (macOS and Linux)

## Build

```bash
make
```

## Usage

```bash
./minishell
```

### Examples

```
/home/user $ ls -la
/home/user $ cat file.txt | grep pattern | wc -l
/home/user $ echo hello > output.txt
/home/user $ sort < input.txt > sorted.txt
/home/user $ history
/home/user $ exit
```

## Clean

```bash
make clean
```
