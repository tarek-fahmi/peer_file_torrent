#ifndef TREE_MERKLETREE_H
#define TREE_MERKLETREE_H

#define CHUNK_SIZE (4096)
#define SHA256_HEXLEN (64)

typedef struct chunk_t{
    uint8_t* data;
	uint32_t size;
	uint32_t offset;
}chunk_t;

typedef struct mtree_node {
    uint8_t is_leaf;
    uint8_t is_complete;

    struct mtree_node* left;
    struct mtree_node* right;
    uint16_t depth;
    uint32_t key[2];
    
    chunk_t* chunk;
    char expected_hash[SHA256_HEXLEN];
    char computed_hash[SHA256_HEXLEN];
    pthread_mutex_t lock;
}mtree_node_t;

typedef struct mtree{// AKA: Binary Hash Tree
    struct merkle_tree_node* root;
    
    uint16_t height;
    uint32_t nnodes;
    uint32_t nchunks;
    uint32_t nhashes;

    mtree_node_t** nodes;
    mtree_node_t** chk_nodes;
    mtree_node_t** hsh_nodes;

    uint8_t* data;
    uint32_t f_size;
}mtree_t;

enum hash_type{
    EXPECTED,
    COMPUTED,
};

/**
 * bpkg file object, stores file metadata and contents.
 * Typically: 
 * 		malloc storage for hashes and chunks.
 * 		Make sure to deallocate all dynamic memory in destroy funciton.
 */
typedef struct bpkg_obj{
	char* ident;
	char* filename;
	
	mtree_t* mtree; // Contains pointers to hashes leaf nodes.
} bpkg_t;

mtree_t* mtree_build(bpkg_t* bpkg);

mtree_node_t* mtree_from_lvlorder(mtree_t* mtree, uint32_t i);

/**
 * @brief  Contructs an array of character pointers to leaf node hashes.
 * @note   
 * @param  mode: Specify desire for expected (mode = 0) or computed (mode = 1) hashes.
 * @retval Hash array.
 */
char** mtree_get_chunk_hashes(mtree_t* mtree, enum hash_type mode);

void mtree_node_destroy(mtree_node_t* node);

void mtree_destroy(mtree_t* mtree);

#endif // (TREE_MERKLETREE_H)