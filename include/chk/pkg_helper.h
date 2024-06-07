#ifndef CHK_PKG_HELPER_H
#define CHK_PKG_HELPER_H

// Local Dependencies:=
#include <crypt/sha256.h>
#include <tree/merkletree.h>
#include <chk/pkgchk.h>
#include <utilities/my_utils.h>
#include <string.h>

/**
 * Unpacks a key-value pair which is stored on one line.
 *
 * @param bpkg, pointer to empty bpkg object
 * @param bpkgString, package file stored in continuous and complete string.
 *
 * @return bpkg, pointer to constructed bpkg object
 */
void bpkg_monoparse(bpkg_t* bpkg, char* key);
/**
 * Unpacks a key-value pair which is stored on multiple lines.
 *
 * @param bpkg, pointer to empty bpkg object
 * @param bpkgString, package file stored in continuous and complete string.
 *
 * @return bpkg, pointer to constructed bpkg object
 */
void bpkg_multiparse(bpkg_t* bpkg, char* key);


/**
 * Unpacks the contents of the package file into a bpkg_tect.
 *
 * @param bpkg, pointer to empty bpkg object
 * @param bpkgString, package file stored in continuous and complete string.
 *
 * @return bpkg, pointer to constructed bpkg object
 */
void bpkg_unpack(bpkg_t* bpkg);


/**
 * Recursively find the uppermost hash which is valid.
 *
 * @param root, largest completed subtree root.
 * @return The root of the largest subtree.
 */
mtree_node_t* bpkg_get_largest_completed_subtree(mtree_node_t* root);

/**
 * Deallocates the chunk object associated 
 * with a lead node, and any dynamically allocated
 * attributes.
*/
void chunk_destroy(chunk_t* cobj);

/**
 * Recursively find the uppermost hash which is valid.
 *
 * @param node, the root node representing the subtree of concern.
 * @param size, the total number of nodes 
 * @return Largest completed subtree root
 */
char** bpkg_get_subtree_chunks(mtree_node_t* node, uint16_t tree_height);

/**
 * @brief Given a node hash, return the corresponding node if it exists.
 * 
 * @param node, the node representing the subtree which contains the node corresponding to the hash.
 * @param query_hash, the hash corresponding to the node we are searching for in the subtree we are searching through.
 * 
 * @returns Node corresponding to the hash we are querying in the merkle (sub)tree, returning -1 if the node doesn't exist.
*/
mtree_node_t* bpkg_find_node_from_hash(mtree_t* mtree, char* query_hash, enum hash_type mode);

/**
 * @brief Given a node hash, return the corresponding node if it exists.
 * 
 * @param node, the node representing the subtree which contains the node corresponding to the hash.
 * @param query_hash, the hash corresponding to the node we are searching for in the subtree we are searching through.
 * @param offset, the offset where the node should be expected to lie in the file. This will be used to narrow down recursive search.
 * 
 * @returns Node corresponding to the hash we are querying in the merkle (sub)tree, returning -1 if the node doesn't exist.
*/
mtree_node_t* bpkg_find_node_from_hash_offset(mtree_node_t* root, char* query_hash, uint32_t offset);


#endif