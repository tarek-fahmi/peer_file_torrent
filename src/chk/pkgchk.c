#include <chk/pkg_helper.h>
#include <chk/pkgchk.h>
#include <crypt/sha256.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <tree/merkletree.h>
#include <utilities/my_utils.h>

// Part 1 Source Code

/**
 * @brief Load a package from a given path.
 *
 * @param path Path to the package file.
 * @return Loaded package object.
 */
bpkg_t* bpkg_load(const char* path) {
    char* sanitizedpath = sanitize_path(path);

    bpkg_t* bpkg = bpkg_create();

    debug_print("loading package file at path: %s\n", sanitizedpath);

    int fd = open(sanitizedpath, O_RDONLY);
    if ( fd < 0 ) {
        perror("Cannot open file");
        bpkg_obj_destroy(bpkg);
        free(sanitizedpath);
        return NULL;
    }

    struct stat statbuf;
    if ( fstat(fd, &statbuf) ) {
        perror("Fstat failure");
        close(fd);
        bpkg_obj_destroy(bpkg);
        free(sanitizedpath);
        return NULL;
    }

    if ( statbuf.st_size == 0 ) {
        fprintf(stderr, "File size is zero, cannot mmap\n");
        close(fd);
        free(sanitizedpath);
        bpkg_obj_destroy(bpkg);
        return NULL;
    }

    bpkg->pkg_data = (char*)mmap(NULL, statbuf.st_size, PROT_READ | PROT_WRITE,
        MAP_PRIVATE, fd, 0);
    if ( bpkg->pkg_data == MAP_FAILED ) {
        perror("Cannot mmap file");
        close(fd);
        free(sanitizedpath);
        bpkg_obj_destroy(bpkg);
        return NULL;
    }

    bpkg->pkg_size = statbuf.st_size;
    close(fd);
    extract_directory(sanitizedpath, bpkg->filename, 256);
    free(sanitizedpath);

    if ( bpkg_unpack(bpkg) != 0 ) {
        munmap(bpkg->pkg_data, statbuf.st_size);
        bpkg_obj_destroy(bpkg);
        free(sanitizedpath);
        return NULL;
    }

    debug_print("Successfully unpacked package file!\n");

    bpkg_query_t* qry = bpkg_file_check(bpkg);
    bpkg_query_destroy(qry);

    bpkg->mtree = mtree_build(bpkg->mtree, bpkg->filename);

    if ( bpkg->mtree == NULL ) {
        munmap(bpkg->pkg_data, statbuf.st_size);
        bpkg_obj_destroy(bpkg);
        return NULL;
    }

    debug_print("Successfully loaded package file!");
    return bpkg;
}

bpkg_query_t* bpkg_file_check(bpkg_t* bpkg) {
    char** hashes = my_malloc(sizeof(char*));

    if ( access(bpkg->filename, F_OK) == 0 ) {
        hashes[0] = "File Exists";
    }
    else {
        int fd = open(bpkg->filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        if ( fd < 0 ) {
            perror("Failed to create bpkg data file");
            free(hashes);
            exit(EXIT_FAILURE);
        }

        // Set the file size
        if ( ftruncate(fd, bpkg->mtree->f_size) != 0 ) {
            perror("Failed to set file size");
            close(fd);
            free(hashes);
            exit(EXIT_FAILURE);
        }

        hashes[0] = "File Created";
        close(fd);
    }

    bpkg_query_t* qobj = bpkg_qry_create(hashes, 1);
    return qobj;
}

/**
 * @brief Check if the referenced filename in the package exists.
 *
 * @param bpkg Package object.
 * @return Query result with "File Exists" or "File Created".
 */
bpkg_query_t* bpkg_get_all_hashes(bpkg_t* bpkg) {
    debug_print("Printing all hashes...\n");
    char** hashes = my_malloc(sizeof(char*) * bpkg->mtree->nnodes);

    mtree_t* mtree = bpkg->mtree;

    for ( int i = 0; i < mtree->nnodes; i++ ) {
        hashes[i] = mtree->nodes[i]->expected_hash;
    }

    bpkg_query_t* qry = bpkg_qry_create(hashes, mtree->nnodes);

    return qry;
}

/**
 * @brief Retrieve all hashes within the package/tree.
 *
 * @param bpkg Package object.
 * @return Query result with a list of hashes and their count.
 */
bpkg_query_t* bpkg_get_completed_chunks(bpkg_t* bpkg) {
    mtree_t* mtree = bpkg->mtree;
    if ( mtree == NULL ) {
        debug_print("Error: Invalid mtree structure.\n");
        return NULL;
    }

    int count = 0;
    debug_print("Running chunk check...\n\tnchunks: %u\n", mtree->nchunks);

    char** hashes = (char**)my_malloc(sizeof(char*) * mtree->nchunks);
    for ( int i = 0; i < mtree->nchunks; i++ ) {
        mtree_node_t* chk_node = mtree->chk_nodes[i];
        if ( chk_node && check_chunk(chk_node) ) {
            hashes[count] = chk_node->expected_hash;
            count++;
        }
    }

    if ( count != 0 && count != mtree->nchunks ) {
        hashes = (char**)realloc(hashes, ( count * sizeof(char*) ));
        if ( hashes == NULL ) {
            perror("Reallocation for hashes array failed...");
            bpkg_obj_destroy(bpkg);
            free(hashes);
            exit(EXIT_FAILURE);
        }
    }

    bpkg_query_t* qry = bpkg_qry_create(hashes, count);
    return qry;
}


/**
 * @brief Get the minimum set of hashes representing the current completion state.
 *
 * @param bpkg Package object.
 * @return Query result with a list of hashes and their count.
 */
bpkg_query_t* bpkg_get_min_completed_hashes(bpkg_t* bpkg) {

    int numchunks = 0;
    char** hashes =
        bpkg_get_largest_completed_subtree(bpkg->mtree->root, &numchunks);

    bpkg_query_t* qry = bpkg_qry_create(hashes, numchunks);
    return qry;
}

/**
 * @brief Retrieve all chunk hashes given an ancestor hash or itself.
 *
 * @param bpkg Package object.
 * @param query_hash Ancestor hash or chunk hash.
 * @return Query result with a list of hashes and their count.
 */
bpkg_query_t* bpkg_get_all_chunk_hashes_from_hash(bpkg_t* bpkg,
    char* query_hash) {
    debug_print("Returning all chunk hashes from hash.....");
    mtree_node_t* node = bpkg_find_node_from_hash(bpkg->mtree, query_hash, ALL);

    int nchunks = 0;
    char** hashes = bpkg_get_subtree_chunks(node, &nchunks);
    bpkg_query_t* q_obj = bpkg_qry_create(hashes, nchunks);
    return q_obj;
}

/**
 * @brief Destroy the query result, freeing allocated memory.
 *
 * @param qobj Query object to be destroyed.
 */
void bpkg_query_destroy(bpkg_query_t* qobj) {
    if ( qobj->hashes )
        free(qobj->hashes);
    if ( qobj )
        free(qobj);
}

/**
 * @brief Create a query object with given hashes.
 *
 * @param hashes Array of hash strings.
 * @param len Number of hashes.
 * @return Created query object.
 */
void bpkg_obj_destroy(bpkg_t* bobj) {
    if ( bobj ) {
        debug_print("Destroying bpkg object\n");
        if ( bobj->mtree ) {
            mtree_destroy(bobj->mtree);
            bobj->mtree = NULL;
            debug_print("Destroyed mtree\n");
        }
        if ( bobj->pkg_data ) {
            munmap(bobj->pkg_data, bobj->pkg_size);
            bobj->pkg_data = NULL;
            debug_print("Unmapped pkg_data\n");
        }
        free(bobj);
        bobj = NULL;
        debug_print("Freed bpkg object\n");
    }
}

/**
 * @brief Destroy the package object, freeing allocated memory.
 *
 * @param bobj Package object to be destroyed.
 */
bpkg_query_t* bpkg_qry_create(char** hashes, uint16_t len) {
    bpkg_query_t* qobj = (bpkg_query_t*)my_malloc(sizeof(bpkg_query_t));
    qobj->hashes = hashes;
    qobj->len = len;
    return qobj;
}

/**
 * @brief Update a chunk node with new data.
 *
 * @param chunk_node Chunk node to be updated.
 * @param newdata New data to update with.
 * @param data_size Size of the new data.
 * @return 0 on success, -1 on failure.
 */
int update_chunk_node(mtree_t* mtree, mtree_node_t* chunk_node, uint8_t* newdata, uint16_t data_size, uint32_t offset) {
    if ( chunk_node->is_leaf != 1 ) {
        return -1;
    }

    pthread_mutex_lock(&chunk_node->lock);

    size_t copy_size = ( data_size < chunk_node->chunk->size ) ? data_size : chunk_node->chunk->size;

    // Copy given data into node data:
    memcpy(mtree->f_data + offset, newdata, copy_size);
    sha256_compute_chunk_hash(chunk_node);

    pthread_mutex_unlock(&chunk_node->lock);

    // Update parent hashes to ensure they reflect the new data:
    update_parent_hashes(chunk_node);

    return 0;
}