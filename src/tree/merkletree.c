#include <tree/merkletree.h>
#include <utilities/my_utils.h>
#include <crypt/sha256.h>
#include <sys/mman.h>
#include <math.h>


mtree_t* mtree_build(mtree_t* mtree, char* filename)
{

    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Cannot open file: %s\n", filename);
        return NULL;
    }

    struct stat statbuf;
    if (fstat(fd, &statbuf)) {
        perror("Fstat failure\n");
        close(fd);
        return NULL;
    }

    mtree->f_data = (uint8_t*)mmap(NULL, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (mtree->f_data == MAP_FAILED) {
        perror("Cannot open file\n");
        close(fd);
        return NULL;
    }

    if (init_chunks_data(mtree)<0)
    {
        perror("Failed to intialize chunk data...");
        return NULL;
    }    

    mtree->root = mtree_from_lvlorder(mtree, 0, 0);
    debug_print("Root node has hash: %.64s\n", mtree->root->expected_hash);
    if (!mtree->root) {
        perror("Could not build merkle tree:(");
        munmap(mtree->f_data, statbuf.st_size);
        return NULL;
    }
    return mtree;
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

mtree_node_t* mtree_from_lvlorder(mtree_t* mtree, uint32_t i, uint16_t depth)
{
    uint32_t i_left = 2 * i + 1;
    uint32_t i_right = 2 * i + 2;

    uint32_t nnodes = mtree->nnodes;
    mtree_node_t** nodes = mtree->nodes;
    if (i < nnodes)
    {
        mtree_node_t* node_cur =mtree->nodes[i];
        node_print_info(nodes[i]);

        if (i_left < nnodes && i_right < nnodes)
        {
            node_cur->left = mtree_from_lvlorder(mtree, i_left, depth + 1);
            node_cur->right = mtree_from_lvlorder(mtree, i_right, depth + 1);
            node_cur->height = 1 + fmax(node_cur->left->height, node_cur->right->height);
            node_cur->depth = depth;
            debug_print("Node at depth %u has children at depths %u and %u.\n",
                         node_cur->depth, node_cur->left->depth, node_cur->right->depth);
            if (node_cur->left->is_leaf == 1)
            {
                node_cur->key[0] = node_cur->left->chunk->offset, 
                node_cur->key[1] = node_cur->right->chunk->offset;
            }else{
                node_cur->key[0] = node_cur->left->key[0];
                node_cur->key[1] = node_cur->right->key[1];
            }
            
             
            sha256_compute_internal_hash(node_cur);
            return node_cur;

        } else
        {
            node_cur->height = 0;
            sha256_compute_chunk_hash(node_cur);
            node_print_info(node_cur);
            return node_cur;
        }
    if (check_chunk(node_cur) == true)
        {
            node_cur->is_complete = true;
        }else{
            node_cur->is_complete = false;
        }
    }
    debug_print("Failed to build node due to invalid index\n");
    return NULL;
}

void mtree_print_info(mtree_t* mtree)
{
    debug_print("%u %u %u", mtree->nnodes, mtree->nhashes, mtree->nchunks);
}


/**
 * @brief  Dynamically allocates and returns an array of chunk hashes, whether expected or computed.
 * @note   
 * @param  mode: Specify desire for expected (mode = 0) or computed (mode = 1) hashes.
 * @retval Hash array.
 */


chunk_t* chunk_create(uint8_t* data, uint32_t size, uint32_t offset) {
    chunk_t* chk = (chunk_t*) my_malloc(sizeof(chunk_t));

    if (data) {
        chk->data = (uint8_t*) my_malloc(size);
        memcpy(chk->data, data, size);
    } else {
        chk->data = NULL;
    }
    chk->size = size;
    chk->offset = offset;

    return chk;
}

mtree_node_t* mtree_node_create(char* expected_hash, bool is_leaf, uint16_t depth, chunk_t* chunk) {
    mtree_node_t* node_new = (mtree_node_t*) my_malloc(sizeof(mtree_node_t));
    if (!node_new) {
        debug_print("Failed to allocate memory for mtree node.\n");
        return NULL;
    }

    node_new->is_leaf = is_leaf;
    node_new->is_complete = false;
    node_new->left = NULL;
    node_new->right = NULL;
    node_new->depth = 0;
    node_new->height = 0;
    node_new->chunk = chunk;

    if (expected_hash) {
        memcpy(node_new->expected_hash, expected_hash, SHA256_HEXLEN);
    } else {
        memset(node_new->expected_hash, 0, SHA256_HEXLEN);
    }
    memset(node_new->computed_hash, 0, SHA256_HEXLEN);

    if (pthread_mutex_init(&node_new->lock, NULL) != 0) {
        debug_print("Failed to init lock for mtree node.\n");
        free(node_new);
        return NULL;
    }
    
    node_print_info(node_new);
    return node_new;
}

void node_print_info(mtree_node_t* node)
{
//     if (node == NULL) {
//         debug_print("\tNode is NULL.\n");
//         return;
//     }

//     if (node->is_leaf) { 
//         if (node->chunk != NULL) {
//             debug_print("\tLeaf = Hash: %s\n\t, Offset: %u, Data: [%64s]\n", 
//                         node->expected_hash, node->chunk->offset, (char*) node->chunk->data);
//         }
//     } else {
//         debug_print("\tInternal = Hash: %s\n\t, Key: [%u, %u], Depth: %u\n", 
//                     node->expected_hash, node->key[0], node->key[1], node->depth);
//     }
}


void mtree_destroy(mtree_t* mtree) {
    if (mtree) {
        debug_print("Destroying Merkle tree\n");

        if (mtree->nodes) {
            for (uint32_t i = 0; i < mtree->nnodes; i++) {
                if (mtree->nodes[i]) {
                    mtree_node_destroy(mtree->nodes[i]);
                    mtree->nodes[i] = NULL;  // Nullify pointer after freeing
                }
            }
            free(mtree->nodes);
            mtree->nodes = NULL;  // Nullify pointer after freeing
        }

        if (mtree->chk_nodes) {
            free(mtree->chk_nodes);
            mtree->chk_nodes = NULL;  // Nullify pointer after freeing
        }

        if (mtree->hsh_nodes) {
            free(mtree->hsh_nodes);
            mtree->hsh_nodes = NULL;  // Nullify pointer after freeing
        }

        if (mtree->f_data) {
            munmap(mtree->f_data, mtree->f_size);
            mtree->f_data = NULL;
        }
        free(mtree);
        
        mtree = NULL;  // Nullify pointer after freeing
    }
    return;
}

void mtree_node_destroy(mtree_node_t* node) {
    if (node) {
        if (node->chunk) {
            bpkg_chunk_destroy(node->chunk);
            node->chunk = NULL;  // Nullify pointer after freeing
        }
        pthread_mutex_destroy(&node->lock);
        free(node);
        node = NULL;  // Nullify pointer after freeing
    }
}
void bpkg_chunk_destroy(chunk_t* cobj) {
    if (cobj) {
        if (cobj->data) {
            cobj->data = NULL;  // Nullify pointer after freeing
        }
        free(cobj);
        cobj = NULL;  // Nullify pointer after freeing
    }
}



int chunk_node_update_data(mtree_node_t* node, uint8_t* newdata) {
    if (node->is_leaf != 1) {
        return -1;
    }
    pthread_mutex_lock(&node->lock);
    memcpy(node->chunk->data, newdata, node->chunk->size);
    sha256_compute_chunk_hash(node);
    pthread_mutex_unlock(&node->lock);
    return 0;
}

int mtree_get_nchunks_from_root(mtree_node_t* node, uint16_t tree_height) {
    return (int)pow(2, (tree_height - node->depth) + 1) - 1;
}

int init_chunks_data(mtree_t* mtree)
{
    mtree_node_t* node_c;
    chunk_t* chk_c;
    for (int i=0; i<mtree->nchunks; i++)
    {
        node_c  = mtree->chk_nodes[i];
        chk_c = node_c->chunk;
        if (!chk_c || !node_c)
        {
            debug_print("Failed to store data_ptr for current chunk...");
            return -1;
        }
        chk_c->data = (mtree->f_data + chk_c->offset);
        sha256_compute_chunk_hash(node_c);
    }

    return 0;

}

bool check_chunk(mtree_node_t* node)
{
    if (strncmp(node->expected_hash, node->computed_hash, SHA256_HEXLEN) == 0) {
        debug_print("Chunk valid!\n");
        return true;
    } else {
        debug_print("Chunk invalid:(\n");
        return false;
    }
}
