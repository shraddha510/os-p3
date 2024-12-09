// btree.c
#include "btree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CACHED_NODES 3

typedef struct CacheNode
{
    uint64_t block_id;
    BTreeNode node;
    int is_dirty; // 1 if node needs to be written back to disk
} CacheNode;

typedef struct NodeCache
{
    CacheNode entries[MAX_CACHED_NODES];
    int count;
} NodeCache;

static NodeCache node_cache = {{0}, 0};

// Forward declarations for internal functions
static int is_leaf(BTreeNode *node);
static int write_block(FILE *fp, uint64_t block_id, const void *buf);
static int read_block(FILE *fp, uint64_t block_id, void *buf);
static int write_header(BTree *tree);
static int read_header(BTree *tree);
static int write_node(BTree *tree, BTreeNode *node);
static int read_node(BTree *tree, uint64_t block_id, BTreeNode *node);
static BTreeNode *create_node(BTree *tree);
static int split_child(BTree *tree, BTreeNode *parent, int child_index);
static int insert_nonfull(BTree *tree, BTreeNode *node, uint64_t key, uint64_t value);
static void write_node_recursive(FILE *fp, BTree *tree, uint64_t block_id);
static void print_node_recursive(BTree *tree, uint64_t block_id, int level);
static void count_nodes_recursive(uint64_t block_id, int level, int *height, int *total_nodes, int *total_keys, BTree *tree);

// Endianness conversion functions
static uint64_t to_big_endian(uint64_t value)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return __builtin_bswap64(value);
#else
    return value;
#endif
}

static uint64_t from_big_endian(uint64_t value)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return __builtin_bswap64(value);
#else
    return value;
#endif
}

// Cache management functions
static void init_node_cache()
{
    node_cache.count = 0;
    for (int i = 0; i < MAX_CACHED_NODES; i++)
    {
        node_cache.entries[i].block_id = 0;
        node_cache.entries[i].is_dirty = 0;
    }
}

static void clear_node_cache(BTree *tree)
{
    // Write any dirty nodes back to disk
    for (int i = 0; i < node_cache.count; i++)
    {
        if (node_cache.entries[i].is_dirty)
        {
            write_node(tree, &node_cache.entries[i].node);
            node_cache.entries[i].is_dirty = 0;
        }
    }
    // Reset cache
    init_node_cache();
}

static BTreeNode *get_cached_node(uint64_t block_id)
{
    for (int i = 0; i < node_cache.count; i++)
    {
        if (node_cache.entries[i].block_id == block_id)
        {
            return &node_cache.entries[i].node;
        }
    }
    return NULL;
}

static void mark_node_dirty(uint64_t block_id)
{
    for (int i = 0; i < node_cache.count; i++)
    {
        if (node_cache.entries[i].block_id == block_id)
        {
            node_cache.entries[i].is_dirty = 1;
            break;
        }
    }
}

static BTreeNode *cache_node(BTree *tree, uint64_t block_id)
{
    // First check if node is already cached
    BTreeNode *cached = get_cached_node(block_id);
    if (cached)
        return cached;

    // If cache is full, write out the oldest entry
    if (node_cache.count == MAX_CACHED_NODES)
    {
        if (node_cache.entries[0].is_dirty)
        {
            write_node(tree, &node_cache.entries[0].node);
        }
        // Shift remaining entries
        for (int i = 1; i < MAX_CACHED_NODES; i++)
        {
            node_cache.entries[i - 1] = node_cache.entries[i];
        }
        node_cache.count--;
    }

    // Add new node to cache
    int idx = node_cache.count++;
    node_cache.entries[idx].block_id = block_id;
    node_cache.entries[idx].is_dirty = 0;

    // Read node from disk
    if (read_node(tree, block_id, &node_cache.entries[idx].node) != 0)
    {
        node_cache.count--;
        return NULL;
    }

    return &node_cache.entries[idx].node;
}

static int check_duplicate_key(BTree *tree, uint64_t key)
{
    if (tree->header.root_block_id == 0)
    {
        return 0; // Empty tree, no duplicates
    }

    BTreeNode *node;
    uint64_t current_block = tree->header.root_block_id;

    while (current_block != 0)
    {
        node = cache_node(tree, current_block);
        if (!node)
            return -1; // Error reading node

        // Search for key in current node
        for (int i = 0; i < node->num_keys; i++)
        {
            if (key == node->keys[i])
            {
                return -1; // Duplicate found
            }
            if (key < node->keys[i])
            {
                current_block = node->children[i];
                goto next_iteration;
            }
        }
        current_block = node->children[node->num_keys];

    next_iteration:
        continue;
    }

    return 0; // No duplicate found
}

// Helper function to check if node is a leaf
static int is_leaf(BTreeNode *node)
{
    return node->children[0] == 0;
}

// Create a new node
static BTreeNode *create_node(BTree *tree)
{
    BTreeNode *node = (BTreeNode *)calloc(1, sizeof(BTreeNode));
    if (!node)
        return NULL;

    node->block_id = tree->header.next_block_id++;
    write_header(tree);

    return node;
}

// Split child node when full
static int split_child(BTree *tree, BTreeNode *parent, int child_index)
{
    BTreeNode child = {0}, new_node = {0};
    read_node(tree, parent->children[child_index], &child);

    new_node.block_id = tree->header.next_block_id++;
    new_node.parent_block_id = parent->block_id;
    new_node.num_keys = MAX_KEYS / 2;

    // Copy second half of child's keys and values to new node
    for (int i = 0; i < MAX_KEYS / 2; i++)
    {
        new_node.keys[i] = child.keys[i + MAX_KEYS / 2 + 1];
        new_node.values[i] = child.values[i + MAX_KEYS / 2 + 1];
        child.keys[i + MAX_KEYS / 2 + 1] = 0;
        child.values[i + MAX_KEYS / 2 + 1] = 0;
    }

    // If not leaf, copy relevant children
    if (!is_leaf(&child))
    {
        for (int i = 0; i <= MAX_KEYS / 2; i++)
        {
            new_node.children[i] = child.children[i + MAX_KEYS / 2 + 1];
            child.children[i + MAX_KEYS / 2 + 1] = 0;
        }
    }

    child.num_keys = MAX_KEYS / 2;

    // Move parent's keys and children to make room
    for (int i = parent->num_keys; i > child_index; i--)
    {
        parent->keys[i] = parent->keys[i - 1];
        parent->values[i] = parent->values[i - 1];
        parent->children[i + 1] = parent->children[i];
    }

    // Add middle key to parent
    parent->keys[child_index] = child.keys[MAX_KEYS / 2];
    parent->values[child_index] = child.values[MAX_KEYS / 2];
    parent->children[child_index + 1] = new_node.block_id;
    parent->num_keys++;

    // Write all modified nodes
    write_node(tree, parent);
    write_node(tree, &child);
    write_node(tree, &new_node);
    write_header(tree);

    return 0;
}

// Insert into a non-full node
static int insert_nonfull(BTree *tree, BTreeNode *node, uint64_t key, uint64_t value)
{
    int i = node->num_keys - 1;

    if (is_leaf(node))
    {
        while (i >= 0 && key < node->keys[i])
        {
            node->keys[i + 1] = node->keys[i];
            node->values[i + 1] = node->values[i];
            i--;
        }

        node->keys[i + 1] = key;
        node->values[i + 1] = value;
        node->num_keys++;

        // Write the modified node back to disk
        int result = write_node(tree, node);
        if (result == 0)
        {
            fflush(tree->fp);
        }
        return result;
    }
    else
    {
        while (i >= 0 && key < node->keys[i])
        {
            i--;
        }
        i++;

        BTreeNode child = {0};
        read_node(tree, node->children[i], &child);

        if (child.num_keys == MAX_KEYS)
        {
            split_child(tree, node, i);
            if (key > node->keys[i])
            {
                i++;
                read_node(tree, node->children[i], &child);
            }
        }

        insert_nonfull(tree, &child, key, value);
    }

    return 0;
}

// Main insert function
int insert_key(BTree *tree, uint64_t key, uint64_t value)
{
    if (!tree->is_open)
        return -1;

    int result;
    if (tree->header.root_block_id == 0)
    {
        BTreeNode *root = create_node(tree);
        if (!root)
            return -1;

        root->keys[0] = key;
        root->values[0] = value;
        root->num_keys = 1;
        tree->header.root_block_id = root->block_id;

        result = write_node(tree, root);
        if (result == 0)
        {
            write_header(tree);
            fflush(tree->fp);
        }

        free(root);
        return result;
    }

    // Create a temporary node for the root
    BTreeNode root = {0};
    result = read_node(tree, tree->header.root_block_id, &root);
    if (result != 0)
        return result;

    // Now pass the node struct instead of the block_id
    result = insert_nonfull(tree, &root, key, value);
    if (result == 0)
    {
        fflush(tree->fp);
    }
    return result;
}

// Search function
int search_key(BTree *tree, uint64_t key, uint64_t *value)
{
    if (!tree->is_open || tree->header.root_block_id == 0)
    {
        return -1;
    }

    BTreeNode node = {0};
    uint64_t current_block = tree->header.root_block_id;

    while (current_block != 0)
    {
        read_node(tree, current_block, &node);

        for (int i = 0; i < node.num_keys; i++)
        {
            if (key == node.keys[i])
            {
                *value = node.values[i];
                return 0;
            }
            if (key < node.keys[i])
            {
                current_block = node.children[i];
                goto next_iteration;
            }
        }

        current_block = node.children[node.num_keys];

    next_iteration:
        continue;
    }

    return -1; // Key not found
}

// Block I/O operations
static int write_block(FILE *fp, uint64_t block_id, const void *buf)
{
    if (fseek(fp, block_id * BLOCK_SIZE, SEEK_SET) != 0)
    {
        return -1;
    }
    if (fwrite(buf, 1, BLOCK_SIZE, fp) != BLOCK_SIZE)
    {
        return -1;
    }
    fflush(fp);
    return 0;
}

static int read_block(FILE *fp, uint64_t block_id, void *buf)
{
    if (fseek(fp, block_id * BLOCK_SIZE, SEEK_SET) != 0)
    {
        return -1;
    }
    if (fread(buf, 1, BLOCK_SIZE, fp) != BLOCK_SIZE)
    {
        return -1;
    }
    return 0;
}

// Header I/O operations
static int write_header(BTree *tree)
{
    unsigned char block[BLOCK_SIZE] = {0};
    memcpy(block, tree->header.magic, 8);

    uint64_t *fields = (uint64_t *)(block + 8);
    fields[0] = to_big_endian(tree->header.root_block_id);
    fields[1] = to_big_endian(tree->header.next_block_id);

    return write_block(tree->fp, 0, block);
}

static int read_header(BTree *tree)
{
    unsigned char block[BLOCK_SIZE];
    if (read_block(tree->fp, 0, block) != 0)
    {
        return -1;
    }

    memcpy(tree->header.magic, block, 8);
    uint64_t *fields = (uint64_t *)(block + 8);
    tree->header.root_block_id = from_big_endian(fields[0]);
    tree->header.next_block_id = from_big_endian(fields[1]);

    return 0;
}

// Node I/O operations
static int write_node(BTree *tree, BTreeNode *node)
{
    unsigned char block[BLOCK_SIZE] = {0};
    uint64_t *fields = (uint64_t *)block;

    // Pack the node data
    fields[0] = to_big_endian(node->block_id);
    fields[1] = to_big_endian(node->parent_block_id);
    fields[2] = to_big_endian(node->num_keys);

    for (int i = 0; i < MAX_KEYS; i++)
    {
        fields[3 + i] = to_big_endian(node->keys[i]);
        fields[3 + MAX_KEYS + i] = to_big_endian(node->values[i]);
    }

    for (int i = 0; i < MAX_CHILDREN; i++)
    {
        fields[3 + 2 * MAX_KEYS + i] = to_big_endian(node->children[i]);
    }

    // Write the block and flush
    int result = write_block(tree->fp, node->block_id, block);
    if (result == 0)
    {
        fflush(tree->fp);
    }
    return result;
}

int read_node(BTree *tree, uint64_t block_id, BTreeNode *node)
{
    BTreeNode *cached = get_cached_node(block_id);
    if (cached)
    {
        *node = *cached;
        return 0;
    }

    unsigned char block[BLOCK_SIZE];
    if (read_block(tree->fp, block_id, block) != 0)
    {
        return -1;
    }

    uint64_t *fields = (uint64_t *)block;
    node->block_id = from_big_endian(fields[0]);
    node->parent_block_id = from_big_endian(fields[1]);
    node->num_keys = from_big_endian(fields[2]);

    for (int i = 0; i < MAX_KEYS; i++)
    {
        node->keys[i] = from_big_endian(fields[3 + i]);
        node->values[i] = from_big_endian(fields[3 + MAX_KEYS + i]);
    }

    for (int i = 0; i < MAX_CHILDREN; i++)
    {
        node->children[i] = from_big_endian(fields[3 + 2 * MAX_KEYS + i]);
    }

    return 0;
}

// Tree operations
int create_btree(BTree *tree, const char *filename)
{
    // First close any currently open tree
    if (tree->is_open)
    {
        close_btree(tree);
    }

    FILE *fp = fopen(filename, "wb+");
    if (!fp)
        return -1;

    tree->fp = fp;
    tree->is_open = 1;
    memcpy(tree->header.magic, MAGIC_NUMBER, 8);
    tree->header.root_block_id = 0;
    tree->header.next_block_id = 1;

    init_node_cache(); // Reset the cache

    if (write_header(tree) != 0)
    {
        fclose(fp);
        tree->is_open = 0;
        return -1;
    }

    fflush(fp); // Force write to disk
    return 0;
}

int open_btree(BTree *tree, const char *filename)
{
    // First close any currently open tree properly
    if (tree->is_open)
    {
        close_btree(tree);
    }

    FILE *fp = fopen(filename, "rb+");
    if (!fp)
        return -1;

    tree->fp = fp;
    tree->is_open = 1;

    if (read_header(tree) != 0 || memcmp(tree->header.magic, MAGIC_NUMBER, 8) != 0)
    {
        fclose(fp);
        tree->is_open = 0;
        return -1;
    }

    return 0;
}

void close_btree(BTree *tree)
{
    if (tree->is_open)
    {
        // Write any dirty nodes in cache
        clear_node_cache(tree);

        // Ensure header is written
        write_header(tree);

        if (tree->fp)
        {
            fclose(tree->fp);
            tree->fp = NULL;
        }
    }
    tree->is_open = 0;
}

// Print function
void print_tree(BTree *tree)
{
    if (!tree->is_open || tree->header.root_block_id == 0)
    {
        printf("Tree is empty.\n");
        return;
    }

    printf("B-Tree Contents:\n");
    printf("---------------\n");
    print_node_recursive(tree, tree->header.root_block_id, 0);
}

// Data load/extract functions
int load_data(BTree *tree, const char *filename)
{
    if (!tree->is_open)
        return -1;

    FILE *fp = fopen(filename, "r");
    if (!fp)
        return -1;

    char line[256];
    int line_num = 0;
    while (fgets(line, sizeof(line), fp))
    {
        line_num++;
        uint64_t key, value;

        if (sscanf(line, "%llu,%llu",
                   (unsigned long long *)&key,
                   (unsigned long long *)&value) != 2)
        {
            printf("Warning: Invalid format at line %d\n", line_num);
            continue;
        }

        if (insert_key(tree, key, value) != 0)
        {
            printf("Warning: Failed to insert key %llu at line %d\n",
                   (unsigned long long)key, line_num);
        }
    }

    fclose(fp);
    return 0;
}

int extract_data(BTree *tree, const char *filename)
{
    if (!tree->is_open || tree->header.root_block_id == 0)
        return -1;

    FILE *fp = fopen(filename, "w");
    if (!fp)
        return -1;

    write_node_recursive(fp, tree, tree->header.root_block_id);

    fclose(fp);
    return 0;
}

static void write_node_recursive(FILE *fp, BTree *tree, uint64_t block_id)
{
    if (block_id == 0)
        return;

    BTreeNode node = {0};
    read_node(tree, block_id, &node);

    // Write current node's key-value pairs
    for (int i = 0; i < node.num_keys; i++)
    {
        fprintf(fp, "%llu,%llu\n",
                (unsigned long long)node.keys[i],
                (unsigned long long)node.values[i]);
    }

    // Recursively process children if not a leaf
    if (!is_leaf(&node))
    {
        for (int i = 0; i <= node.num_keys; i++)
        {
            write_node_recursive(fp, tree, node.children[i]);
        }
    }
}

static void print_node_recursive(BTree *tree, uint64_t block_id, int level)
{
    if (block_id == 0)
        return;

    BTreeNode node = {0};
    read_node(tree, block_id, &node);

    // Print current node with proper indentation
    for (int i = 0; i < node.num_keys; i++)
    {
        for (int j = 0; j < level; j++)
        {
            printf("  "); // Two spaces per level for indentation
        }
        printf("Key: %llu, Value: %llu\n",
               (unsigned long long)node.keys[i],
               (unsigned long long)node.values[i]);
    }

    // Recursively print children
    if (!is_leaf(&node))
    {
        for (int i = 0; i <= node.num_keys; i++)
        {
            print_node_recursive(tree, node.children[i], level + 1);
        }
    }
}

// Additional utility function to validate B-tree properties
static int validate_node(BTree *tree, uint64_t block_id, uint64_t *min_key, uint64_t *max_key)
{
    if (block_id == 0)
    {
        *min_key = 0;
        *max_key = 0;
        return 1;
    }

    BTreeNode node = {0};
    read_node(tree, block_id, &node);

    // Check number of keys
    if (node.num_keys < 0 || node.num_keys > MAX_KEYS)
    {
        return 0;
    }

    // Check key ordering
    for (int i = 1; i < node.num_keys; i++)
    {
        if (node.keys[i] <= node.keys[i - 1])
        {
            return 0;
        }
    }

    *min_key = node.keys[0];
    *max_key = node.keys[node.num_keys - 1];

    // Recursively validate children
    if (!is_leaf(&node))
    {
        uint64_t child_min, child_max;

        // Validate leftmost child
        if (!validate_node(tree, node.children[0], &child_min, &child_max))
        {
            return 0;
        }
        if (child_max >= node.keys[0])
        {
            return 0;
        }

        // Validate middle children
        for (int i = 1; i < node.num_keys; i++)
        {
            if (!validate_node(tree, node.children[i], &child_min, &child_max))
            {
                return 0;
            }
            if (child_min <= node.keys[i - 1] || child_max >= node.keys[i])
            {
                return 0;
            }
        }

        // Validate rightmost child
        if (!validate_node(tree, node.children[node.num_keys], &child_min, &child_max))
        {
            return 0;
        }
        if (child_min <= node.keys[node.num_keys - 1])
        {
            return 0;
        }
    }

    return 1;
}

// Public function to validate entire B-tree
int validate_btree(BTree *tree)
{
    if (!tree->is_open)
        return -1;
    if (tree->header.root_block_id == 0)
        return 1; // Empty tree is valid

    uint64_t min_key, max_key;
    return validate_node(tree, tree->header.root_block_id, &min_key, &max_key);
}

// Additional helper function to get tree statistics
void get_tree_stats(BTree *tree, int *height, int *total_nodes, int *total_keys)
{
    *height = 0;
    *total_nodes = 0;
    *total_keys = 0;

    if (!tree->is_open || tree->header.root_block_id == 0)
    {
        return;
    }

    count_nodes_recursive(tree->header.root_block_id, 1, height, total_nodes, total_keys, tree);
}

// Function to get cache statistics
void get_cache_stats(int *num_cached, int *num_dirty)
{
    *num_cached = node_cache.count;
    *num_dirty = 0;

    for (int i = 0; i < node_cache.count; i++)
    {
        if (node_cache.entries[i].is_dirty)
        {
            (*num_dirty)++;
        }
    }
}

static void count_nodes_recursive(uint64_t block_id, int level, int *height,
                                  int *total_nodes, int *total_keys, BTree *tree)
{
    if (block_id == 0)
        return;

    BTreeNode node = {0};
    read_node(tree, block_id, &node);

    (*total_nodes)++;
    (*total_keys) += node.num_keys;
    if (level > *height)
        *height = level;

    if (!is_leaf(&node))
    {
        for (int i = 0; i <= node.num_keys; i++)
        {
            count_nodes_recursive(node.children[i], level + 1, height, total_nodes, total_keys, tree);
        }
    }
}