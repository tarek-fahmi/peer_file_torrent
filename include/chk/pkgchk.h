#ifndef PKGCHK_H
#define PKGCHK_H

#include <stddef.h>
#include <stdint.h>
#include <merkletree.h>

#define IDENT_MAX (1024)
#define FILENAME_MAX (256)

#define NHASHES (h) (2^(h-1) - 1)
#define HASH_SIZE (64)

#define NCHUNKS (h) (2^(h-1) - 1)
#define CHUNK_SIZE (4096)

/**
 * Query object, allows you to assign
 * hash strings to it.
 * Typically: malloc N number of strings for hashes
 *    after malloc the space for each string
 *    Make sure you deallocate in the destroy function
 */
	typedef struct bpkg_query{
	char** hashes;
	size_t len;
} bpkg_query;


/**
 * bpkg file object, stores file metadata and contents.
 * Typically: 
 * 		malloc storage for hashes and chunks.
 * 		Make sure to deallocate all dynamic memory in destroy funciton.
 */
typedef struct bpkg_obj{
	char* ident;
	char* filename;
	uint32_t size;

	uint32_t nhashes;
	char** hashes; // Fill in size specs here...

	uint32_t nchunks;
	char** chunks;

	mtree_node* nodes;

} bpkg_obj;

typedef struct chunk_obj{
	void* data;
	uint32_t size;
	uint32_t offset;
}chunk_obj;


/**
 * Loads the package for when a value path is given
 */
struct bpkg_obj* bpkg_load(const char* path);

/**
 * Checks to see if the referenced filename in the bpkg file
 * exists or not.
 * @param bpkg, constructed bpkg object
 * @return query_result, a single string should be
 *      printable in hashes with len sized to 1.
 * 		If the file exists, hashes[0] should contain "File Exists"
 *		If the file does not exist, hashes[0] should contain "File Created"
 */
struct bpkg_query bpkg_file_check(struct bpkg_obj* bpkg);

/**
 * Retrieves a list of all hashes within the package/tree
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
struct bpkg_query bpkg_get_all_hashes(struct bpkg_obj* bpkg);

/**
 * Retrieves all completed chunks of a package object
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
struct bpkg_query bpkg_get_completed_chunks(struct bpkg_obj* bpkg);


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
struct bpkg_query bpkg_get_min_completed_hashes(struct bpkg_obj* bpkg); 


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
struct bpkg_query bpkg_get_all_chunk_hashes_from_hash(struct bpkg_obj* bpkg, char* hash);


/**
 * Deallocates the query result after it has been constructed from
 * the relevant queries above.
 */
void bpkg_query_destroy(struct bpkg_query* qry);

/**
 * Deallocates memory at the end of the program,
 * make sure it has been completely deallocated
 */
void bpkg_obj_destroy(struct bpkg_obj* obj);

#endif

