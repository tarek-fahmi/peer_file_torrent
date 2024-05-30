#ifndef PKG_HELPER_H
#define PKG_HELPER_H

#include <pkg_helper.h>
#include <pkgchk.h>

/**
 * @brief  Parses a chunk into the attributes of a node and chunk object.
 * @note
 * @param  node*: The data container.
 * @param  dataStart: Pointer to the start of the data from bpkg file.
 * @param  rest: Pointer to the rest of the file.
 * @retval None
 */
void chk_parse(mtree_node* node, char* dataStart, char* rest, uint8_t* data_file);

/**
 * Unpacks a key-value pair which is stored on one line.
 *
 * @param bpkg, pointer to empty bpkg object
 * @param bpkgString, package file stored in continuous and complete string.
 *
 * @return bpkg, pointer to constructed bpkg object
 */
void bpkg_monoparse(struct bpkg_obj* bpkg, char* dataStart, char* key,
                    char* rest);

/**
 * Unpacks a key-value pair which is stored on multiple lines.
 *
 * @param bpkg, pointer to empty bpkg object
 * @param bpkgString, package file stored in continuous and complete string.
 *
 * @return bpkg, pointer to constructed bpkg object
 */
void bpkg_multiparse(bpkg_obj* bpkg, char* startData, char* key, char* rest);
/**
 * Unpacks the contents of the package file into a bpkg_object.
 *
 * @param bpkg, pointer to empty bpkg object
 * @param bpkgString, package file stored in continuous and complete string.
 *
 * @return bpkg, pointer to constructed bpkg object
 */
bpkg_obj* bpkg_unpack(struct bpkg_obj* bpkg, char* bpkgString);
/**
 * Recursively find the uppermost hash which is valid.
 *
 * @param root, largest completed subtree root.
 * @return The root of the largest subtree.
 */
mtree_node* bpkg_get_largest_completed_subtree(mtree_node* root){;
/**
 * Deallocates the chunk object associated 
 * with a lead node, and any dynamically allocated
 * attributes.
*/
void bpkg_chunk_destroy(struct chunk_obj* cobj){;

/**
 * Recursively find the uppermost hash which is valid.
 *
 * @param node, the root node representing the subtree of concern.
 * @param size, the total number of nodes 
 * @return Largest completed subtree root
 */
char** bpkg_get_subtree_chunks(mtree_node* node, int* size);
/**
 * @brief Given a node hash, return the corresponding node if it exists.
 * 
 * @param node, the node representing the subtree which contains the node corresponding to the hash.
 * @param query_hash, the hash corresponding to the node we are searching for in the subtree we are searching through.
 * 
 * @returns Node corresponding to the hash we are querying in the merkle (sub)tree, returning -1 if the node doesn't exist.
*/
mtree_node* bpkg_get_node_from_hash(mtree_node* node, char* query_hash);

#endif