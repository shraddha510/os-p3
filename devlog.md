# Development Log

## November 24, 2024 - Initial Project Setup and Requirements Analysis

### Thoughts So Far
Today I'm starting to understand the requirements for the B-tree index file project. Here's what I've learned:

Core Project Requirements:
   - Need to build an interactive program that manages B-tree index files
   - Each block is 512 bytes
   - File format is very specific with 8-byte integers in big-endian format
   - Can only have 3 nodes in memory at any time (this will be challenging!)

B-tree Structure:
   - Minimal degree is 10
   - Each node can hold up to 19 key/value pairs
   - Each node has up to 20 child pointers
   - All numbers are 64-bit (8 bytes) in big-endian format

File Structure:
   - First block is header (512 bytes)
     * Magic number "4337PRJ3" (8 bytes)
     * Root node block ID (8 bytes)
     * Next available block ID (8 bytes)
   - Each node block has:
     * Block ID (8 bytes)
     * Parent block ID (8 bytes)
     * Number of current key/value pairs (8 bytes)
     * 19 keys (152 bytes)
     * 19 values (152 bytes)
     * 20 child pointers (160 bytes)

Required Commands:
   - create: Make new index file
   - open: Open existing file (check magic number)
   - insert: Add key/value pair
   - search: Find value by key
   - load: Read pairs from file
   - print: Show all pairs
   - extract: Save pairs to file
   - quit: Exit program

### Initial Setup Steps Completed
- Created new git repository
- Created this devlog.md file

### Plan for Next Session
- Choose programming language (leaning toward Python for easier file/byte handling)
- Create basic project structure:
   - Main program file
   - Class file for B-tree operations
   - Class file for block management
   - Or keep everything in one file? (unsure)
- Start implementing the block file I/O operations

### Questions/Concerns
- Memory Management:
   - Need to carefully track which nodes are in memory
   - Need strategy for choosing which nodes to remove when loading new ones
   - Maybe implement a simple node cache?

- Implementation Decisions Needed:
   - How to handle file overwrite confirmations
   - Best way to organize the code for maintainability
   - How to implement efficient tree traversal with memory constraints

- Testing Strategy:
   - Need to plan how to test B-tree operations
   - Need to verify byte ordering is correct
   - Need to test edge cases with file handling

### Next Steps
- Research Python's struct module for handling binary data
- Look up examples of B-tree implementations for reference
- Plan out the class structure in more detail

