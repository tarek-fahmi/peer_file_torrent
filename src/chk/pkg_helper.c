// Local Dependencies:
#include <fcntl.h>
#include <merkletree.h>
#include <my_utils.h>
#include <pkgchk.h>
#include <pkg_helper.h>
#include <sha256.h>
// Standard Linux Dependencies:
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
void chk_parse(mtree_node* node, char* dataStart, char* rest, uint8_t* data_file){
    chunk_obj* chunk = (chunk_obj*) (node->value);

    char* exp_hash_tmp = truncate_string((dataStart, ",", &rest), SHA256_HEXLEN);
    strcpy(node->expected_hash, exp_hash_tmp);

    chunk->offset = (uint32_t) strtoi(strtok_r(dataStart, ",", &rest));
    chunk->size = (uint32_t) strtoi(strtok_r(dataStart, ",", &rest));
    chunk->data = (data_file + chunk->offset);
}

/**
 * Unpacks a key-value pair which is stored on one line.
 *
 * @param bpkg, pointer to empty bpkg object
 * @param bpkgString, package file stored in continuous and complete string.
 *
 * @return bpkg, pointer to constructed bpkg object
 */
void bpkg_monoparse(struct bpkg_obj* bpkg, char* dataStart, char* key,
                    char* rest) {

    char* data = strtok_r(dataStart, "\n", &rest);
    if (strcmp(key, "ident") == 0) {
        bpkg->ident = truncate_string(data, IDENT_MAX);

    } else if (strcmp(key, "filename") == 0) {
        truncate_strint(data, FILENAME_MAX);
        bpkg->filename = data;

    } else if (strcmp(key, "size") == 0) {
        bpkg->size = strtoi(data);

    } else if (strcmp(key, "nhashes") == 0) {
        bpkg->nhashes = strtoi(data);
        bpkg->hashes = (char**) malloc(sizeof(char*) * bpkg->nchunks);

    } else if (strcmp(key, "nchunks") == 0) {
        bpkg->nchunks = strtoi(data);
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
void bpkg_multiparse(bpkg_obj* bpkg, char* startData, char* key, char* rest)
{
    if (bpkg->nhashes != NULL && strcmp(key, "hashes") == 0) {
        for (int i = 0; i < bpkg->nhashes, i++;) {
            mtree_node* node = (mtree_node*)malloc(sizeof(mtree_node));

            char* buffer = string_truncate(strtok_r(startData, "\n", &rest), SHA256_HEXLEN);
            node->is_leaf = 1;

            strcpy(node->expected_hash, buffer);
            bpkg->hashes[i] = &node->expected_hash;
            bpkg->mtree->chk_nodes[i] = node;
        }
    } else if (bpkg->nchunks != NULL && strcmp(key, "chunks") == 0) {

        int fd = open(bpkg->filename, O_RDONLY);
        if (fd < 0) {
            perror("Failed to open file...");
            return NULL;
        }

        struct stat fstats;
        if (fstat(fd, &fstats)) {
            perror("Fstat failure");
        }

        uint8_t* data_file = (char*)mmap(NULL, fstats.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (data_file = MAP_FAILED) {
            perror("mmap");
            return NULL;
        }

        for (int i = 0; i < bpkg->nhashes, i++;) {
            mtree_node* node = (mtree_node*)malloc(sizeof(mtree_node));

            char* buffer = string_truncate(strtok_r(startData, "\n", &rest), SHA256_HEXLEN);
            node->is_leaf = 0;

            chk_parse(node, startData, rest, data_file);
            strcpy(node->expected_hash, buffer);
            bpkg->mtree->chk_nodes[i] = node;
        }
    }
}

/**
 * Unpacks the contents of the package file into a bpkg_object.
 *
 * @param bpkg, pointer to empty bpkg object
 * @param bpkgString, package file stored in continuous and complete string.
 *
 * @return bpkg, pointer to constructed bpkg object
 */
bpkg_obj* bpkg_unpack(struct bpkg_obj* bpkg, char* bpkgString)
{
    char* key;
    char* dataStart;
    char* rest;

    while ((key = strtok_r(bpkgString, ":", &rest))) {
        if (strcmp(key, "chunks") != 0 && strcmp(key, "hashes") != 0) {
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
mtree_node* bpkg_get_largest_completed_subtree(mtree_node* root){

    if (root->is_completed){
        return root;
    }else if(root->left->is_completed){
        bpkg_get_min_completed_hashes(root->left);
    }else if (root->right->is_completed){
        bpkg_get_min_completed_hashes(root->right);
    }else{
       printf("No nodes are yet completed...");
       return NULL;
    }
}

/**
 * Deallocates the chunk object associated 
 * with a lead node, and any dynamically allocated
 * attributes.
*/
void bpkg_chunk_destroy(struct chunk_obj* cobj){
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
char** bpkg_get_subtree_chunks(mtree_node* node, int* size)
{
    char** a_hashes = malloc(sizeof(char*)); 
    char** b_hashes = malloc(sizeof(char*));

    if(node->is_leaf){
        a_hashes[0] = node->expected_hash;
        *size = 1;
        return a_hashes;
    }else{
        uint32_t a_size = 0, b_size = 0;

        a_hashes = bpkg_get_subtree_chunks(node->left, &a_size);
        b_hashes = bpkg_get_subtree_chunks(node->right, &b_size);

        char** arr_merged = merge_arrays(a_hashes, b_hashes, a_size, b_size);
        *size = a_size + b_size;
        return arr_merged;
    }
    
}

/**
 * @brief Given a node hash, return the corresponding node if it exists.
 * 
 * @param node, the node representing the subtree which contains the node corresponding to the hash.
 * @param query_hash, the hash corresponding to the node we are searching for in the subtree we are searching through.
 * 
 * @returns Node corresponding to the hash we are querying in the merkle (sub)tree, returning -1 if the node doesn't exist.
*/
mtree_node* bpkg_get_node_from_hash(mtree_node* node, char* query_hash){
    
    return;
}
