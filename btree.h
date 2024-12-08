#ifndef BTREE_H
#define BTREE_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Size of each disk block in bytes.
 */
#define BLOCK_SIZE 512

/**
 * B-Tree order parameters:
 * - MAX_KEYS (19): Maximum number of keys per node, derived from minimal degree t=10
 * - MAX_CHILDREN (20): Maximum number of children per node (always MAX_KEYS + 1)
 *
 * These values ensure that:
 * - Each node (except root) is at least half full
 * - A non-leaf node with k keys must have k+1 children
 * - All leaves appear at the same level
 */
#define MAX_KEYS 19
#define MAX_CHILDREN (MAX_KEYS + 1)

/**
 * Magic number used to identify valid B-Tree files.
 * This helps prevent accidental processing of non-B-Tree files.
 */
#define MAGIC_NUMBER "4337PRJ3"

/**
 * B-Tree Node Structure
 * --------------------
 * In-memory representation of a B-Tree node containing:
 * - Metadata (block ID, parent reference)
 * - Keys and associated values
 * - Child pointers
 */
typedef struct
{
    uint64_t block_id;               // Unique identifier for this node's disk block
    uint64_t parent_block_id;        // Block ID of this node's parent (0 if root)
    uint64_t num_keys;               // Current number of keys stored in this node
    uint64_t keys[MAX_KEYS];         // Array of keys in ascending order
    uint64_t values[MAX_KEYS];       // Array of values corresponding to keys
    uint64_t children[MAX_CHILDREN]; // Block IDs of child nodes
} BTreeNode;

#endif /* BTREE_H */