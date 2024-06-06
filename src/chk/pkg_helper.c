#define _GNU_SOURCE

// Local Dependencies:
#include <fcntl.h>
#include <tree/merkletree.h>
#include <utilities/my_utils.h>
#include <chk/pkgchk.h>
// Standard Linux Dependencies:
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// Additional Linux Dependencies:
#include <string.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

// Helper Functions:
/**
 * @brief  Parses a chunk into the attributes of a node and chunk object.
 * @note
 * @param  node*: The data container.
 * @param  dataStart: Pointer to the start of the data from bpkg file.
 * @param  rest: Pointer to the rest of the file.
 * @retval None
 */
void load_chunk(mtree_node_t* node, char* dataStart){
    chunk_t* chunk = (node->chunk);
    
    char* rest;
    char* hash_temp = strtok_r(dataStart, ",", &rest);

    strcpy(node->expected_hash, hash_temp);

    chunk->offset = (uint32_t) strtol(dataStart, &dataStart, 10);
    chunk->size = (uint32_t) strtol(dataStart, &dataStart, 10);
}

/**
 * Unpacks a key-value pair which is stored on one line.
 *
 * @param bpkg, pointer to empty bpkg object
 * @param bpkgString, package file stored in continuous and complete string.
 *
 * @return bpkg, pointer to constructed bpkg object
 */
void bpkg_monoparse(bpkg_t* bpkg, char* dataStart, char* key,
                    char* rest) {

    mtree_t* mtree = bpkg->mtree;


    char* data = strtok_r(dataStart, "\n", &dataStart);
    if (strcmp(key, "ident") == 0) {
        bpkg->ident = truncate_string(data, IDENT_MAX);

    } else if (strcmp(key, "filename") == 0) {
        truncate_string(data, FILENAME_MAX);
        bpkg->filename = data;

    } else if (strcmp(key, "size") == 0) {
        mtree->f_size = strtol(data, &data, 10);

    } else if (strcmp(key, "nhashes") == 0) {
        mtree->nhashes = strtol(data, &data, 10);

    } else if (strcmp(key, "nchunks") == 0) {
        mtree->nchunks = strtol(data, &data, 10);
    }
}

/**
 * Unpacks a key-value pair which is stored on multiple lines.
 *
 * @param bpkg, pointer to empty bpkg object
 * @param bpkgString, package file stored in continuous and complete string.
 *
 * @return bpkg, pointer to constructed bpkg object
 */
void bpkg_multiparse(bpkg_t* bpkg, char* startData, char* key, char* rest)
{
    if (bpkg->mtree->nhashes && strcmp(key, "hashes") == 0) 
    {
        for (int i = 0; i < bpkg->mtree->nhashes; i++) 
        {
            mtree_node_t* node = (mtree_node_t*)malloc(sizeof(mtree_node_t));


            strtok_r(rest, "\n", &rest);
            char* buffer = strtok_r(rest, "\n", &rest);
            node->is_leaf = 1;

            memcpy(node->expected_hash, buffer, SHA256_HEXLEN);
            bpkg->mtree->chk_nodes[i] = node;
        }
    } else if (bpkg->mtree->nchunks && strcmp(key, "chunks") == 0) 
    {
        for (int i = 0; i < bpkg->mtree->nhashes; i++) 
        {
            mtree_node_t* node = (mtree_node_t*)malloc(sizeof(mtree_node_t));

            char* buffer = truncate_string(strtok_r(rest, "\n", &rest), SHA256_HEXLEN);
            node->is_leaf = 0;

            load_chunk(node, startData);

            memcpy(node->expected_hash, buffer, SHA256_HEXLEN);
            bpkg->mtree->chk_nodes[i] = node;
        }
    }
}

/**
 * Unpacks the contents of the package file into a bpkg_tect.
 *
 * @param bpkg, pointer to empty bpkg object
 * @param bpkgString, package file stored in continuous and complete string.
 *
 * @return bpkg, pointer to constructed bpkg object
 */
void bpkg_unpack(bpkg_t* bpkg, char* bpkgString)
{
    char* key;
    char* dataStart;
    char* rest;

    while ((key = strtok_r(bpkgString, ":", &rest))) {
        if (strncmp(key, "chunks") != 0 && strcmp(key, "hashes") != 0) {
            bpkg_multiparse(bpkg, dataStart, key, rest);
        } else {
            char* defStr = strtok_r(bpkgString, "\n", &rest);
            bpkg_monoparse(bpkg, dataStart, key, rest);
        }
    }
}

/**
 * Recursively find the uppermost hash which is valid.
 *
 * @param root, largest completed subtree root.
 * @return The root of the largest subtree.
 */
mtree_node_t* bpkg_get_largest_completed_subtree(mtree_node_t* root){

    if (root->is_complete){
        return root;
    }else if(root->is_leaf)
    {
        return NULL;
        debug_print("No nodes are completed...");
    }
    else if(root->left->is_complete){
        return root;
    }else if (root->right->is_complete){
        return root;
    }else{
        mtree_node_t* left = bpkg_get_largest_completed_subtree(root->left);
        mtree_node_t* right = bpkg_get_largest_completed_subtree(root->right);
        if (left != NULL)
        {
            if (right != NULL)
            {
                if (left->depth >= right->depth)
                {
                    return left;
                }
                else{
                    return right;
                }
            }
            return left;
        }
        else if (right != NULL)
        {
            return right;
        }
    }
    
}

/**
 * Deallocates the chunk object associated 
 * with a lead node, and any dynamically allocated
 * attributes.
*/
void bpkg_chunk_destroy(chunk_t* cobj){
    free(cobj->data);
    free(cobj);
}


/**
 * Recursively find the uppermost hash which is valid.
 *
 * @param node, the root node representing the subtree of concern.
 * @param size, the total number of nodes 
 * @return Largest completed subtree root
 */
char** bpkg_get_subtree_chunks(mtree_node_t* node, int* size)
{
    char** a_hashes = malloc(sizeof(char*)); 
    char** b_hashes = malloc(sizeof(char*));

    if(node->is_leaf){
        a_hashes[0] = node->expected_hash;
        *size = 1;
        return a_hashes;
    }else{
        u_int32_t a_size = 0, b_size = 0;

        a_hashes = bpkg_get_subtree_chunks(node->left, &a_size);
        b_hashes = bpkg_get_subtree_chunks(node->right, &b_size);

        char** arr_merged = merge_arrays(a_hashes, b_hashes, a_size, b_size);
        *size = a_size + b_size;
        return arr_merged;
    }
    
}

int bpkg_validate_node_completion(mtree_node_t* node)
{
    if (strncmp(node->expected_hash, node->computed_hash, SHA256_HEXLEN) == 0); return 1;
    return 0;
}