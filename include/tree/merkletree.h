#ifndef MERKLE_TREE_H
#define MERKLE_TREE_H

// Local Dependencies:=
#include <crypt/sha256.h>
#include <tree/merkletree.h>
#include <chk/pkgchk.h>
#include <chk/pkg_helper.h>
#include <utilities/my_utils.h>
#include <peer_2_peer/peer_handler.h>
#include <peer_2_peer/peer_server.h>
#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/packet.h>
#include <peer_2_peer/package.h>
#include <config.h>
#include <cli.h>
// Standard Linux Dependencies:
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
// Additional Linux Dependencies:
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/select.h>

#define CHUNK_SIZE (4096)
#define SHA256_HEXLEN (64)

typedef struct mtree_node {
    void* key;
    void* value;
    uint16_t depth;

    struct mtree_node* left;
    struct mtree_node* right;

    uint8_t is_leaf;
    uint8_t is_completed;

    char expected_hash[SHA256_HEXLEN];
    char computed_hash[SHA256_HEXLEN];
}mtree_node_t;


typedef struct mtree{// AKA: Binary Hash Tree
    struct merkle_tree_node* root;
    
    uint8_t* data;
    uint16_t height;
    size_t nnodes;
    mtree_node_t** nodes;
    mtree_node_t** chk_nodes;
    mtree_node_t** hsh_nodes;

    chunk_t* value;
}mtree_t;

typedef struct chunk_t{
    uint8_t* data;
	uint32_t size;
	uint32_t offset;
}chunk_t;
#endif

mtree_t* mtree_build(bpkg_t* bpkg);

mtree_node_t* mtree_from_lvlorder(mtree_t* mtree, uint32_t i);

/**
 * @brief  Contructs an array of character pointers to leaf node hashes.
 * @note   
 * @param  mode: Specify desire for expected (mode = 0) or computed (mode = 1) hashes.
 * @retval Hash array.
 */
char** mtree_get_chunk_hashes(struct mtree_t* mtree, int mode, int nchunks);

uint32_t mtree_get_nchunks_from_root(mtree_node_t* root, uint32_t mtree_height);

void mtree_node_t_destroy(mtree_node_t* node);

void mtree_destroy(mtree_t* mtree, uint32_t nnodes);

#endif