#define _GNU_SOURCE

// Local Dependencies:

#include <tree/merkletree.h>
#include <utilities/my_utils.h>
#include <chk/pkgchk.h>
#include <chk/pkg_helper.h>
#include <fcntl.h>

bpkg_t* bpkg_create() {
    bpkg_t* bpkg = (bpkg_t*) my_malloc(sizeof(bpkg_t));
    if (!bpkg) {
        perror("Failed to allocate bpkg");
        return NULL;
    }
    bpkg->mtree = (mtree_t*) malloc(sizeof(mtree_t));
    if (!bpkg->mtree) {
        perror("Failed to allocate mtree");
        free(bpkg);
        return NULL;
    }
    bpkg->pkg_data =         NULL;
    bpkg->mtree->root =      NULL;
    bpkg->mtree->hsh_nodes = NULL;
    bpkg->mtree->chk_nodes = NULL;
    bpkg->mtree->nodes =     NULL;
    bpkg->mtree->f_data =    NULL;
    bpkg->mtree->height =  0;
    bpkg->mtree->nchunks = 0;
    bpkg->mtree->nnodes =  0;
    bpkg->mtree->nhashes = 0;
    return bpkg;
}

int bpkg_unpack(bpkg_t* bpkg) {
    if (bpkg == NULL || bpkg->pkg_data == NULL) {
        debug_print("Error: Invalid package data.\n");
        return -1;
    }

    mtree_t* mtree = bpkg->mtree;
    if (mtree == NULL) {
        debug_print("Error: Invalid mtree structure.\n");
        return -1;
    }

    char *data = strdup(bpkg->pkg_data);
    if (data == NULL) {
        debug_print("Error: Memory allocation failed.\n");
        return -1;
    }

    char *line;
    unsigned int i = 0;

    debug_print("Parsing package metadata...\n");

    // Tokenize data by lines
    line = strtok(data, "\n");
    while (line != NULL) {

        if (strncmp(line, "ident:", 6) == 0) {
            sscanf(line, "ident:%s", bpkg->ident);
        } else if (strncmp(line, "filename:", 9) == 0) {
            sscanf(line, "filename:%s", bpkg->filename);
        } else if (strncmp(line, "size:", 5) == 0) {
            sscanf(line, "size:%u", &bpkg->mtree->f_size);
        } else if (strncmp(line, "nhashes:", 8) == 0) {
            sscanf(line, "nhashes:%u", &bpkg->mtree->nhashes);
            mtree->hsh_nodes = (mtree_node_t**)my_malloc(mtree->nhashes * sizeof(mtree_node_t*));
        } else if (strncmp(line, "hashes:", 7) == 0) {
            debug_print("Hashes section found.\n");
            for (i = 0; i < mtree->nhashes; i++) {
                line = strtok(NULL, "\n");
                line++;
                chunk_t* chunk = NULL;
                mtree_node_t *node = mtree_node_create(line, 0, 0, chunk);
                mtree->hsh_nodes[i] = node;
            }

        } else if (strncmp(line, "nchunks:", 8) == 0) 
        {
            sscanf(line, "nchunks:%u", &bpkg->mtree->nchunks);
            mtree->nnodes = bpkg->mtree->nhashes + bpkg->mtree->nchunks;
            mtree->chk_nodes = (mtree_node_t**)my_malloc(mtree->nchunks * sizeof(mtree_node_t*));

        } 
        else if (strncmp(line, "chunks:", 7) == 0) 
        {
            debug_print("Chunks section found, nchunks: %u\n", mtree->nchunks);
            for (i = 0; i < mtree->nchunks; i++) 
            {
                line = strtok(NULL, "\n");
                line++;
                if (line != NULL) 
                {
                    char hash[SHA256_HEXLEN + 1]; // Ensure there's space for null-termination
                    hash[SHA256_HEXLEN] = '\0';
                    uint32_t offset, size;

                    // Use sscanf to split the line safely
                    if (sscanf(line, "%64s,%u,%u", hash, &offset, &size) != 3)
                    {
                        debug_print("Error: Invalid chunk format.\n");
                        free(data);
                        return -1;
                    }

                    //debug_print("Chunk parsed - Hash: %s, Offset: %u, Size: %u\n", hash, offset, size);

                    chunk_t *chunk = chunk_create(NULL, size, offset);
                    mtree_node_t *node_curr = mtree_node_create(hash, true, 0, chunk);
                    mtree->chk_nodes[i] = node_curr;
                }

            }
        }
        line = strtok(NULL, "\n");
    }



    debug_print("Finished parsing package data. Now merging arrays...\n");
    combine_nodes(mtree);
    free(data);
    return 0;
}

void combine_nodes(mtree_t* mtree) {
    if (mtree == NULL) {
        debug_print("Error: mtree is NULL.\n");
        return;
    }

    // Calculate the total number of nodes
    uint32_t total_nodes = mtree->nhashes + mtree->nchunks;
    mtree->nnodes = total_nodes;

    // Allocate memory for the combined nodes array
    mtree->nodes = (mtree_node_t**)malloc(total_nodes * sizeof(mtree_node_t*));
    if (mtree->nodes == NULL) {
        debug_print("Error: Memory allocation for combined nodes failed.\n");
        return;
    }

    // Copy hsh_nodes to nodes array
    for (uint32_t i = 0; i < mtree->nhashes; i++) {
        mtree->nodes[i] = mtree->hsh_nodes[i];
    }

    // Copy chk_nodes to nodes array
    for (uint32_t i = 0; i < mtree->nchunks; i++) {
        mtree->nodes[mtree->nhashes + i] = mtree->chk_nodes[i];
    }

    debug_print("Combined nodes array created with %u nodes.\n", total_nodes);
}


// Helper Functions:
/**
 * @brief  Parses a chunk into the attributes of a node and chunk object.
 * @note
 * @param  node*: The data container.
 * @param  pkg_data: Pointer to the start of the data from bpkg file.
 * @param  pkg_data: Pointer to the pkg_data of the file.
 * @retval None
 */

/**
 * Recursively find the uppermost hash which is valid.
 *
 * @param root, largest completed subtree root.
 * @return The root of the largest subtree.
 */
mtree_node_t* bpkg_get_largest_completed_subtree(mtree_node_t* root) {
    if (!root) return NULL;

    if (check_chunk(root)) {
        return root;
    }

    mtree_node_t* left = bpkg_get_largest_completed_subtree(root->left);
    mtree_node_t* right = bpkg_get_largest_completed_subtree(root->right);

    if(left && right){
        if(left->depth >= right->depth)
        {
            debug_print("Left subtree contains contender.\n");
            return left;
        }
        debug_print("Right subtree contains contender.\n");
        return right;
    }
    if (right){
        debug_print("Right subtree graduates.\n");
        return right;
    }if (left){
        debug_print("Left subtree graduates.\n");
        return left;
    }
    debug_print("Node has no left or right children...\n");

    return NULL;
}


/**
 * Recursively find the uppermost hash which is valid.
 *
 * @param node, the root node representing the subtree of concern.
 * @param size, the total number of nodes 
 * @return Largest completed subtree root
 */
char** bpkg_get_subtree_chunks(mtree_node_t* root, int* total_chunks) {
    if (root == NULL) {
        *total_chunks = 0;
        return NULL;
    }

    if (root->is_leaf) {
        char** leaf_hashes = (char**) malloc(sizeof(char*));
        if (!leaf_hashes) {
            debug_print("Error: Memory allocation failed for leaf_hashes.\n");
            *total_chunks = 0;
            return NULL;
        }
        leaf_hashes[0] = root->expected_hash;
        *total_chunks = 1;
        return leaf_hashes;
    }

    int left_chunks = 0, right_chunks = 0;
    char** left_hashes = bpkg_get_subtree_chunks(root->left, &left_chunks);
    char** right_hashes = bpkg_get_subtree_chunks(root->right, &right_chunks);

    *total_chunks = left_chunks + right_chunks;

    if (*total_chunks <= 0) {
        if (left_hashes) free(left_hashes);
        if (right_hashes) free(right_hashes);
        return NULL;
    }

    char** all_hashes = (char**) malloc((*total_chunks) * sizeof(char*));
    if (!all_hashes) {
        debug_print("Error: Memory allocation failed for all_hashes.\n");
        if (left_hashes) free(left_hashes);
        if (right_hashes) free(right_hashes);
        *total_chunks = 0;
        return NULL;
    }

    if (left_hashes) {
        memcpy(all_hashes, left_hashes, left_chunks * sizeof(char*));
        free(left_hashes);
    }

    if (right_hashes) {
        memcpy(all_hashes + left_chunks, right_hashes, right_chunks * sizeof(char*));
        free(right_hashes);
    }

    return all_hashes;
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