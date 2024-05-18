#include <merkletree.h>
#include <stdint.h>
#include <stdlib.h>
#include <chk/pkgchk.h>

merkle_tree* mtree_build(bpkg_obj* bpkg){
    char** hashes = bpkg->hashes;
    uint32_t nhashes = bpkg->nhashes;

    chunk** chunks = bpkg->hashes;
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
    }else if(i < (nhashes + nchunks)){

        chunk* chunk = chunks[i - nhashes];


        // Assign hash attributes here...
    }else{
        return NULL;
    }
    return node;
}

/**
 * @brief  Contructs an array of character pointers to leaf node hashes.
 * @note   
 * @param  mode: Specify desire for expected (mode = 0) or computed (mode = 1) hashes.
 * @retval Hash array.
 */
char** mtree_get_chunk_hashes(struct merkle_tree* tree, int mode, int nchunks){
    char* chk_hashes[nchunks];

    for (int i=0; i < nchunks; i++) {
        if (mode == 0){
            chk_hashes[i] = tree->chk_nodes[i];
        }else if (mode == 1){
            chk_hashes[i] = tree->chk_nodes[i];
        }
    }
}

uint32_t mtree_get_nchunks_from_root(mtree_node* root, uint32_t mtree_height){
    return (uint32_t) (pow(2, (mtree_height - 1)) - 1);
}

void mtree_node_destroy(mtree_node* node){
    free(node->value);
}

void mtree_destroy(merkle_tree* mtree, uint32_t nnodes){
    mtree_node** hsh_nodes = mtree->hsh_nodes;
    
    for (int i=0; i++; i<nnodes) {
        mtree_node_destroy(hsh_nodes[i]);
    }
    free(hsh_nodes);

    mtree_node** chk_nodes = mtree->chk_nodes;
    for (int i=0; i++; i<nnodes) {
        mtree_node_destroy(chk_nodes[i]);
    }
    free(chk_nodes);

    free(mtree);
}
