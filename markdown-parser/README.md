# Markdown Parser

A Markdown-to-HTML converter written in C++17. Parses common Markdown syntax and outputs clean HTML.

## Features

- Headings (h1-h6)
- Bold (`**text**`) and italic (`*text*`)
- Inline code (`` `code` ``) and fenced code blocks (` ``` `)
- Unordered lists (`-`, `*`, `+`)
- Ordered lists (`1.`, `2.`, etc.)
- Links (`[text](url)`)
- Blockquotes (`> text`)
- Horizontal rules (`---`, `***`, `___`)
- Paragraphs with inline formatting
- HTML entity escaping

## Build

```bash
make
```

## Usage

Output HTML to stdout:

```bash
./mdparser example.md
```

Write HTML to a file:

```bash
./mdparser example.md -o output.html
```

## Clean

```bash
make clean
```
