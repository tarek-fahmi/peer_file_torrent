#ifndef PEER_2_PEER_PACKAGE_H
#define PEER_2_PEER_PACKAGE_H

#include <chk/pkgchk.h>
#include <peer_2_peer/packet.h>
#include <peer_2_peer/peer_data_sync.h>
#include <tree/merkletree.h>

typedef struct bpkgs {
    queue_t* ls;  // Linked list of packages
    uint8_t count;
    pthread_mutex_t lock;
    char* directory;
} bpkgs_t;

/* Packet fetching and handling for peer communication and package management */

/**
 * @brief Fetch a packet from a specified peer.
 *
 * @param peer Pointer to the peer
 * @param pkt Packet to fetch
 * @param bpkg Associated package
 * @return int 0 on success, -1 on failure
 */
int pkt_fetch_from_peer(peer_t* peer, pkt_t* pkt, bpkg_t* bpkg);

/**
 * @brief Prepare a request packet for a chunk.
 *
 * @param bpkg Pointer to the package
 * @param node Node in the Merkle tree
 * @return pkt_t* Pointer to the prepared packet
 */
pkt_t* pkt_prepare_request_pkt(bpkg_t* bpkg, mtree_node_t* node);

/**
 * @brief Get response payload for a request.
 *
 * @param payload Incoming payload
 * @param bpkgs Pointer to package manager
 * @return payload_t Response payload
 */
payload_t payload_get_res_for_req(payload_t payload, mtree_node_t* chk_node, uint32_t offset, uint16_t size);

/**
 * @brief Attempt to install payload data into a package.
 *
 * @param bpkg Target package
 * @param payload Payload data
 * @return int 1 on success, -1 on failure
 */
int pkg_try_install_payload(bpkg_t* bpkg, payload_t payload);

/**
 * @brief Install an incoming packet's payload.
 *
 * @param pkt_in Incoming packet
 * @param peer Source peer
 * @param bpkgs Package manager
 * @return int 1 on success, -1 on failure
 */
int pkt_install(pkt_t* pkt_in, peer_t* peer, bpkgs_t* bpkgs);

/**
 * @brief Update chunk node data with payload content.
 *
 * @param chk_node Target chunk node
 * @param payload Payload data
 * @return int 0 on success, -1 on failure
 */
int pkt_chk_update_data(mtree_t* mtree, mtree_node_t* chk_node, payload_t payload);

/* Functions for managing packages within shared thread resources */

/**
 * @brief Find a package by its identifier.
 *
 * @param bpkgs Package manager
 * @param ident_qry Identifier to search for
 * @return bpkg_t* Pointer to the found package, or NULL if not found
 */
bpkg_t* pkg_find_by_ident(bpkgs_t* bpkgs, char* ident_qry);

/**
 * @brief Add a package to the package manager.
 *
 * @param bpkgs Package manager
 * @param bpkg Package to add
 * @return int 1 on success, -1 on failure
 */
int pkgs_add(bpkgs_t* bpkgs, bpkg_t* bpkg);

/**
 * @brief Remove a package by its identifier.
 *
 * @param bpkgs Package manager
 * @param ident Identifier of the package to remove
 * @return int 1 on success, -1 on failure
 */
int pkgs_rem(bpkgs_t* bpkgs, char* ident);

/**
 * @brief Initialize the package manager.
 *
 * @param directory Directory for package storage
 * @return bpkgs_t* Pointer to the initialized package manager
 */
bpkgs_t* pkgs_init(char* directory);

/**
 * @brief Destroy the package manager and free resources.
 *
 * @param bpkgs Package manager to destroy
 */
void pkgs_destroy(bpkgs_t* bpkgs);

#endif