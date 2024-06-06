#include <tree/merkletree.h>
#include <utilities/my_utils.h>
#include <crypt/sha256.h>
#include <sys/mman.h>

void mtree_destroy(mtree_t* mtree){
    
    for (int i=0; i++; i<mtree->nnodes) {
        mtree_node_destroy(mtree->nodes[i]);
    }

    free(mtree->nodes);
    free(mtree->chk_nodes);
    free(mtree->hsh_nodes);

    munmap(mtree->data, mtree->f_size);

    free(mtree);
}

mtree_t* mtree_build(bpkg_t* bpkg)
{
    mtree_t* mtree = (mtree_t*) my_malloc(sizeof(mtree_t));
    bpkg->mtree = mtree;

    int i = 0;

    int fd = open(bpkg->filename, O_RDONLY);
    if (fd < 0) {
        perror("Cannot open file\n");
        return NULL;
    }

    struct stat statbuf;
    if (fstat(fd, &statbuf)) {
        perror("Fstat failure\n");
    }

    bpkg->mtree->data = (uint8_t*)mmap(NULL, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);

    if (bpkg->mtree->data == MAP_FAILED) {
        perror("Cannot open file\n");
        return NULL;
    }

    close(fd);
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

    node_new->depth = depth;
    node_new->is_leaf = is_leaf;
    if (is_leaf == 1)
    {
        node_new->chunk = chunk;
    } 
    
    check_err(pthread_mutex_init(&node_new->lock, NULL), "Failed to init lock for mtree node.");
    return node_new;
}

void mtree_node_destroy(mtree_node_t* node){
    pthread_mutex_destroy(&node->lock);
    free(node->chunk);
    free(node);
}

chunk_t* chunk_create(uint8_t* data, uint32_t size, uint32_t offset)
{
    chunk_t* chk = (chunk_t*) my_malloc(sizeof(chunk_t));

    chk->data = data;
    chk->size = size;
    chk->offset = offset;

    return chk;
}

void chunk_destroy(chunk_t* chk)
{
    free(chk);
}

int chunk_node_update_data(mtree_node_t* node, uint8_t* newdata)
{

    if (node->is_leaf != 1){
        return -1;
    }

    pthread_mutex_lock(&node->lock);
    memcpy(node->chunk->data, newdata, node->chunk->size);
    sha256_compute_chunk_hash(node);
}


int mtree_get_nchunks_from_root(mtree_node_t* node)
{
    return pow(2, node->height + 1) -1;
}