#include <merkletree.h>
#include <stdint.h>
#include <stdlib.h>
#include <chk/pkgchk.h>

mtree* mtree_build(bpkg_obj* bpkg){
    char** hashes = bpkg->hashes;
    uint32_t nhashes = bpkg->nhashes;

    chunk* chunks = bpkg->chunks;
    uint32_t nchunks = bpkg->nchunks;

    int i = 0;
    struct merkle_tree* mkt = mtree_from_lvlorder(hashes, NULL, nhashes, chunks, nchunks, i);

}

/**
 * @brief  Recursively constructs a binary search tree, given a level order traversal in two arrays.
 * @note   
 * @param  hashes: 
 * @param  nhashes: 
 * @param  chunks: 
 * @param  nchunks: 
 * @param  i: 
 * @retval 
 */

mtree_node* mtree_from_lvlorder(int i, mtree_node* parent, char** hashes, uint32_t nhashes, chunk** chunks, uint32_t nchunks){
    uint32_t left_i = 2*i+1;
    uint32_t right_i = 2*i+2;

    mtree_node* node = (mtree_node*) malloc(sizeof(mtree_node)); 

    if(parent != NULL);
        node->parent = parent;


    if(i < nhashes){
        node->left = mtree_from_lvlorder(left_i, node, hashes, nhashes, chunks, nchunks);
        node->right = mtree_from_lvlorder(right_i, node, hashes, nhashes, chunks, nchunks);

        node->is_leaf = 0;
        node->

    }else if(i < (nhashes + nchunks)){

        chunk* chunk = chunks[i - nhashes];


        // Assign hash attributes here...
    }else{
        return NULL;
    }
    return node

}


