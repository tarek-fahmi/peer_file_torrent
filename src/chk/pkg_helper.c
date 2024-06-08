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
    bpkg->ident = NULL;
    bpkg->filename = NULL;
    bpkg->pkg_data = NULL;
    bpkg->mtree->root = NULL;
    bpkg->mtree->hsh_nodes = NULL;
    bpkg->mtree->chk_nodes = NULL;
    bpkg->mtree->nodes = NULL;
    bpkg->mtree->f_data = NULL;
    bpkg->mtree->height = 0;
    bpkg->mtree->nchunks = 0;
    bpkg->mtree->nnodes = 0;
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

    debug_print("Starting to parse package data...\n");

    // Tokenize data by lines
    line = strtok(data, "\n");
    while (line != NULL) {
        debug_print("Parsing line: %s\n", line);

        if (strncmp(line, "ident:", 6) == 0) {
            debug_print("Ident line found.\n");
            // Do nothing with ident in this implementation
        } else if (strncmp(line, "filename:", 9) == 0) {
            debug_print("Filename line found.\n");
            // Do nothing with filename in this implementation
        } else if (strncmp(line, "size:", 5) == 0) {
            debug_print("Size line found: %s\n", line + 5);
            mtree->f_size = atoi(line + 5);
        } else if (strncmp(line, "nhashes:", 8) == 0) {
            debug_print("Nhases line found: %s\n", line + 8);
            mtree->nhashes = atoi(line + 8);
        } else if (strncmp(line, "hashes:", 7) == 0) {
            debug_print("Hashes section found.\n");
            // Skip hashes for this implementation
            for (i = 0; i < mtree->nhashes; i++) {
                line = strtok(NULL, "\n");
                debug_print("Hash line: %s\n", line);
            }
        } else if (strncmp(line, "nchunks:", 8) == 0) {
            debug_print("Nchunks line found: %s\n", line + 8);
            mtree->nchunks = atoi(line + 8);
        } else if (strncmp(line, "chunks:", 7) == 0) {
            debug_print("Chunks section found.\n");
            mtree->chk_nodes = (mtree_node_t**)malloc(mtree->nchunks * sizeof(mtree_node_t*));
            if (mtree->chk_nodes == NULL) {
                debug_print("Error: Memory allocation for chunk nodes failed.\n");
                free(data);
                return -1;
            }
            for (i = 0; i < mtree->nchunks; i++) {
                line = strtok(NULL, "\n");
                if (line != NULL) {
                    char hash[SHA256_HEXLEN + 1]; // Ensure there's space for null-termination
                    uint32_t offset, size;

                    // Use sscanf to split the line safely
                    if (sscanf(line, "%64s,%u,%u", hash, &offset, &size) != 3) {
                        debug_print("Error: Invalid chunk format.\n");
                        free(data);
                        return -1;
                    }

                    debug_print("Chunk parsed - Hash: %s, Offset: %u, Size: %u\n", hash, offset, size);

                    chunk_t *chunk = chunk_create(NULL, size, offset);
                    if (chunk == NULL) {
                        debug_print("Error: Failed to create chunk.\n");
                        free(data);
                        return -1;
                    }

                    mtree_node_t *node = mtree_node_create(hash, 1, 0, chunk);
                    if (node == NULL) {
                        debug_print("Error: Failed to create mtree node.\n");
                        free(data);
                        return -1;
                    }

                    mtree->chk_nodes[i] = node;
                }
}
        }

        line = strtok(NULL, "\n");
    }

    debug_print("Finished parsing package data.\n");

    free(data);
    return 0;
}

int bpkg_monoparse(bpkg_t *bpkg, char *key, char *data) {
    if (!bpkg || !key) {
        fprintf(stderr, "Invalid arguments to bpkg_monoparse\n");
        return -1;
    }

    if (strcmp(key, "ident") == 0) {
        bpkg->ident = truncate_string(data, IDENT_MAX);
        debug_print("Parsed ident: %s\n", bpkg->ident);
    } else if (strcmp(key, "filename") == 0) {
        bpkg->filename = truncate_string(data, FILENAME_MAX);
        debug_print("Parsed filename: %s\n", bpkg->filename);
    } else if (strcmp(key, "size") == 0) {
        sscanf(data, " %u", &bpkg->mtree->f_size);
        debug_print("Parsed size: %d\n", bpkg->mtree->f_size);
    } else if (strcmp(key, "nhashes") == 0) {
        sscanf(data, " %u", &bpkg->mtree->nhashes);
        bpkg->mtree->hsh_nodes = (mtree_node_t**) my_malloc(sizeof(mtree_node_t*) * bpkg->mtree->nhashes);
        bpkg->mtree->nodes = (mtree_node_t**) my_malloc(sizeof(mtree_node_t*) * bpkg->mtree->nhashes);
        debug_print("Parsed nhashes: %d\n", bpkg->mtree->nhashes);
    } else if (strcmp(key, "nchunks") == 0) {
        sscanf(data, " %u", &bpkg->mtree->nchunks);
        bpkg->mtree->nnodes = bpkg->mtree->nhashes + bpkg->mtree->nchunks;
        bpkg->mtree->chk_nodes = (mtree_node_t**) my_malloc(sizeof(mtree_node_t*) * (bpkg->mtree->nchunks));
        bpkg->mtree->nodes = (mtree_node_t**) realloc(bpkg->mtree->nodes, (sizeof(mtree_node_t*) * (bpkg->mtree->nnodes)));
        debug_print("Parsed nchunks: %d\n", bpkg->mtree->nchunks);
    } else {
        fprintf(stderr, "Failed to find a matching key for key [%s]\n", key);
        return -1;
    }

    return 0;
}


/**
 * Unpacks a key-value pair which is stored on multiple lines.
 *
 * @param bpkg, pointer to empty bpkg object
 * @param pkg_data, package file stored in continuous and complete string.
 *
 * @return bpkg, pointer to constructed bpkg object
 */
int bpkg_multiparse(bpkg_t *bpkg, char *key, char *pkg_data) {
    char *line = NULL;

    if (strcmp(key, "hashes") == 0) {
        for (int i = 0; i < bpkg->mtree->nhashes; i++) {
            line = strsep(&pkg_data, "\n");
            if (!line) {
                fprintf(stderr, "Failed to parse hash at index %d\n", i);
                return -1;
            }
            mtree_node_t *node = mtree_node_create(line, 0, 0, NULL);
            node->is_leaf = 1;
            bpkg->mtree->hsh_nodes[i] = node;
            bpkg->mtree->nodes[i] = node;
            debug_print("Added hash [%s]\n", line);
        }
        return 0;
    } else if (strcmp(key, "chunks") == 0) {
        for (int i = 0; i < bpkg->mtree->nchunks; i++) {
            line = strsep(&pkg_data, "\n");
            if (!line) {
                fprintf(stderr, "Failed to parse chunk at index %d\n", i);
                return -1;
            }
            char *hash = strsep(&line, ",");
            char *offset_str = strsep(&line, ",");
            char *size_str = line;

            if (!hash || !offset_str || !size_str) {
                fprintf(stderr, "Invalid chunk format at index %d\n", i);
                return -1;
            }

            mtree_node_t *node = mtree_node_create(hash, 1, 0, NULL);
            load_chunk(node, offset_str, size_str);
            node->is_leaf = 1; // It should be a leaf node
            int index = bpkg->mtree->nhashes + i;
            bpkg->mtree->chk_nodes[i] = node;
            bpkg->mtree->nodes[index] = node;
            debug_print("Added chunk [%s, %s, %s]\n", hash, offset_str, size_str);
        }
        return 0;
    }
    return -1;
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
void load_chunk(mtree_node_t* node, char* offset_str, char* size_str) {
    if (!node || !offset_str || !size_str) {
        fprintf(stderr, "Invalid input to load_chunk\n");
        return;
    }

    node->chunk = (chunk_t*) malloc(sizeof(chunk_t));
    if (node->chunk == NULL) {
        perror("Memory allocation failure for chunk\n");
        return;
    }

    node->chunk->offset = strtoul(offset_str, NULL, 10);
    node->chunk->size = strtoul(size_str, NULL, 10);
    node->chunk->data = NULL; // Initialize data to NULL
}
/**
 * Recursively find the uppermost hash which is valid.
 *
 * @param root, largest completed subtree root.
 * @return The root of the largest subtree.
 */
mtree_node_t* bpkg_get_largest_completed_subtree(mtree_node_t* root){
    if (!root || !root->left || !root->right) return NULL;

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