#include <tree/merkletree.h>
#include <utilities/my_utils.h>
#include <chk/pkgchk.h>
#include <chk/pkg_helper.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _GNU_SOURCE

bpkg_t* bpkg_create() {
    bpkg_t* bpkg = (bpkg_t*)my_malloc(sizeof(bpkg_t));
    if ( !bpkg ) {
        perror("Failed to allocate bpkg");
        return NULL;
    }
    bpkg->mtree = (mtree_t*)malloc(sizeof(mtree_t));
    if ( !bpkg->mtree ) {
        perror("Failed to allocate mtree");
        free(bpkg);
        return NULL;
    }
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
    if ( bpkg == NULL || bpkg->pkg_data == NULL ) {
        debug_print("Error: Invalid package data.\n");
        return -1;
    }

    mtree_t* mtree = bpkg->mtree;
    if ( mtree == NULL ) {
        debug_print("Error: Invalid mtree structure.\n");
        return -1;
    }

    char* data = strdup(bpkg->pkg_data);
    if ( data == NULL ) {
        debug_print("Error: Memory allocation failed.\n");
        return -1;
    }

    char* line;
    char* next_line;
    unsigned int i = 0;

    debug_print("Parsing package metadata...\n");
    debug_print("%s\n", data);

    // Tokenize data by lines
    line = strtok(data, "\n");
    while ( line != NULL ) {

        if ( strncmp(line, "ident:", 6) == 0 ) {
            sscanf(line, "ident:%s", bpkg->ident);
        }
        else if ( strncmp(line, "filename:", 9) == 0 ) {
            process_filename(line, bpkg->filename, 256);
        }
        else if ( strncmp(line, "size:", 5) == 0 ) {
            sscanf(line, "size:%u", &bpkg->mtree->f_size);
        }
        else if ( strncmp(line, "nhashes:", 8) == 0 ) {
            sscanf(line, "nhashes:%u", &bpkg->mtree->nhashes);
            mtree->hsh_nodes = (mtree_node_t**)my_malloc(mtree->nhashes * sizeof(mtree_node_t*));
        }
        else if ( strncmp(line, "hashes:", 7) == 0 && mtree->nhashes > 0 ) {
            for ( i = 0; i < mtree->nhashes && ( next_line = strtok(NULL, "\n") ); i++ ) {
                next_line = trim_whitespace(next_line);
                mtree->hsh_nodes[i] = mtree_node_create(next_line, 0, 0, NULL);
            }
        }
        else if ( strncmp(line, "nchunks:", 8) == 0 ) {
            sscanf(line, "nchunks:%u", &bpkg->mtree->nchunks);
            mtree->nnodes = bpkg->mtree->nhashes + bpkg->mtree->nchunks;
            mtree->chk_nodes = (mtree_node_t**)my_malloc(mtree->nchunks * sizeof(mtree_node_t*));
        }
        else if ( strncmp(line, "chunks:", 7) == 0 ) {
            debug_print("Chunks section found, nchunks: %u\n", mtree->nchunks);
            for ( i = 0; i < mtree->nchunks; i++ ) {
                line = strtok(NULL, "\n");
                if ( line != NULL ) {
                    line = trim_whitespace(line);
                    char hash[SHA256_HEXLEN + 1]; // Ensure there's space for null-termination
                    hash[SHA256_HEXLEN] = '\0';
                    uint32_t offset, size;

                    // Use sscanf to split the line safely
                    if ( sscanf(line, "%64s,%u,%u", hash, &offset, &size) != 3 ) {
                        debug_print("Error: Invalid chunk format.\n");
                        free(data);
                        return -1;
                    }

                    chunk_t* chunk = chunk_create(NULL, size, offset);
                    mtree_node_t* node_curr = mtree_node_create(hash, 1, 0, chunk);
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
    if ( mtree == NULL ) {
        debug_print("Error: mtree is NULL.\n");
        return;
    }

    uint32_t total_nodes = mtree->nhashes + mtree->nchunks;
    mtree->nnodes = total_nodes;


    mtree->nodes = (mtree_node_t**)malloc(total_nodes * sizeof(mtree_node_t*));
    if ( mtree->nodes == NULL ) {
        debug_print("Error: Memory allocation for combined nodes failed.\n");
        return;
    }

    for ( uint32_t i = 0; i < mtree->nhashes; i++ ) {
        mtree->nodes[i] = mtree->hsh_nodes[i];
    }

    for ( uint32_t i = 0; i < mtree->nchunks; i++ ) {
        mtree->nodes[mtree->nhashes + i] = mtree->chk_nodes[i];
    }

    debug_print("Combined nodes array created with %u nodes.\n", total_nodes);
}

char** bpkg_get_largest_completed_subtree(mtree_node_t* root, int* count) {
    if ( check_chunk(root) ) {
        char** hashes = (char**)my_malloc(sizeof(char*));
        if ( !hashes ) {
            perror("Memory allocation failed for leaf_hashes.");
            *count = 0;
            return NULL;
        }
        hashes[0] = root->expected_hash;
        *count = 1;
        return hashes;
    }
    else if ( root->is_leaf ) {
        *count = 0;
        return NULL;
    }
    else {
        int lcount = 0, rcount = 0;
        char** left_hashes = bpkg_get_largest_completed_subtree(root->left, &lcount);
        char** right_hashes = bpkg_get_largest_completed_subtree(root->right, &rcount);

        if ( lcount > 0 && rcount > 0 ) {
            char** hashes = (char**)merge_arrays((void**)left_hashes, (void**)right_hashes, lcount, rcount);
            *count = lcount + rcount;
            return hashes;
        }
        else if ( rcount > 0 ) {
            *count = rcount;
            return right_hashes;
        }
        else {
            *count = lcount;
            return left_hashes;
        }
    }
}

char** bpkg_get_subtree_chunks(mtree_node_t* root, int* total_chunks) {
    if ( root == NULL ) {
        *total_chunks = 0;
        return NULL;
    }

    if ( root->is_leaf ) {
        char** leaf_hashes = (char**)my_malloc(sizeof(char*));
        leaf_hashes[0] = root->expected_hash;
        *total_chunks = 1;
        return leaf_hashes;
    }

    int left_chunks = 0, right_chunks = 0;
    char** left_hashes = bpkg_get_subtree_chunks(root->left, &left_chunks);
    char** right_hashes = bpkg_get_subtree_chunks(root->right, &right_chunks);

    *total_chunks = left_chunks + right_chunks;

    if ( *total_chunks <= 0 ) {
        if ( left_hashes ) free(left_hashes);
        if ( right_hashes ) free(right_hashes);
        return NULL;
    }
    else if ( left_chunks > 0 && right_chunks > 0 ) {
        char** hashes = (char**)merge_arrays((void**)left_hashes, (void**)right_hashes, left_chunks, right_chunks);
        return hashes;
    }
    else if ( left_chunks > 0 ) {
        *total_chunks = left_chunks;
        return left_hashes;
    }
    else {
        *total_chunks = right_chunks;
        return right_hashes;
    }
}


int bpkg_validate_node_completion(mtree_node_t* node) {
    if ( strncmp(node->expected_hash, node->computed_hash, SHA256_HEXLEN) == 0 ) {
        return 1;
    }
    return 0;
}

mtree_node_t* bpkg_find_node_from_hash(mtree_t* mtree, char* query_hash, int mode)
{
    mtree_node_t** nodes;
    uint16_t count = 0;
    if ( mode == ALL )
    {
        nodes = mtree->nodes;
        count = mtree->nnodes;
    }
    else if ( mode == INTERNAL )
    {
        nodes = mtree->nodes;
        count = mtree->nhashes;
    }
    else if ( mode == CHUNK )
    {
        nodes = mtree->nodes;
        count = mtree->nchunks;
    }

    mtree_node_t* current_node;
    for ( int i = 0; i < count; i++ )
    {
        current_node = nodes[i];

        if ( strncmp(nodes[i]->expected_hash, query_hash, SHA256_HEXLEN) == 0 )
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
    if ( root == NULL ) {
        return NULL;
    }

    if ( strncmp(root->expected_hash, query_hash, SHA256_HEXLEN) == 0 )
    {
        return root;
    }
    else if ( root->is_leaf )
    {
        return NULL;
    }
    else
    {
        if ( root->left && root->left->key[0] <= offset && root->left->key[1] >= offset )
        {
            mtree_node_t* left_qry = bpkg_find_node_from_hash_offset(root->left, query_hash, offset);
            if ( left_qry != NULL )
            {
                return left_qry;
            }
        }

        if ( root->right && root->right->key[0] <= offset && root->right->key[1] >= offset )
        {
            mtree_node_t* right_qry = bpkg_find_node_from_hash_offset(root->right, query_hash, offset);
            if ( right_qry != NULL )
            {
                return right_qry;
            }
        }
    }
    return NULL;
}

void process_filename(const char* line, char* filename, size_t max_len) {
    const char* prefix = "filename:";
    const char* start = strstr(line, prefix);

    if ( start ) {
        start += strlen(prefix);
        if ( strncmp(start, "./", 2) == 0 ) {
            start += 2;
        }
        strncat(filename, start, max_len - strlen(filename));
        filename[max_len - 1] = '\0';
    }
    else {
        filename[0] = '\0';
    }
}

void extract_directory(const char* filepath, char* filename, size_t max_len) {
    if ( filepath == NULL || filename == NULL || max_len == 0 ) {
        return;
    }

    const char* last_slash = strrchr(filepath, '/');


    if ( last_slash == NULL ) {
        strncpy(filename, filepath, max_len - 1);
        filename[max_len - 1] = '\0';
        return;
    }

    int dir_length = last_slash - filepath + 1;


    if ( dir_length >= max_len ) {
        dir_length = max_len - 1;
    }

    strncpy(filename, filepath, dir_length);
    filename[dir_length] = '\0';
}