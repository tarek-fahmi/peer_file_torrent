#define _GNU_SOURCE

// Local Dependencies:

#include <tree/merkletree.h>
#include <utilities/my_utils.h>
#include <chk/pkgchk.h>
#include <chk/pkg_helper.h>
#include <fcntl.h>
// Helper Functions:
/**
 * @brief  Parses a chunk into the attributes of a node and chunk object.
 * @note
 * @param  node*: The data container.
 * @param  pkg_data: Pointer to the start of the data from bpkg file.
 * @param  pkg_data: Pointer to the pkg_data of the file.
 * @retval None
 */
void load_chunk(mtree_node_t* node, char* pkg_data) {
    node->chunk = (chunk_t*)malloc(sizeof(chunk_t));
    if (node->chunk == NULL) {
        perror("Memory allocation failure\n");
        return;
    }
    
    chunk_t* chunk = node->chunk;
    char* hash_temp = strtok_r(pkg_data, ",", &pkg_data);

    strncpy(node->expected_hash, hash_temp, SHA256_HEXLEN);

    chunk->offset = (uint32_t) strtol(pkg_data, &pkg_data, 10);
    chunk->size = (uint32_t) strtol(pkg_data, &pkg_data, 10);
}

/**
 * Unpacks a key-value pair which is stored on one line.
 *
 * @param bpkg, pointer to empty bpkg object
 * @param pkg_data, package file stored in continuous and complete string.
 *
 * @return bpkg, pointer to constructed bpkg object
 */
void bpkg_monoparse(bpkg_t* bpkg, char* key) {
    mtree_t* mtree = bpkg->mtree;
    char* pkg_data = bpkg->pkg_data;

    char* data = strtok_r(pkg_data, "\n", &pkg_data);
    if (strcmp(key, "ident") == 0) {
        bpkg->ident = truncate_string(data, IDENT_MAX);
    } else if (strcmp(key, "filename") == 0) {
        bpkg->filename = truncate_string(data, FILENAME_MAX);
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
 * @param pkg_data, package file stored in continuous and complete string.
 *
 * @return bpkg, pointer to constructed bpkg object
 */
void bpkg_multiparse(bpkg_t* bpkg, char* key) {
    char* pkg_data = bpkg->pkg_data;
    if (bpkg->mtree->nhashes && strcmp(key, "hashes") == 0) {
        for (int i = 0; i < bpkg->mtree->nhashes; i++) {
            char* expected_hash = strtok_r(pkg_data, "\n", &pkg_data);
            mtree_node_t* node = mtree_node_create(expected_hash, 0, 0, NULL);
            node->is_leaf = 1;
            bpkg->mtree->hsh_nodes[i] = node;
            bpkg->mtree->nodes[i] = node;
        }
    } else if (bpkg->mtree->nchunks && strcmp(key, "chunks") == 0) {
        for (int i = 0; i < bpkg->mtree->nchunks; i++) {
            char* expected_hash = truncate_string(strtok_r(pkg_data, "\n", &pkg_data), SHA256_HEXLEN);
            mtree_node_t* node = mtree_node_create(expected_hash, 1, 0, NULL);
            load_chunk(node, pkg_data);
            node->is_leaf = 0;
            bpkg->mtree->chk_nodes[i] = node;
            int index = (i + (pow(2, bpkg->mtree->height + 1) - 1));
            bpkg->mtree->nodes[index] = node;
        }
    }
}

/**
 * Unpacks the contents of the package file into a bpkg_tect.
 *
 * @param bpkg, pointer to empty bpkg object
 * @param pkg_data, package file stored in continuous and complete string.
 *
 * @return bpkg, pointer to constructed bpkg object
 */
void bpkg_unpack(bpkg_t* bpkg) {
    char* key;
    char* pkg_data = bpkg->pkg_data;

    while ((key = strtok_r(pkg_data, ":", &pkg_data))) {
        if (strcmp(key, "chunks") == 0 || strcmp(key, "hashes") == 0) {
            bpkg_multiparse(bpkg, key);
        } else {
            strtok_r(pkg_data, "\n", &pkg_data);
            bpkg_monoparse(bpkg, key);
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
        debug_print("No nodes are completed...\n");
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
    return NULL;
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
char** bpkg_get_subtree_chunks(mtree_node_t* node, uint16_t tree_height) {
    char** a_hashes = NULL; 
    char** b_hashes = NULL;

    if (node->is_leaf) {
        a_hashes = (char**)malloc(sizeof(char*));
        a_hashes[0] = node->expected_hash;
        return a_hashes;
    } else {
        int b_size = mtree_get_nchunks_from_root(node->left, tree_height);
        int a_size = b_size;

        a_hashes = bpkg_get_subtree_chunks(node->left, tree_height);
        b_hashes = bpkg_get_subtree_chunks(node->right, tree_height);

        char** arr_merged = merge_arrays(a_hashes, b_hashes, a_size, b_size);
        free(a_hashes);
        free(b_hashes);
        return arr_merged;
    }
}

int bpkg_validate_node_completion(mtree_node_t* node) {
    if (strncmp(node->expected_hash, node->computed_hash, SHA256_HEXLEN) == 0) {
        return 1;
    }
    return 0;
}

mtree_node_t* bpkg_find_node_from_hash(mtree_t* mtree, char* query_hash, int mode)
{
    mtree_node_t** nodes;
    uint16_t count;
    if (mode == ALL)
    {
        nodes = mtree->nodes;
        count = mtree->nnodes;
    }
    else if (mode == INTERNAL)
    {
         nodes = mtree->nodes;
         count = mtree->nhashes;
    }
    else if (mode == CHUNK)
    { 
        nodes = mtree->nodes;
        count = mtree->nchunks;
    }

    mtree_node_t* current_node;
    for (int i = 0; i < count; i++)
    {
        current_node = nodes[i];
        if (strncmp(nodes[i]->expected_hash, query_hash, SHA256_HEXLEN) == 0)
        {
            debug_print("Queried node was found in this tree!\n");
            return current_node;
        }
    }
    debug_print("Queried node was not found in this tree...\n");
    return NULL;

}

mtree_node_t* bpkg_find_node_from_hash_offset(mtree_node_t* root, char* query_hash, uint32_t offset)
{

    if (strncmp(root->expected_hash, query_hash, SHA256_HEXLEN) == 0)
        {
            return root;
        }
    else if (root->is_leaf)
    {
        return NULL;
    }
    else
    {
        if (root->left->key[0] <= offset && root->left->key[1] <= offset)
            {
            mtree_node_t* left_qry = bpkg_find_node_from_hash_offset(root->left, query_hash, offset);
            if (left_qry != NULL)
            {
                return left_qry;
            }
        }
        

        if (root->right->key[0] <= offset && root->right->key[1] <= offset)
            {
            mtree_node_t* right_qry = bpkg_find_node_from_hash_offset(root->right, query_hash, offset);
            if (right_qry != NULL)
            {
                return right_qry;
            }
        }
    }
    return NULL;
}