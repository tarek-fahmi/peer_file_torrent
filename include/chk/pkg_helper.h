#ifndef CHK_PKG_HELPER_H
#define CHK_PKG_HELPER_H

// Local Dependencies:=
#include <crypt/sha256.h>
#include <tree/merkletree.h>
#include <utilities/my_utils.h>

// Helper Functions:
/**
 * @brief  Parses a chunk into the attributes of a node and chunk object.
 * @note
 * @param  node*: The data container.
 * @param  dataStart: Pointer to the start of the data from bpkg file.
 * @param  rest: Pointer to the rest of the file.
 * @retval None
 */ 
void chk_parse(mtree_node_t* node, char* dataStart, char* rest, uint8_t* data_file);
/**
 * Unpacks a key-value pair which is stored on one line.
 *
 * @param bpkg, pointer to empty bpkg object
 * @param bpkgString, package file stored in continuous and complete string.
 *
 * @return bpkg, pointer to constructed bpkg object
 */
void bpkg_monoparse(bpkg_t* bpkg, char* dataStart, char* key,
                    char* rest);
/**
 * Unpacks a key-value pair which is stored on multiple lines.
 *
 * @param bpkg, pointer to empty bpkg object
 * @param bpkgString, package file stored in continuous and complete string.
 *
 * @return bpkg, pointer to constructed bpkg object
 */
void bpkg_multiparse(bpkg_t* bpkg, char* startData, char* key, char* rest);


/**
 * Unpacks the contents of the package file into a bpkg_tect.
 *
 * @param bpkg, pointer to empty bpkg object
 * @param bpkgString, package file stored in continuous and complete string.
 *
 * @return bpkg, pointer to constructed bpkg object
 */
int bpkg_unpack(bpkg_t* bpkg);


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
char** bpkg_get_subtree_chunks(mtree_node_t* node);

/**
 * @brief Given a node hash, return the corresponding node if it exists.
 * 
 * @param node, the node representing the subtree which contains the node corresponding to the hash.
 * @param query_hash, the hash corresponding to the node we are searching for in the subtree we are searching through.
 * 
 * @returns Node corresponding to the hash we are querying in the merkle (sub)tree, returning -1 if the node doesn't exist.
*/
mtree_node_t* bpkg_get_node_from_hash(mtree_t* mtree, char* query_hash);


#endif