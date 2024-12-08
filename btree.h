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

#endif /* BTREE_H */