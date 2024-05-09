#ifndef MERKLE_TREE_H
#define MERKLE_TREE_H

#include <stddef.h>
#include <stdint.h>

#include <chk/pkgchk.h>

#define SHA256_HEXLEN (64)

typedef struct mtree_node {
    void* key;
    void* value;

    struct mtree_node* left;
    struct mtree_node* right;
    struct mtree_node* parent;

    int is_leaf;
    char expected_hash[SHA256_HEXLEN];
    char computed_hash[SHA256_HEXLEN];
} mtree_node;


typedef struct mtree { // AKA: Binary Hash Tree
    struct merkle_tree_node* root;
    size_t n_nodes;
}mtree;

typedef struct chunk{
    char hash[SHA256_HEXLEN];
    uint32_t offset;
    uint32_t size;
}chunk;

#endif
