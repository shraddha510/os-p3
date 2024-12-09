# Development Log

## November 24, 2024 12 PM - Initial Project Setup and Requirements Analysis

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

## December 7, 2024 10 AM

### Thoughts Since Last Session
After reviewing the previous analysis, I've had more thoughts about the implementation:

- The 3-node memory limit will require careful planning for tree traversal
- Need to ensure proper byte ordering for file compatibility
- Should implement a block cache system to track which nodes are in memory

### Changes/Development Since Last Session

- Reviewed project requirements in detail
- Better understanding of the memory constraints and their implications
- Identified that Python would be good choice for implementation due to:

- Built-in struct module for binary data handling
- Easy file I/O with byte operations
- Simple implementation of big-endian conversions with .to_bytes()

### Plan for Today's Session

Create basic program structure:

class BlockManager:
    # Handle reading/writing blocks
    # Manage 3-node memory limit
    pass

class BTreeNode:
    # Node operations
    # Key/value storage
    # Child pointer management
    pass

class IndexFile:
    # File operations
    # Header management
    pass

def main():
    # Command loop
    # User interface
    pass

Begin implementing core functionality:

- Set up the command loop structure
- Implement basic file operations (create/open)
- Create functions for reading/writing blocks

Begin implementing core functionality:

- Set up the command loop structure
- Implement basic file operations (create/open)
- Create functions for reading/writing blocks

### Questions to Resolve

Block Management:

- How to handle block allocation efficiently?
- Best way to track free/used blocks?
- Strategy for node splitting when full?

Error Handling:

- How to handle file corruption?
- What to do if magic number check fails?
- How to recover from failed operations?

## December 7, 2024 5 PM

I am going to change from python to C.

I would like to gain a better understanding of all that is happening at a lower level and feel more comfortable with C.

## December 7, 2024 6 PM 

### Thoughts so far

I am going to start by creating the header file for the btree.c file

I am having trouble getting started with btree.c but I will just start with the implementation of create?

### Plan for this session

- Header file
  - Write up all the expectations detailed in the Project Description
- Create a btree.c file where I start implementing all the specified functions.

## December 7, 2024 8 PM 

### Thoughts so far

I have been reading about B-Trees and the endian stuff so I'm gaining a clearer picture on how to move forward.

I'm going to start by adding the menu.

### Plan for this session

Create function (here are my thoughts so far)-

Algorithm: createTree
- Open file in write/binary mode ("wb+")
- If file open fails:
   - Return error

- Initialize tree structure:
   - Set file pointer
   - Set is_open flag
   - Copy magic number
   - Set root_block_id to 0 (empty tree)
   - Set next_block_id to 1

- Write header block:
   - Create 512-byte buffer
   - Write magic number (first 8 bytes)
   - Convert root_block_id to big-endian
   - Convert next_block_id to big-endian
   - Write to block 0
   - If write fails:
      - Close file
      - Return error

- Initialize node cache:
   - Clear cache entries
   - Set count to 0

- Return success

### Challenges

I am struggling with implementing the B-Trees. Splitting and Parent-Child relationship is confusing to me.

## December 7, 2024 11 PM 

### Thoughts so far

I have gotten a clearer understanding. 

### Plan for this session

I'm going to start creating a BTree struct in my header file. It will contain:
- File access
- Header information
- Status tracking

Following that, I will create a simple outline of creating, opening and closing a btree in my btree.c file.

I am going to include the endianness functions from the Project Description as well.

### Challenges

Confused about different file modes ("w+", "wb+", "r+", "rb+")

Solutions:
Changed to "wb+" for create_btree
Used "rb+" for open_btree
i have learned the importance of binary mode for cross-platform compatibility

## December 8, 2024 12 AM 

### Thoughts so far

I will be implementing the create and open functions in main and testing if these work. Maybe also close or insert.

### Plan for this session

1. Write a 512-byte block to a specific location in file

Algorithm:
Calculate file position (block_id * BLOCK_SIZE)
Seek to position
Write entire block (512 bytes)
Flush to ensure data is written
Return success/failure

Error Handling:
- Check seek operation success
- Verify complete write
- Ensure data is flushed to disk

2. Read a 512-byte block from a specific location in file

Algorithm:
Calculate file position (block_id * BLOCK_SIZE)
Seek to position
Read entire block (512 bytes)
Return success/failure

Error Handling:
- Check seek operation success
- Verify complete read

3. Header

-> Write B-tree header information to first block

Algorithm:
Create zeroed 512-byte buffer
Write magic number (first 8 bytes)
Convert root_block_id to big-endian
Convert next_block_id to big-endian
Write to block 0
Return success/failure

Layout:
- Bytes 0-7:   Magic Number
- Bytes 8-15:  Root Block ID
- Bytes 16-23: Next Block ID

-> Read and valdate B-tree header from first block

Algorithm:
Read block 0
Extract magic number
Convert root_block_id from big-endian
Convert next_block_id from big-endian
Return success/failure

Error Handling:
- Check for read success
- Validate magic number

### Challenges

will add tomorrow

## December 8, 2024 12 PM

### Thoughts so far

I coded a lot last night and I will summarize the design process and challenges which I had laid out on paper in my notebook:

Create:
   - Test creating new file
   - Verify header contents
   - Check error conditions

Open:
   - Test opening valid file
   - Test invalid file
   - Test non-existent file

Close:
   - Test normal close
   - Test double close
   - Test NULL pointer

I have been testing with basic creation/close and insertion of a file. Currently everything prints out correctly.
The magic number appears and then does inserted elements.

I updated the main function to contain all functionalities of the program thusfar.


### Plan for this session

I will implement the rest of the requirements in the project description such as extract, load, search, etc.

I am creating the functions within btree.c that will create a new node, and check if a node is a leaf.

### Challenges

I am having trouble with the split node function because there is a lot of shifting between the keys and children. 
I am going to lay out the design for split node function.


## December 8, 2024 3 PM

### Thoughts so far

Working on the split node function

### Design

Before Split (t=10):
Node: [k1,k2,k3,...,k19] (full with 19 keys)

After Split:
Left:  [k1,k2,...,k9]
Middle: k10 (moves up to parent)
Right: [k11,k12,...,k19]

Split child node:

Left half keeps first 9 keys (MAX_KEYS/2)
Middle key moves to parent
Right half gets remaining 9 keys

Memory management:

Created temporary nodes (child and new_node)
Zero-initialized to prevent garbage data
Careful tracking of block IDs

Key redistribution:

// Moving keys to new node
for (int i = 0; i < MAX_KEYS/2; i++) {
    new_node.keys[i] = child.keys[i + MAX_KEYS/2];
    // Clear old location
    child.keys[i + MAX_KEYS/2] = 0;
}

ohhhhhhhh
i was forgetting to add 1.

***testing if split node works fine***
it does!

--> Now i will update and finalize the write node and print node recursive functions


errors: implicit declaration of functions

Function: write_node_recursive
Export B-tree nodes to a file in a format suitable for later import

Algorithm:
Base Case Check:
   IF block_id = 0 THEN
       RETURN  // Empty subtree
   
Node Processing:
   node = read_node(block_id)  // Load node from disk
   
Data Writing:
   FOR each key i in node.keys (0 to num_keys-1):
       WRITE node.keys[i], node.values[i] to file
       ADD newline
   
Child Traversal:
   IF node is not leaf THEN
       FOR each child i (0 to num_keys):
           write_node_recursive(file, tree, node.children[i])

Function: print_node_recursive
Display B-tree structure with visual hierarchy

Algorithm:
Base Case Check:
   IF block_id = 0 THEN
       RETURN  // Empty subtree
   
Node Processing:
   node = read_node(block_id)  // Load node from disk
   
Data Display:
   FOR each key i in node.keys (0 to num_keys-1):
       PRINT level * 2 spaces (indentation)
       PRINT key[i] and value[i]
   
Child Traversal:
   IF node is not leaf THEN
       FOR each child i (0 to num_keys):
           print_node_recursive(tree, node.children[i], level + 1)


         [10]
        /    \
    [5,8]    [15,20]
    
Start at root (10)
   - Write: "10,value"
   - Process left child
Process [5,8]
   - Write: "5,value"
   - Write: "8,value"
Process [15,20]
   - Write: "15,value"
   - Write: "20,value"

Output file:
10,value1
5,value2
8,value3
15,value4
20,value5

print_node_recursive traversal:

1. Start at root (10)
   Level 0: "Key: 10, Value: value1"
   
2. Process [5,8]
   Level 1: "  Key: 5, Value: value2"
   Level 1: "  Key: 8, Value: value3"
   
3. Process [15,20]
   Level 1: "  Key: 15, Value: value4"
   Level 1: "  Key: 20, Value: value5"

Console output:
  Key: 10, Value: value1
  Key: 5, Value: value2
  Key: 8, Value: value3
  Key: 15, Value: value4
  Key: 20, Value: value5

Function: validate_node(tree, block_id, min_key, max_key)
Verify B-tree properties are maintained

Algorithm:
Base Case:
   IF block_id = 0 THEN
       min_key = 0
       max_key = 0
       RETURN true

Property Checks:
   - Key ordering
   - Node filling
   - Child relationships
   - Parent pointers

## December 8, 2024 6 PM

### Thoughts so far

So far I have been able to fix the errors at hand dealing with creating a node and printing. The split node function caused some issues but the code is working up until now.

### Working on:

Caching nodes
B-Tree operations
   -Create new node
   -Move half of keys
   -Update parent
   -Handle children
   -Write all changes

### Debugging:
Found issue with pointer updates
Fixed key distribution
Added validation checks

### Challenges:

-Cache replacement strategy

   -Decided on FIFO for simplicity
   -Must handle dirty nodes properly
   -Need to track node ages

-Memory management

   -Ensuring no memory leaks
   -Proper cleanup on close
   -Handling allocation failures

## December 8, 2024 7 PM

###Thoughts so far:

### What im working on:

Working on:
-Implementing the tree and node validation
   1. All leaves must be at same level
   2. Nodes must have keys in ascending order
   3. Children between keys k1 and k2 must have all keys in range (k1,k2)
   4. Non-leaf nodes with n keys must have n+1 children
   5. Node must not exceed MAX_KEYS

Started with helper function to validate individual nodes.

Updating main.c to catch up with all changes in btree.c

### Challenges Faced:

Range Validation:

   - Had to carefully track valid ranges for each subtree
   - Edge cases with minimum and maximum values
   - Properly handling leaf nodes vs internal nodes

Test Cases:
   - Empty tree
   - Single node
   - Multiple levels
   - Invalid structures:
      - Out of order keys
      - Invalid children
      - Wrong key ranges

STRUGGLING right now to get everything to work.

I will complete btree.c and main.c to get it to work before moving on!!


## December 8, 2024 7:30 PM

Things are still not working but I think it is because I have not linked the functions properly between main.c and btree.c

I am going to complete btree.c with finalizing tree statistical information and then try to make sure both files are properly aligned.

--> get_tree_stats

Initialize statistics:
   Set height = 0
   Set total_nodes = 0
   Set total_keys = 0

Validate tree state:
   IF tree is not open OR root block is 0
      RETURN (empty tree)

Begin recursive count:
   Start at root node
   Initial level = 1
   Call count_nodes_recursive

--> count_nodes_recursive 

Base case:
   IF block_id is 0
      RETURN

Process current node:
   Read node from disk/cache
   Increment total_nodes
   Add node.num_keys to total_keys
   Update height if current level is deeper

Recursive case (if not leaf):
   FOR each child (0 to num_keys):
      Recurse with:
         - child's block_id
         - level + 1
         - same statistics pointers

Maintain statistics:
   Update counters during traversal
   Track maximum depth reached

--> get_cache_stats

Get cache count:
   num_cached = current number of cached nodes

Count dirty nodes:
   Initialize dirty_count = 0
   FOR each cached node:
      IF node is marked dirty
         Increment dirty_count

Return statistics:
   Set num_cached pointer
   Set num_dirty pointer

### Challenges

#### Problem 1:
- Tree height was reporting incorrectly
- Sometimes off by one level
- Inconsistent with actual tree depth

Added debug to print statements
printf("Current level: %d, Max height: %d\n", level, *height);

Level counting started at 1
But was subtracting 1 unnecessarily
Root node was at level 1, not 0

// Fixed height tracking
if (level > *height)
    *height = level;  // Keep actual level

#### Problem 2:
- Random crashes during traversal
- Statistics occasionally wrong
- Memory corruption errors

// Added parameter validation
static void count_nodes_recursive(...) {
    if (!tree || !height || !total_nodes || !total_keys) {
        return;  // Prevent null pointer crashes
    }}

Root Cause:

Not zeroing node structure before reading
Garbage values causing issues
Memory leaks in recursion

Solution-
BTreeNode node = {0};  // Zero initialization
read_node(tree, block_id, &node);

#### Problem 3:
- Cache count didn't match reality
- Dirty nodes not properly tracked
- Cache eviction issues

Added cache state logging-
void print_cache_state() {
    printf("Cache entries: %d\n", node_cache.count);
    for (int i = 0; i < node_cache.count; i++) {
        printf("[%d] block=%llu, dirty=%d\n", 
               i, node_cache.entries[i].block_id,
               node_cache.entries[i].is_dirty);
    }
}

Solution:
void get_cache_stats(int *num_cached, int *num_dirty) {
    *num_cached = node_cache.count;
    *num_dirty = 0;
    
    // Clear tracking
    for (int i = 0; i < node_cache.count; i++) {
        if (node_cache.entries[i].is_dirty) {
            (*num_dirty)++;
        }
    }
}

## December 8, 2024 8:45 PM

### Issue
The issue right now is almost everything works except when i test it and this issue arises:

when i create two separate files, i can't access the first file.

Steps"
Add debug statements to verify magic number and header fields during file creation (`create_btree`) and file opening (`open_btree`).
Test the modified code to identify root causes of file-related issues.
Record results for further analysis.

#### Reflections:
Added debug outputs to `btree.c` in relevant locations for header validation. Moving to test changes in the next session.

solution:
corrected header initialization in create_btree
made sure the header is read and validated against the expected magic number before considering the file valid.
made sure that there was proper cache clearing on file wwitch

### UPDATE

I can now modify multiple files but i am now having an issue where after that if i add more entries it clears the entire thing.