# Student Database

An interactive student record management system in C++17 with file-based persistence.

## Features

- Add, view, search, update, and delete student records
- Student fields: ID, name, age, grade (A-F), GPA
- Search by ID (exact) or name (substring, case-insensitive)
- Sort by name, GPA, or ID
- Statistics: average GPA, highest/lowest GPA, grade distribution, count
- Binary file storage (`students.dat`) for persistent data
- Interactive menu-driven interface with input validation

## Build

```bash
make
```

## Usage

```bash
./studentdb
```

Follow the on-screen menu to manage student records. Data is automatically saved to `students.dat`.

## Clean

```bash
make clean
```

This removes the binary and the data file.
