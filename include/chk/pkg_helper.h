#ifndef CHK_PKG_HELPER_H
#define CHK_PKG_HELPER_H

#include <chk/pkgchk.h>
#include <crypt/sha256.h>
#include <string.h>
#include <tree/merkletree.h>
#include <utilities/my_utils.h>

/**
 * @brief Extract the directory path from a given file path.
 *
 * @param filepath Path to the file.
 * @param filename Buffer to store the extracted directory path.
 * @param max_len Maximum length of the buffer.
 */
void extract_directory(const char* filepath, char* filename, size_t max_len);

/**
 * @brief Process and sanitize a filename from a given line.
 *
 * @param line Input line containing the filename.
 * @param filename Buffer to store the processed filename.
 * @param max_len Maximum length of the buffer.
 */
void process_filename(const char* line, char* filename, size_t max_len);

/**
 * @brief Create an empty bpkg object.
 *
 * @return Pointer to the created bpkg object.
 */
bpkg_t* bpkg_create();

/**
 * @brief Unpack the contents of a package file into a bpkg object.
 *
 * @param bpkg Pointer to the empty bpkg object.
 * @param bpkgString Package file stored as a continuous string.
 * @return 0 on success, -1 on failure.
 */
int bpkg_unpack(bpkg_t* bpkg);

/**
 * @brief Find the uppermost valid subtree root in a Merkle tree.
 *
 * @param root Root node of the subtree.
 * @param count Pointer to store the number of valid nodes found.
 * @return Array of hashes of the largest completed subtree.
 */
char** bpkg_get_largest_completed_subtree(mtree_node_t* root, int* count);

/**
 * @brief Deallocate a chunk object and its attributes.
 *
 * @param cobj Chunk object to be destroyed.
 */
void bpkg_chunk_destroy(chunk_t* cobj);

/**
 * @brief Combine nodes in a Merkle tree.
 *
 * @param mtree Pointer to the Merkle tree.
 */
void combine_nodes(mtree_t* mtree);

/**
 * @brief Retrieve all chunk hashes in a subtree.
 *
 * @param node Root node of the subtree.
 * @param numchunks Pointer to store the number of chunks found.
 * @return Array of chunk hashes.
 */
char** bpkg_get_subtree_chunks(mtree_node_t* node, int* numchunks);

/**
 * @brief Find a node by hash in a Merkle tree.
 *
 * @param mtree Pointer to the Merkle tree.
 * @param query_hash Hash of the node to find.
 * @param mode Search mode (INTERNAL, CHUNK, ALL).
 * @return Pointer to the found node, or NULL if not found.
 */
mtree_node_t* bpkg_find_node_from_hash(mtree_t* mtree, char* query_hash, int mode);

/**
 * @brief Find a node by hash and offset in a Merkle tree.
 *
 * @param root Root node of the subtree.
 * @param query_hash Hash of the node to find.
 * @param offset Expected offset of the node in the file.
 * @return Pointer to the found node, or NULL if not found.
 */
mtree_node_t* bpkg_find_node_from_hash_offset(mtree_node_t* root, char* query_hash, uint32_t offset);

#endif