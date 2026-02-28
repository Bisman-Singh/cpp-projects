# File Encryption Tool

A command-line tool for encrypting and decrypting files using XOR or Caesar ciphers.

## Features

- XOR cipher encryption/decryption with arbitrary string key
- Caesar cipher encryption/decryption with integer shift
- Buffered I/O for efficient processing of large files
- Progress indicator for files over 1 MB

## Build

```bash
make
```

## Usage

```bash
./encrypt <mode> <input> <output> <key>
```

### Modes

| Mode         | Description                |
|--------------|----------------------------|
| `xor-enc`    | Encrypt using XOR cipher   |
| `xor-dec`    | Decrypt using XOR cipher   |
| `caesar-enc` | Encrypt using Caesar cipher|
| `caesar-dec` | Decrypt using Caesar cipher|

### Examples

```bash
# XOR encrypt
./encrypt xor-enc secret.txt secret.enc mypassword

# XOR decrypt
./encrypt xor-dec secret.enc secret.txt mypassword

# Caesar encrypt with shift of 13
./encrypt caesar-enc message.txt message.enc 13

# Caesar decrypt
./encrypt caesar-dec message.enc message.txt 13
```

## Clean

```bash
make clean
```
