#include <crypt/sha256.h>
#include <tree/merkletree.h>
#include <chk/pkgchk.h>
#include <chk/pkg_helper.h>
#include <utilities/my_utils.h>
#include <peer_2_peer/peer_handler.h>
#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/packet.h>
#include <peer_2_peer/package.h>
#include <config.h>
#include <cli.h>
// Standard Linux Dependencies:
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
// Additional Linux Dependencies:
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/select.h>


// Part 1 Source Code

/**
 * Gets only the required/min hashes to represent the current completion state
 * Return the smallest set of hashes of completed branches to represent
 * the completion state of the file.
 *
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
bpkg_t* bpkg_load(const char* path)
{
    bpkg_t* bpkg = (bpkg_t*) my_malloc(sizeof(bpkg_t));

    struct stat statbuf;
    if (fstat(path, &statbuf)) {
        perror("Fstat failure");
    }

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open file...");
        return NULL;
    }

    bpkg->mtree->data = (char*)mmap(NULL, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);

    if (bpkg->mtree->data = MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    close(fd);

    bpkg_unpack(bpkg);
    return;
}

/**
 * Checks to see if the referenced filename in the bpkg file
 * exists or not.
 * @param bpkg, constructed bpkg object
 * @return query_result, a single string should be
 *      printable in hashes with len sized to 1.
 * 		If the file exists, hashes[0] should contain "File Exists"
 *		If the file does not exist, hashes[0] should contain "File Created"
 */
bpkg_query_t bpkg_file_check(bpkg_t* bpkg)
{
    bpkg_query_t qobj;

    if (access(bpkg->filename, R_OK)) {
        qobj.hashes[0] = "File exists";

    } else {
        FILE* fptr = fopen(bpkg->filename, "wb");
        if (fptr == NULL) 
        {
            perror("Failed to create bpkg data file...");
        }

        fseek(fptr, bpkg->size - 1, SEEK_SET);
        fwrite("\0", 1, 1, fptr);
        qobj.hashes[0] = "File created";
    }

    return qobj;
}

/**
 * Retrieves a list of all hashes within the package/tree
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
bpkg_query_t bpkg_get_all_hashes(bpkg_t* bpkg)
{
    char* hashes[SHA256_HEXLEN];

    mtree_t* mtree = bpkg->mtree;

    for (int i=0; i < mtree->nnodes; i++)
    {
        hashes[i] = mtree->nodes[i]->expected_hash;
    }

    bpkg_query_t qry = {
        .hashes = hashes,
        .len = mtree->nnodes,
    };
    
    return;
}

/**
 * Retrieves all completed chunks of a package object
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
bpkg_query_t bpkg_get_completed_chunks(bpkg_t* bpkg)
{

    mtree_t* mtree = bpkg->mtree;
    mtree_node_t** nodes = mtree->nodes;
    
    char* comp_chk_hashes[SHA256_HEXLEN] = (char**) malloc(bpkg->nchunks * SHA256_HEXLEN);
 
    for (int i= ((mtree->nnodes - 1) / 2); i < bpkg->nchunks, i++;){

        mtree_node_t* chk_node = nodes[i];

        if (strcmp(chk_node->expected_hash, chk_node->computed_hash) == 0)
        {
            comp_chk_hashes[i] = chk_node->expected_hash;
        }

        bpkg_query_t qry = {
            .hashes = comp_chk_hashes,
            .len = bpkg->nchunks,
        };
        
    }
}

/**
 * Gets the mininum of hashes to represented the current completion state
 * Example: If chunks representing start to mid have been completed but
 * 	mid to end have not been, then we will have (N_CHUNKS/2) + 1 hashes
 * 	outputted
 *
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
bpkg_query_t bpkg_get_min_completed_hashes(bpkg_t* bpkg){

    mtree_node_t* subtree_root = bpkg_get_largest_completed_subtree(bpkg->mtree->root);
    
    bpkg_query_t* qry;
    qry->hashes = bpkg_get_subtree_chunks(subtree_root, 0);
    qry->len = mtree_get_nchunks_from_root(subtree_root);
}

/**
 * Retrieves all chunk hashes given a certain an ancestor hash (or itself)
 * Example: If the root hash was given, all chunk hashes will be outputted
 * 	If the root's left child hash was given, all chunks corresponding to
 * 	the first half of the file will be outputted
 * 	If the root's right child hash was given, all chunks corresponding to
 * 	the second half of the file will be outputted
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
bpkg_query_t bpkg_get_all_chunk_hashes_from_hash(bpkg_t* bpkg, char* query_hash)
{
    mtree_node_t* root = bpkg->mtree->root;
    mtree_node_t* node = bpkg_find_node_from_hash(root, query_hash);
    

    bpkg_query_t q_obj;

    q_obj.hashes = bpkg_get_subtree_chunks(node);
    return;
}

/**
 * Deallocates the query result after it has been constructed from
 * the relevant queries above.
 */
void bpkg_query_destroy(bpkg_query_t* qobj)
{
    free(qobj->hashes);
    free(qobj);
}

/**
 * Deallocates memory at the end of the program,
 * make sure it has been completely deallocated
 */
void bpkg_t_destroy(bpkg_t* bobj)
{
    mtree_destroy(bobj->mtree, bobj->nchunks + bobj->nhashes);
    free(bobj);
}

int bpkg_check_chunk(mtree_node_t* node)
{
    if (strncmp(node->expected_hash, node->computed_hash, SHA256_HEXLEN) == 0)
    {
        debug_print("Chunk valid!");
        return 1;
    }
    else
    {
        debug_print("Chunk invalid:(");
        return -1;
    }
}
