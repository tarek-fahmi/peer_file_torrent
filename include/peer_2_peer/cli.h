#ifndef PEER_2_PEER_CLI_H
#define PEER_2_PEER_CLI_H

#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/peer_handler.h>
#include <peer_2_peer/peer_server.h>
#include <netinet/in.h>
#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/peer_handler.h>
#include <peer_2_peer/packet.h>
#include <peer_2_peer/package.h>
#include <chk/pkgchk.h>
#include <sys/socket.h>
#include <tree/merkletree.h>
#include <utilities/my_utils.h>
#include <cli.h>

// Standard Linux Dependencies:
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/**
 * @brief Connect to a peer using IP and port
 *
 * @param ip Peer IP address
 * @param port Peer port number
 * @param peers Pointer to the peers list
 * @param bpkgs Pointer to the packages manager
 */
void cli_connect(char* ip, int port, peers_t* peers, bpkgs_t* bpkgs);

/**
 * @brief Disconnect from a peer using IP and port
 *
 * @param ip Peer IP address
 * @param port Peer port number
 * @param peers Pointer to the peers list
 */
void cli_disconnect(char* ip, int port, peers_t* peers);

/**
 * @brief Add a package to manage
 *
 * @param filename Name of the package to add
 */
void cli_add_package(char* filename, bpkgs_t* bpkgs);

/**
 * @brief Remove a managed package
 *
 * @param ident Identifier of the package to remove
 */
void cli_rem_package(char* ident, bpkgs_t* bpkgs);

/**
 * @brief Report the statuses of the loaded packages
 */
void cli_report_packages(bpkgs_t* bpkgs);

/**
 * @brief List and ping all connected peers
 *
 * @param peers Pointer to the peers list
 */
void cli_list_peers(peers_t* peers);

/**
 * @brief Request chunks related to a given hash
 *
 * @param args String containing the hash, IP, port, offset, and identifier
 * @param bpkgs Pointer to the packages manager
 * @param peers Pointer to the peers list
 */
void cli_fetch(char* args, bpkgs_t* bpkgs, peers_t* peers);

/**
 * @brief Parse and execute a command
 *
 * @param input Command input string
 * @param peers Pointer to the peers list
 * @param bpkgs Pointer to the packages manager
 */
int cli_process_command(char* input, peers_t* peers, bpkgs_t* bpkgs);

#endif