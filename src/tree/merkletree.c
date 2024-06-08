#include <tree/merkletree.h>
#include <utilities/my_utils.h>
#include <crypt/sha256.h>
#include <sys/mman.h>
#include <math.h>


mtree_t* mtree_build(mtree_t* mtree, char* filename)
{

    int i = 0;

    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Cannot open file\n");
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
    close(fd);

    mtree->root = mtree_from_lvlorder(mtree, i, 0);
    if (!mtree->root) {
        perror("Could not build merkle tree:(");
        munmap(mtree->f_data, statbuf.st_size);
        return NULL;
    }
    check_mtree_construction(mtree);
    return mtree;
}

void check_mtree_construction(mtree_t* mtree)
{
    if (mtree && mtree->nodes)
    {
        for(int i=0; i< mtree->nnodes; i++)
        {
            mtree_node_t* node = mtree->nodes[i];
            debug_print("Node %u:\n\tHash:%s\n\tDepth:%u\n\tCompleted: %u\n\tIs_Leaf:%u\n\n", i, node->expected_hash, node->depth, node->is_complete, node->is_leaf);
        }
    }
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
        mtree_node_t* node_cur = nodes[i];
        node_cur->depth = depth;

        if (i_left < nnodes && i_right < nnodes)
        {
            node_cur->left = mtree_from_lvlorder(mtree, i_left, depth + 1);
            node_cur->right = mtree_from_lvlorder(mtree, i_right, depth + 1);
            node_cur->height = 1 + fmax(node_cur->left->height, node_cur->right->height);
            sha256_compute_internal_hash(node_cur);
            return node_cur;

        } else
        {
            node_cur->height = 0;
            sha256_compute_chunk_hash(node_cur);
            return node_cur;
        }
    }
    return NULL;
}
/**
 * @brief  Dynamically allocates and returns an array of chunk hashes, whether expected or computed.
 * @note   
 * @param  mode: Specify desire for expected (mode = 0) or computed (mode = 1) hashes.
 * @retval Hash array.
 */
char** mtree_get_chunk_hashes(mtree_t* mtree, enum hash_type mode){
    char** chk_hashes = (char**) my_malloc(SHA256_HEXLEN * mtree->nchunks);

    for (int i=0; i < mtree->nchunks; i++) {
        if (mode == EXPECTED){
            chk_hashes[i] = mtree->chk_nodes[i]->expected_hash;
        }else if (mode == COMPUTED){
            chk_hashes[i] = mtree->chk_nodes[i]->computed_hash;
        }
    }
    return chk_hashes;
}

mtree_node_t* mtree_node_create(char* expected_hash, uint8_t is_leaf, uint16_t depth, chunk_t* chunk)
{
    mtree_node_t* node_new = (mtree_node_t*) my_malloc(sizeof(mtree_node_t));
    node_new->is_leaf = is_leaf;
    node_new->is_complete = 0;
    node_new->left = NULL;
    node_new->right = NULL;
    node_new->depth = depth;
    node_new->height = 0;
    node_new->chunk = chunk;

    if (expected_hash) {
        strncpy(node_new->expected_hash, expected_hash, SHA256_HEXLEN);
    }
    check_err(pthread_mutex_init(&node_new->lock, NULL), "Failed to init lock for mtree node.");
    return node_new;
}


void mtree_destroy(mtree_t* mtree) {
    if (mtree) {
        printf("Destroying Merkle tree at %p\n", (void*)mtree);

        if (mtree->nodes) {
            for (uint32_t i = 0; i < mtree->nnodes; i++) {
                if (mtree->nodes[i]) {
                    printf("Destroying node %u at %p\n", i, (void*)mtree->nodes[i]);
                    mtree_node_destroy(mtree->nodes[i]);
                    mtree->nodes[i] = NULL;  // Nullify pointer after freeing
                }
            }
            free(mtree->nodes);
            mtree->nodes = NULL;  // Nullify pointer after freeing
        }

        if (mtree->chk_nodes) {
            for (uint32_t i = 0; i < mtree->nchunks - 1; i++) {
                if (mtree->chk_nodes[i]) {
                    printf("Destroying chunk node %u at %p\n", i, (void*)mtree->chk_nodes[i]);
                    mtree_node_destroy(mtree->chk_nodes[i]);
                    mtree->chk_nodes[i] = NULL;  // Nullify pointer after freeing
                }
            }
            free(mtree->chk_nodes);
            mtree->chk_nodes = NULL;  // Nullify pointer after freeing
        }

        if (mtree->hsh_nodes) {
            for (uint32_t i = 0; i < mtree->nhashes; i++) {
                if (mtree->hsh_nodes[i]) {
                    printf("Destroying hash node %u at %p\n", i, (void*)mtree->hsh_nodes[i]);
                    mtree_node_destroy(mtree->hsh_nodes[i]);
                    mtree->hsh_nodes[i] = NULL;  // Nullify pointer after freeing
                }
            }
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
}
void mtree_node_destroy(mtree_node_t* node) {
    if (node) {
        if (node->chunk) {
            printf("Destroying chunk at %p\n", (void*)node->chunk);
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
            printf("Freeing chunk data at %p\n", (void*)cobj->data);
            free(cobj->data);  // Free the data
            cobj->data = NULL;  // Nullify pointer after freeing
        }
        printf("Freeing chunk at %p\n", (void*)cobj);
        free(cobj);
        cobj = NULL;  // Nullify pointer after freeing
    }
}


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