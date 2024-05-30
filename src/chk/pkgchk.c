// Local Dependencies:
#include <fcntl.h>
#include <merkletree.h>
#include <my_utils.h>
#include <pkg_helper.h>
#include <pkgchk.h>
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
struct bpkg_obj* bpkg_load(const char* path)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open file...");
        return NULL;
    }

    struct stat fstats;
    if (fstat(fd, &fstats)) {
        perror("Fstat failure");
    }

    char* bpkg_data = (char*)mmap(NULL, fstats.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (bpkg_data = MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    struct bpkg_obj bpkg;

    bpkg_monoparse()



    close(fd);
    struct bpkg_obj* obj;
    return obj;
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
bpkg_query bpkg_file_check(struct bpkg_obj* bpkg)
{
    bpkg_query qobj;

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
struct bpkg_query bpkg_get_all_hashes(struct bpkg_obj* bpkg)
{
    struct bpkg_query qry;
 
    char** c_hashes = mtree_get_chunk_hashes(bpkg->mtree, 0);
 
    qry.hashes = merge_arrays(bpkg->hashes, c_hashes, bpkg->nhashes, bpkg->nchunks);
    qry.len = bpkg->nhashes + bpkg->nchunks;
    return qry;
}

/**
 * Retrieves all completed chunks of a package object
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
struct bpkg_query bpkg_get_completed_chunks(struct bpkg_obj* bpkg)
{
    struct bpkg_query qry;
    mtree_node** nodes = bpkg->mtree->hsh_nodes;
    
    char* comp_chk_hashes[SHA256_HEXLEN];
 
    for (int i=0; i < bpkg->nchunks, i++;){
        mtree_node* chk_node = nodes[i];
 
        if (strcmp(chk_node->expected_hash, chk_node->computed_hash) == 0)
        {
            comp_chk_hashes[i] = chk_node->expected_hash;
        }
        qry.hashes = comp_chk_hashes;
        qry.len = bpkg->nchunks;
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
struct bpkg_query bpkg_get_min_completed_hashes(struct bpkg_obj* bpkg){

    mtree_node* subtree_root = bpkg_get_largest_completed_subtree(bpkg->mtree->root);
    
    bpkg_query* qry;
    qry->hashes = bpkg_get_subtree_chunks(subtree_root, 0);
    qry->len = mtree_get_nchunks_from_root(node);
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
bpkg_query bpkg_get_all_chunk_hashes_from_hash(bpkg_obj* bpkg, char* query_hash)
{
    mtree_node* root = bpkg->mtree->root;
    mtree_node* node = bpkg_find_node_from_hash(root, query_hash);
    

    bpkg_query q_obj;

    q_obj.hashes = bpkg_get_subtree_chunks(node);
    return;

}



/**
 * Deallocates the query result after it has been constructed from
 * the relevant queries above.
 */
void bpkg_query_destroy(struct bpkg_query* qobj)
{
    free(qobj->hashes);
    free(qobj);
}

/**
 * Deallocates memory at the end of the program,
 * make sure it has been completely deallocated
 */
void bpkg_obj_destroy(struct bpkg_obj* bobj)
{
    free(bobj->hashes);
    mtree_destroy(bobj->mtree, bobj->nchunks + bobj->nhashes);
    free(bobj);

}