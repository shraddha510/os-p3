#include "btree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// Insert into a non-full node
static int insert_nonfull(BTree *tree, BTreeNode *node, uint64_t key, uint64_t value)
{
    int i = node->num_keys - 1;

    if (is_leaf(node))
    {
        // Move keys greater than new key
        while (i >= 0 && key < node->keys[i])
        {
            node->keys[i + 1] = node->keys[i];
            node->values[i + 1] = node->values[i];
            i--;
        }

        node->keys[i + 1] = key;
        node->values[i + 1] = value;
        node->num_keys++;
        write_node(tree, node);
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

    // Check for duplicate key
    if (check_duplicate_key(tree, key) != 0)
    {
        return -1;
    }

    // Handle empty tree
    if (tree->header.root_block_id == 0)
    {
        BTreeNode *root = create_node(tree);
        if (!root)
            return -1;

        root->keys[0] = key;
        root->values[0] = value;
        root->num_keys = 1;
        tree->header.root_block_id = root->block_id;

        write_node(tree, root);
        write_header(tree);
        free(root);
        return 0;
    }

    // Handle root splitting if needed
    BTreeNode root = {0};
    read_node(tree, tree->header.root_block_id, &root);

    if (root.num_keys == MAX_KEYS)
    {
        BTreeNode *new_root = create_node(tree);
        if (!new_root)
            return -1;

        new_root->children[0] = tree->header.root_block_id;
        tree->header.root_block_id = new_root->block_id;

        write_header(tree);
        split_child(tree, new_root, 0);
        insert_nonfull(tree, new_root, key, value);

        free(new_root);
    }
    else
    {
        insert_nonfull(tree, &root, key, value);
    }

    return 0;
}

int create_btree(BTree *tree, const char *filename)
{
    FILE *fp = fopen(filename, "wb+");
    if (!fp)
        return -1;

    tree->fp = fp;
    tree->is_open = 1;
    memcpy(tree->header.magic, MAGIC_NUMBER, 8);
    tree->header.root_block_id = 0;
    tree->header.next_block_id = 1;

    if (write_header(tree) != 0)
    {
        fclose(fp);
        tree->is_open = 0;
        return -1;
    }
    return 0;
}

int open_btree(BTree *tree, const char *filename)
{
    FILE *fp = fopen(filename, "rb+");
    if (!fp)
        return -1;

    tree->fp = fp;
    tree->is_open = 1;

    return 0;
}

void close_btree(BTree *tree)
{
    if (tree->fp)
    {
        fclose(tree->fp);
        tree->fp = NULL;
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