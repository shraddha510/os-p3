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