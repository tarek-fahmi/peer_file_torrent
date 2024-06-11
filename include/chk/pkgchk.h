#ifndef CHK_PKGCHK_H
#define CHK_PKGCHK_H

#include <chk/pkgchk.h>
#include <chk/pkg_helper.h>
#include <tree/merkletree.h>
#include <utilities/my_utils.h>
#include <stddef.h>

#define IDENT_MAX (1024)
#define CHUNK_SIZE (4096)

/**
 * @brief Query object to hold hash strings.
 * Make sure to allocate and deallocate memory properly.
 */
typedef struct bpkg_query {
    char** hashes;  ///< Array of hash strings
    size_t len;     ///< Number of hashes
} bpkg_query_t;

enum search_mode {
    INTERNAL,
    CHUNK,
    ALL,
};

// Part 1 Source Code

/**
 * @brief Load a package from a given path.
 *
 * @param path Path to the package file.
 * @return Loaded package object.
 */
bpkg_t* bpkg_load(const char* path);

/**
 * @brief Check if the referenced filename in the package exists.
 *
 * @param bpkg Package object.
 * @return Query result with "File Exists" or "File Created".
 */
bpkg_query_t* bpkg_file_check(bpkg_t* bpkg);

/**
 * @brief Retrieve all hashes within the package/tree.
 *
 * @param bpkg Package object.
 * @return Query result with a list of hashes and their count.
 */
bpkg_query_t* bpkg_get_all_hashes(bpkg_t* bpkg);

/**
 * @brief Retrieve all completed chunks of a package.
 *
 * @param bpkg Package object.
 * @return Query result with a list of hashes and their count.
 */
bpkg_query_t* bpkg_get_completed_chunks(bpkg_t* bpkg);

/**
 * @brief Get the minimum set of hashes representing the current completion state.
 *
 * @param bpkg Package object.
 * @return Query result with a list of hashes and their count.
 */
bpkg_query_t* bpkg_get_min_completed_hashes(bpkg_t* bpkg);

/**
 * @brief Retrieve all chunk hashes given an ancestor hash or itself.
 *
 * @param bpkg Package object.
 * @param query_hash Ancestor hash or chunk hash.
 * @return Query result with a list of hashes and their count.
 */
bpkg_query_t* bpkg_get_all_chunk_hashes_from_hash(bpkg_t* bpkg, char* query_hash);

/**
 * @brief Destroy the query result, freeing allocated memory.
 *
 * @param qobj Query object to be destroyed.
 */
void bpkg_query_destroy(bpkg_query_t* qobj);

/**
 * @brief Create a query object with given hashes.
 *
 * @param hashes Array of hash strings.
 * @param len Number of hashes.
 * @return Created query object.
 */
bpkg_query_t* bpkg_qry_create(char** hashes, uint16_t len);

/**
 * @brief Destroy the package object, freeing allocated memory.
 *
 * @param bobj Package object to be destroyed.
 */
void bpkg_obj_destroy(bpkg_t* bobj);

/**
 * @brief Update a chunk node with new data.
 *
 * @param chunk_node Chunk node to be updated.
 * @param newdata New data to update with.
 * @param data_size Size of the new data.
 * @return 0 on success, -1 on failure.
 */
int update_chunk_node(mtree_t* mtree, mtree_node_t* chunk_node, uint8_t* newdata, uint16_t data_size, uint32_t offset);

#endif