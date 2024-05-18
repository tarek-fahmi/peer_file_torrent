#ifndef MERKLE_TREE_H
#define MERKLE_TREE_H

#include <stddef.h>
#include <stdint.h>

#include <chk/pkgchk.h>

#define SHA256_HEXLEN (64)

typedef struct mtree_node {
    void* key;
    void* value;
    uint16_t depth;

    struct mtree_node* left;
    struct mtree_node* right;
    struct mtree_node* parent;

    uint8_t is_leaf;
    uint8_t is_completed;

    char expected_hash[SHA256_HEXLEN];
    char computed_hash[SHA256_HEXLEN];
}mtree_node;


typedef struct merkle_tree{// AKA: Binary Hash Tree
    struct merkle_tree_node* root;
    uint16_t height;

    struct merkle_tree_node** hsh_nodes;
    struct merkle_tree_node** chk_nodes;
}merkle_tree;

typedef struct chunk{
    char hash[SHA256_HEXLEN];
    uint32_t offset;
    uint32_t size;
}chunk;

#endif
