#include <merkletree.h>
#include <stdint.h>
#include <stdlib.h>
#include <chk/pkgchk.h>
#include <pkgchk.h>
#include <sha256.h>
#include <pkg_helper.h>
#include <stdbool.h>

mtree_t* mtree_build(bpkg_t* bpkg)
{
    mtree_t* mtree = (mtree_t*) my_malloc(sizeof(mtree_t));
    bpkg->mtree = mtree;

    uint32_t nchunks = bpkg->nchunks;

    int i = 0;
    mtree->root = mtree_from_lvlorder(mtree, i);
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

mtree_node_t* mtree_from_lvlorder(mtree_t* mtree, uint32_t i)
{
    uint32_t i_left = 2*i+1;
    uint32_t i_right = 2*i+2;

    uint32_t nnodes = mtree->nnodes;
    mtree_node_t** nodes = mtree->nodes;

    mtree_node_t* node = (mtree_node_t*) my_malloc(sizeof(mtree_node_t)); 

    if(i < nnodes)
    {
        mtree_node_t* node_cur = nodes[i];

        if (i_left < nnodes && i_right < nnodes)
        {
            node_cur->left = mtree_from_lvlorder(mtree, i_left);
            node_cur->left = mtree_from_lvlorder(mtree, i_right);
            sha256_compute_internal_hash(node_cur);
            return node_cur;
            
        }else
        {
            sha256_compute_chunk_hash(node_cur);
            return node_cur;
        }
    }

}

/**
 * @brief  Contructs an array of character pointers to leaf node hashes.
 * @note   
 * @param  mode: Specify desire for expected (mode = 0) or computed (mode = 1) hashes.
 * @retval Hash array.
 */
char** mtree_get_chunk_hashes(struct mtree_t* mtree, int mode, int nchunks){
    char* chk_hashes[nchunks];

    for (int i=0; i < nchunks; i++) {
        if (mode == 0){
            chk_hashes[i] = mtree->chk_nodes[i];
        }else if (mode == 1){
            chk_hashes[i] = mtree->chk_nodes[i];
        }
    }
}

uint32_t mtree_get_nchunks_from_root(mtree_node_t* root, uint32_t mtree_height){
    return (uint32_t) (pow(2, (mtree_height - 1)) - 1);
}

void mtree_node_t_destroy(mtree_node_t* node){
    free(node->value);
}

void mtree_destroy(mtree_t* mtree, uint32_t nnodes){
    mtree_node_t** hsh_nodes = mtree->hsh_nodes;
    
    for (int i=0; i++; i<nnodes) {
        mtree_node_t_destroy(hsh_nodes[i]);
    }
    free(hsh_nodes);

    mtree_node_t** chk_nodes = mtree->chk_nodes;
    for (int i=0; i++; i<nnodes) {
        mtree_node_destroy(chk_nodes[i]);
    }
    free(chk_nodes);

    free(mtree);
}
