#ifndef TREE_MERKLETREE_H
#define TREE_MERKLETREE_H

#define CHUNK_SIZE (4096)
#define SHA256_HEXLEN (64)
#define FILE_MAX (256)
#define IDENTITY_MAX (4096)

#include <utilities/my_utils.h>
#include <stdint.h>

typedef struct chunk_t {
    uint8_t* data;      ///< Pointer to the chunk data
    uint32_t size;      ///< Size of the chunk
    uint32_t offset;    ///< Offset of the chunk in the file
} chunk_t;

typedef struct mtree_node {
    bool is_leaf;                     ///< Indicates if the node is a leaf
    bool is_complete;                 ///< Indicates if the node's hash is complete

    struct mtree_node* left;          ///< Pointer to the left child node
    struct mtree_node* right;         ///< Pointer to the right child node
    struct mtree_node* parent;        ///< Pointer to the parent node
    uint16_t depth;                   ///< Depth of the node in the tree
    uint16_t height;                  ///< Height of the node in the tree

    uint32_t key[2];                  ///< Key range for the node

    chunk_t* chunk;                   ///< Pointer to the chunk associated with this node
    char expected_hash[SHA256_HEXLEN];///< Expected hash value
    char computed_hash[SHA256_HEXLEN];///< Computed hash value
    pthread_mutex_t lock;             ///< Mutex for thread safety
} mtree_node_t;

typedef struct mtree {
    mtree_node_t* root;               ///< Root of the Merkle tree

    uint16_t height;                  ///< Height of the tree
    uint32_t nnodes;                  ///< Number of nodes in the tree
    uint32_t nchunks;                 ///< Number of chunks in the tree
    uint32_t nhashes;                 ///< Number of hash nodes in the tree

    mtree_node_t** nodes;             ///< Array of all nodes
    mtree_node_t** chk_nodes;         ///< Array of chunk nodes
    mtree_node_t** hsh_nodes;         ///< Array of hash nodes

    uint8_t* f_data;                  ///< File data
    uint32_t f_size;                  ///< File size
} mtree_t;

enum hash_type {
    EXPECTED,                         ///< Expected hash type
    COMPUTED,                         ///< Computed hash type
};

/**
 * @brief Represents a package object with file metadata and contents.
 */
typedef struct bpkg_obj {
    char ident[IDENTITY_MAX];         ///< Identifier for the package
    char filename[FILE_MAX];          ///< Name of the package file
    char* pkg_data;                   ///< Pointer to the package data
    uint32_t pkg_size;                ///< Size of the package

    mtree_t* mtree;                   ///< Pointer to the Merkle tree
} bpkg_t;

/**
 * @brief Builds a Merkle tree from a file.
 * 
 * @param mtree Pointer to the Merkle tree structure.
 * @param filepath Path to the file.
 * @return Pointer to the built Merkle tree.
 */
mtree_t* mtree_build(mtree_t* mtree, char* filepath);

/**
 * @brief Creates a Merkle tree node from a level-order array.
 * 
 * @param mtree Pointer to the Merkle tree structure.
 * @param i Index of the node in the array.
 * @param depth Depth of the node.
 * @return Pointer to the created node.
 */
mtree_node_t* mtree_from_lvlorder(mtree_t* mtree, uint32_t i, uint16_t depth);

/**
 * @brief Constructs an array of character pointers to leaf node hashes.
 * 
 * @param mtree Pointer to the Merkle tree structure.
 * @param mode Specify desire for expected or computed hashes.
 * @return Array of hash pointers.
 */
char** mtree_get_chunk_hashes(mtree_t* mtree, enum hash_type mode);

/**
 * @brief Creates a Merkle tree node.
 * 
 * @param expected_hash Expected hash value.
 * @param is_leaf Indicates if the node is a leaf.
 * @param depth Depth of the node in the tree.
 * @param chunk Pointer to the chunk associated with the node.
 * @return Pointer to the created node.
 */
mtree_node_t* mtree_node_create(char* expected_hash, bool is_leaf, uint16_t depth, chunk_t* chunk);

/**
 * @brief Checks the construction of a Merkle tree.
 * 
 * @param mtree Pointer to the Merkle tree structure.
 */
void check_mtree_construction(mtree_t* mtree);

/**
 * @brief Destroys a Merkle tree, freeing all allocated memory.
 * 
 * @param mtree Pointer to the Merkle tree structure.
 */
void mtree_destroy(mtree_t* mtree);

/**
 * @brief Destroys a chunk, freeing its allocated memory.
 * 
 * @param chunk Pointer to the chunk.
 */
void bpkg_chunk_destroy(chunk_t* chunk);

/**
 * @brief Gets the number of chunks from the root of the tree.
 * 
 * @param node Pointer to the root node.
 * @param tree_height Height of the tree.
 * @return Number of chunks.
 */
int mtree_get_nchunks_from_root(mtree_node_t* node, uint16_t tree_height);

/**
 * @brief Destroys a Merkle tree node, freeing its allocated memory.
 * 
 * @param node Pointer to the node.
 */
void mtree_node_destroy(mtree_node_t* node);

/**
 * @brief Creates a chunk.
 * 
 * @param data Pointer to the chunk data.
 * @param size Size of the chunk.
 * @param offset Offset of the chunk in the file.
 * @return Pointer to the created chunk.
 */
chunk_t* chunk_create(uint8_t* data, uint32_t size, uint32_t offset);

/**
 * @brief Updates the data of a chunk node.
 * 
 * @param node Pointer to the node.
 * @param newdata Pointer to the new data.
 * @return 0 if successful, otherwise an error code.
 */
int chunk_node_update_data(mtree_node_t* node, uint8_t* newdata);

/**
 * @brief Gets the number of chunks from the root node.
 * 
 * @param node Pointer to the root node.
 * @param tree_height Height of the tree.
 * @return Number of chunks.
 */
int mtree_get_nchunks_from_root(mtree_node_t* node, uint16_t tree_height);

/**
 * @brief Prints information about a node.
 * 
 * @param node Pointer to the node.
 */
void node_print_info(mtree_node_t* node);

/**
 * @brief Initializes the chunk data in a Merkle tree.
 * 
 * @param mtree Pointer to the Merkle tree structure.
 * @return 0 if successful, otherwise an error code.
 */
int init_chunks_data(mtree_t* mtree);

/**
 * @brief Checks the validity of a chunk.
 * 
 * @param node Pointer to the node.
 * @return true if the chunk is valid, false otherwise.
 */
bool check_chunk(mtree_node_t* node);

/**
 * @brief Updates the hashes of parent nodes.
 * 
 * @param node Pointer to the node.
 */
void update_parent_hashes(mtree_node_t* node);

#endif