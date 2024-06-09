#ifndef TREE_MERKLETREE_H
#define TREE_MERKLETREE_H

#define CHUNK_SIZE (4096)
#define SHA256_HEXLEN (64)
#define FILE_MAX (256)
#define IDENTITY_MAX (4096)

#include <utilities/my_utils.h>
#include <stdint.h>

typedef struct chunk_t{
    uint8_t* data;
	uint32_t size;
	uint32_t offset;
}chunk_t;

typedef struct mtree_node {
    bool is_leaf;
    bool is_complete;

    struct mtree_node* left;
    struct mtree_node* right;
    uint16_t depth;
    uint16_t height;
    
    uint32_t key[2];
    
    chunk_t* chunk;
    char expected_hash[SHA256_HEXLEN];
    char computed_hash[SHA256_HEXLEN];
    pthread_mutex_t lock;
}mtree_node_t;

typedef struct mtree{// AKA: Binary Hash Tree
    mtree_node_t* root;
    
    uint16_t height;
    uint32_t nnodes;
    uint32_t nchunks;
    uint32_t nhashes;

    mtree_node_t** nodes;
    mtree_node_t** chk_nodes;
    mtree_node_t** hsh_nodes;

    uint8_t* f_data;
    uint32_t f_size;
}mtree_t;

enum hash_type
{
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
	char ident[IDENTITY_MAX];
	char filename[FILE_MAX];
    char* pkg_data;
    uint32_t pkg_size;
	
	mtree_t* mtree; // Contains pointers to hashes leaf nodes.
}bpkg_t;

mtree_t* mtree_build(mtree_t* mtree, char* filepath);

mtree_node_t* mtree_from_lvlorder(mtree_t* mtree, uint32_t i, uint16_t depth);

/**
 * @brief  Contructs an array of character pointers to leaf node hashes.
 * @note   
 * @param  mode: Specify desire for expected (mode = 0) or computed (mode = 1) hashes.
 * @retval Hash array.
 */
char** mtree_get_chunk_hashes(mtree_t* mtree, enum hash_type mode);

mtree_node_t* mtree_node_create(char* expected_hash, bool is_leaf, uint16_t depth, chunk_t* chunk);

void check_mtree_construction(mtree_t* mtree);

void mtree_destroy(mtree_t* mtree);

void bpkg_chunk_destroy(chunk_t* chunk);

int mtree_get_nchunks_from_root(mtree_node_t* node, uint16_t tree_height);


void mtree_node_destroy(mtree_node_t* node);

chunk_t* chunk_create(uint8_t* data, uint32_t size, uint32_t offset);

void chunk_destroy(chunk_t* chk);

int chunk_node_update_data(mtree_node_t* node, uint8_t* newdata);

int mtree_get_nchunks_from_root(mtree_node_t* node, uint16_t tree_height);

void node_print_info(mtree_node_t* node);

int init_chunks_data(mtree_t* mtree);

bool check_chunk(mtree_node_t* node);

#endif