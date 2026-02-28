# JSON Parser

A recursive descent JSON parser written in C++ that parses JSON strings into a typed data structure and pretty-prints them back.

## Features

- Parses all JSON types: null, bool, integer, float, string, array, object
- `JsonValue` class using `std::variant` for type-safe value storage
- Recursive descent parsing with clear error messages
- Pretty-print output with configurable indentation
- Handles escape sequences including `\uXXXX` unicode escapes
- Graceful error handling with position-aware messages
- Preserves object key insertion order

## Build

```bash
make
```

## Usage

```bash
./jsonparser
```

The program parses several built-in example JSON strings and pretty-prints the results, including an intentional error case to demonstrate error handling.
