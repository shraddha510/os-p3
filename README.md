# B-tree Index File Manager

This program implements a B-tree-based index file manager that allows for the creation, manipulation, and management of B-tree index files. The implementation maintains a maximum of 3 nodes in memory at any time and uses big-endian byte ordering for file storage.

## Project Structure

```
.
├── btree.h         # Header file containing data structures and function declarations
├── btree.c         # Implementation of B-tree operations
├── main.c          # Main program file with user interface
├── Makefile        # Build configuration
└── README.md       # This file
```

## Compilation Instructions

### Prerequisites
- GCC compiler
- Make utility

### Building the Program
1. Using Make:
```bash
make
```

2. Manual compilation:
```bash
gcc -Wall -g -c btree.c
gcc -Wall -g -c main.c
gcc -Wall -g -o btree btree.o main.o
```

3. Cleaning build files:
```bash
make clean
```

## Running the Program

Execute the compiled program:
```bash
./btree
```
