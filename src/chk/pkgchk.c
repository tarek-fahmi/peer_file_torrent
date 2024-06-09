#include <crypt/sha256.h>
#include <tree/merkletree.h>
#include <chk/pkgchk.h>
#include <chk/pkg_helper.h>
#include <utilities/my_utils.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdio.h>


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
bpkg_t* bpkg_load(const char* path) {
    bpkg_t* bpkg = bpkg_create();
    if (!bpkg) return NULL;

    int fd = open(path, O_RDWR);
    if (fd < 0) {
        perror("Cannot open file\n");
        bpkg_obj_destroy(bpkg);
        return NULL;
    }

    struct stat statbuf;
    if (fstat(fd, &statbuf)) {
        perror("Fstat failure\n");
        close(fd);
        bpkg_obj_destroy(bpkg);
        return NULL;
    }

    bpkg->pkg_data = (char*)mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (bpkg->pkg_data == MAP_FAILED) {
        perror("Cannot mmap file\n");
        close(fd);
        bpkg_obj_destroy(bpkg);
        return NULL;
    }

    bpkg->pkg_size = statbuf.st_size;
    close(fd);

    if (bpkg_unpack(bpkg) != 0) {
        munmap(bpkg->pkg_data, statbuf.st_size);
        bpkg_obj_destroy(bpkg);
        return NULL;
    }

    bpkg_query_t* qry = bpkg_file_check(bpkg);
    bpkg_query_destroy(qry);

    debug_print("Successfully unpacked package file!\n");

    if (mtree_build(bpkg->mtree, bpkg->filename) == NULL) {
        munmap(bpkg->pkg_data, statbuf.st_size);
        bpkg_obj_destroy(bpkg);
        return NULL;
    }

    debug_print("Successfully built merkle tree!\n");
    return bpkg;
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
bpkg_query_t* bpkg_file_check(bpkg_t* bpkg)
{
    char** hashes = (char**) my_malloc(sizeof(char*));

    if (access(bpkg->filename, F_OK) == 0) 
    {
        hashes[0] = "File exists";
    } 
    else 
    {
        FILE* fptr = fopen(bpkg->filename, "wb");
        if (fptr == NULL) 
        {
            perror("Failed to create bpkg data file...");
            hashes[0] = "File creation failed";
        }
        fclose(fptr);
        hashes[0] = "File created";
    }
    bpkg_query_t* qobj = bpkg_qry_create(hashes, 1);
    return qobj;
}

/**
 * Retrieves a list of all hashes within the package/tree
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
bpkg_query_t* bpkg_get_all_hashes(bpkg_t* bpkg)
{
    char** hashes = my_malloc(sizeof(char*) * bpkg->mtree->nnodes);
 
    mtree_t* mtree = bpkg->mtree;

    for (int i=0; i < mtree->nnodes; i++)
    {
        hashes[i] = mtree->nodes[i]->expected_hash;
    }

    bpkg_query_t* qry = bpkg_qry_create(hashes, mtree->nnodes);
    
    return qry;
}

/**
 * Retrieves all completed chunks of a package object
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
bpkg_query_t* bpkg_get_completed_chunks(bpkg_t* bpkg)
{
    mtree_t* mtree = bpkg->mtree;
    char** hashes;

    debug_print("Running chunk check...\n\tnchunks: %u\n",mtree->nchunks);

    char** temp = (char**)malloc(mtree->nchunks * sizeof(char*));
    int count = 0;
    for (int i = 0; i < mtree->nchunks; i++) {
        mtree_node_t* chk_node = mtree->chk_nodes[i];
        if (chk_node->is_complete){
            temp[count] = chk_node->expected_hash;
            count++;
        }
    }
    if (count != 0){
        hashes = (char**) realloc(temp, (sizeof(char*) * count));
    }
    else{
        free(temp);
    }

    bpkg_query_t* qry = bpkg_qry_create(hashes, count);
    return qry;
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
bpkg_query_t* bpkg_get_min_completed_hashes(bpkg_t* bpkg){

    mtree_node_t* subtree_root = bpkg_get_largest_completed_subtree(bpkg->mtree->root);
    if (!subtree_root) debug_print("Failed to find a subtree root meeting the criteria...\n");
    debug_print("Largest completed subtree root found at depth %u", subtree_root->depth);
    int len = 0;
    char** hashes = bpkg_get_subtree_chunks(subtree_root, &len);
    bpkg_query_t* qry = bpkg_qry_create(hashes, len);

    return qry;
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

bpkg_query_t* bpkg_get_all_chunk_hashes_from_hash(bpkg_t* bpkg, char* query_hash)
{

    mtree_node_t* node = bpkg_find_node_from_hash(bpkg->mtree, query_hash, EXPECTED);

    int nchunks = 0;
    char** hashes = bpkg_get_subtree_chunks(node, &nchunks);
    bpkg_query_t* q_obj = bpkg_qry_create(hashes, nchunks);
    return q_obj;
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
void bpkg_obj_destroy(bpkg_t* bobj) {
    if (bobj) {
        debug_print("Destroying bpkg object\n");
        if (bobj->mtree) {
            mtree_destroy(bobj->mtree);
            bobj->mtree = NULL;
            debug_print("Destroyed mtree\n");
        }
        if (bobj->pkg_data) {
            munmap(bobj->pkg_data, bobj->pkg_size);
            bobj->pkg_data = NULL;
            debug_print("Unmapped pkg_data\n");
        }
        free(bobj);
        bobj = NULL;
        debug_print("Freed bpkg object\n");
    }
}




bpkg_query_t* bpkg_qry_create(char** hashes, uint16_t len)
{
    bpkg_query_t* qobj = (bpkg_query_t*) my_malloc(sizeof(bpkg_query_t));
    qobj->hashes = hashes;
    qobj->len = len;
    return qobj;
}
