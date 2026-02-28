# Snake Game

A classic snake game that runs entirely in the terminal using ANSI escape codes for rendering. No external dependencies like ncurses required.

## Features

- Smooth terminal-based rendering with ANSI escape codes
- WASD and arrow key controls
- Snake grows when eating food
- Score tracking with live display
- Speed increases as score goes up
- Game over on wall collision or self collision
- Colored output: green snake, red food, cyan border
- Non-blocking input using termios (macOS/Linux)

## Build

```bash
make
```

## Usage

```bash
./snake
```

### Controls

| Key | Action |
|-----|--------|
| W / Up Arrow | Move up |
| A / Left Arrow | Move left |
| S / Down Arrow | Move down |
| D / Right Arrow | Move right |
| Q | Quit |
