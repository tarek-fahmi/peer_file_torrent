#include <chk/pkgchk.h>
#include <cli.h>
#include <peer_2_peer/package.h>
#include <peer_2_peer/packet.h>
#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/peer_handler.h>
#include <sys/socket.h>
#include <tree/merkletree.h>
#include <utilities/my_utils.h>

// Standard Linux Dependencies:
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>


/**
 * @brief Parses a command, identifying and running valid commands with
 * respective arguments.
 *
 * @param input: A string which was inputted into the CLI, allowing return of
 * the command.
 *
 * @returns Intger, 0 if valid command, -1 if invalid command.
 */

int cli_process_command(char* input, request_q_t* reqs_q, peers_t* peers, bpkgs_t* bpkgs) {
     char* command = strtok(input, " \n"); // Get the command
     char* arguments = strtok(NULL, "\n"); // Get the rest of the line

     if ( !command ) {
          printf("Invalid Input.\n");
          return 1;
     }

     if ( strcmp(command, "CONNECT") == 0 ) {
          char ip[INET_ADDRSTRLEN];
          uint32_t port;
          if ( arguments && sscanf(arguments, "%15[^:]:%u", ip, &port) == 2 ) {
               cli_connect(ip, port, reqs_q, peers, bpkgs);
          }
          else {
               printf("Missing address and port argument.\n");
          }
     }
     else if ( strcmp(command, "DISCONNECT") == 0 ) {
          char ip[INET_ADDRSTRLEN];
          uint32_t port;
          if ( arguments && sscanf(arguments, "%15[^:]:%u", ip, &port) == 2 ) {
               cli_disconnect(ip, port, peers, reqs_q);
          }
          else {
               printf("Missing address and port argument.\n");
          }
     }
     else if ( strcmp(command, "ADDPACKAGE") == 0 ) {
          if ( arguments ) {
               cli_add_package(arguments, bpkgs);
          }
          else {
               printf("Missing file argument.\n");
          }
     }
     else if ( strcmp(command, "REMPACKAGE") == 0 ) {
          if ( arguments ) {
               cli_rem_package(arguments, bpkgs);
          }
          else {
               printf("Missing identifier argument.\n");
          }
     }
     else if ( strcmp(command, "PACKAGES") == 0 ) {
          cli_report_packages(bpkgs);
     }
     else if ( strcmp(command, "PEERS") == 0 ) {
          cli_list_peers(peers);
     }
     else if ( strcmp(command, "FETCH") == 0 ) {
          if ( arguments ) {
               cli_fetch(arguments, bpkgs, peers, reqs_q);
          }
          else {
               printf("Missing arguments from command\n");
          }
     }
     else if ( strcmp(command, "QUIT") == 0 ) {
          return 0;
     }
     else {
          printf("Invalid Input.\n");
     }

     return 1;
}



/**
 *@brief Execute command triggered peer connection: Try
 */
void cli_connect(char* ip, int port, request_q_t* reqs_q, peers_t* peers,
     bpkgs_t* bpkgs) {
     if ( !ip || port <= 0 ) {
          debug_print("Missing address or invalid port argument.\n");
          return;
     }

     peer_t* peer = peer_create(ip, port);
     if ( !peer ) {
          perror("Memory allocation failed for peer structure.\n");
          return;
     }

     peer->sock_fd = socket(AF_INET, SOCK_STREAM, 0);
     if ( peer->sock_fd < 0 ) {
          perror("Socket creation failed...");
          free(peer);
          return;
     }

     struct sockaddr_in peer_addr = { .sin_family = AF_INET,
                                     .sin_port = htons(peer->port),
                                     .sin_addr.s_addr = inet_addr(ip) };

     if ( connect(peer->sock_fd, (struct sockaddr*)&peer_addr,
          sizeof(peer_addr)) < 0 ) {
          perror("Failed to connect to peer...");
          close(peer->sock_fd);
          free(peer);
          return;
     }

     printf("Connection established with peer\n");
     peer_create_thread(peer, reqs_q, peers, bpkgs);
     cli_list_peers(peers);
}

/**
 * @brief Add a package to manage.
 *
 *
 * @param filename, the name of the package to add.
 *
 * @returns -1 if unsuccessful.
 */
void cli_disconnect(char* ip, int port, peers_t* peers, request_q_t* reqs_q) {
     if ( !ip || port <= 0 ) {
          printf("Missing address and port argument.\n");
          return;
     }

     pthread_mutex_lock(&peers->lock);
     peer_t* peer_target = peers_find(peers, ip, port);
     if ( peer_target == NULL ) {
          pthread_mutex_unlock(&peers->lock);
          printf("Unknown peer, not connected.\n");
          return;
     }

     // Preparing a packet to notify the peer about the disconnection
     pkt_t* dsn_pkt = (pkt_t*)my_malloc(sizeof(pkt_t));
     if ( !dsn_pkt ) {
          perror("Failed to allocate memory for disconnection packet.\n");
          pthread_mutex_unlock(&peers->lock);
          return;
     }

     dsn_pkt->msg_code = PKT_MSG_DSN;

     // Creating a request for disconnection
     request_t* req = req_create(dsn_pkt, peer_target);
     if ( !req ) {
          perror("Failed to create disconnection request.\n");
          free(dsn_pkt);
          pthread_mutex_unlock(&peers->lock);
          return;
     }

     pthread_mutex_lock(&reqs_q->lock);
     reqs_enqueue(reqs_q, req);
     pthread_mutex_unlock(&reqs_q->lock);

     printf("Disconnection request for peer %s:%d has been queued.\n", ip,
          port);

     // Removing the peer from the list
     pthread_mutex_unlock(&peers->lock);
     peers_remove(peers, ip, port);
}
/**
 * @brief Add a package to manage.
 *
 *
 * @param filename, the name of the package to add.
 *
 * @returns -1 if unsuccessful.
 */
void cli_add_package(char* filename, bpkgs_t* bpkgs) {
     if ( !filename || strlen(filename) <= 1 ) {
          printf("Missing file argument.\n");
          return;
     }

     char filepath[512] = { 0 };  // Initialize buffer to avoid undefined behavior
     snprintf(filepath, sizeof(filepath), "%s/%s", bpkgs->directory,
          filename);  // Corrected path concatenation

     bpkg_t* bpkg = bpkg_load(filepath);
     if ( !bpkg ) {
          printf("Unable to parse bpkg file.\n");
          return;
     }

     if ( pkgs_add(bpkgs, bpkg) < 0 ) {
          perror(
               "Failed to add new package to shared package resource "
               "manager.\n");
     }
}

/**
 * @brief Remove a package that is being maintained.
 *
 * @param filename: the name of the package to remove.
 *
 * @returns -1 if unsuccessful.
 */
void cli_rem_package(char* ident, bpkgs_t* bpkgs) {
     if ( !ident || strlen(ident) < 20 ) {
          printf(
               "Missing identifier argument, please specify whole 1024 "
               "character or at least 20 characters.\n");
          return;
     }

     int result = pkgs_rem(bpkgs, ident);

     if ( result < 0 ) {
          printf("Identifier provided does not match managed packages.\n");
     }
     else {
          printf("Package has been removed.\n");
     }
}

/**
 * @briefReport the statuses of the packages loaded.
 *
 * @returns -1 if unsuccessful.
 */
void cli_report_packages(bpkgs_t* bpkgs) {
     pthread_mutex_lock(&bpkgs->lock);

     if ( bpkgs->count == 0 ) {
          printf("No packages managed.\n");
          pthread_mutex_unlock(&bpkgs->lock);
          return;
     }

     q_node_t* current = bpkgs->ls->head;
     int i = 1;

     while ( current != NULL ) {
          bpkg_t* bpkg_curr = (bpkg_t*)current->data;
          if ( !bpkg_curr || !bpkg_curr->mtree || !bpkg_curr->mtree->root ) {
               printf("Invalid package data.\n");
               current = current->next;
               continue;
          }
          mtree_node_t* root = bpkg_curr->mtree->root;

          const char* status =
               ( strncmp(root->expected_hash, root->computed_hash,
                    SHA256_HEXLEN) == 0 )
               ? "COMPLETE"
               : "INCOMPLETE";
          printf("%d. %.32s, %s : %s\n", i, bpkg_curr->ident,
               bpkg_curr->filename, status);
          current = current->next;
          i++;
     }

     pthread_mutex_unlock(&bpkgs->lock);
}
/**
 * @brief Lists and pings all connected peers.
 *
 * @returns -1 if unsuccessful.
 */
void cli_list_peers(peers_t* peers) {
     pthread_mutex_lock(&peers->lock);

     if ( peers->npeers_cur <= 0 ) {
          printf("Not connected to any peers\n");
          pthread_mutex_unlock(&peers->lock);
          return;
     }
     else {
          printf("Connected to:\n\n");
     }

     int j = 1;
     for ( size_t i = 0; i < peers->npeers_max; i++ ) {
          peer_t* current = peers->list[i];
          if ( current != NULL ) {
               printf("%d. %s:%d\n", j, current->ip, current->port);
               j++;
          }
     }

     pthread_mutex_unlock(&peers->lock);
}

/**
 * @brief Requests chunks related to a given hash.
 *
 * @param args: the string containging the hash, ip, port, offset, and
 * identifier.
 *
 * @returns -1 if unsuccessful.
 */
void cli_fetch(char* args, bpkgs_t* bpkgs, peers_t* peers,
     request_q_t* reqs_q) {
     char ip[INET_ADDRSTRLEN];
     uint32_t port;
     char ident[IDENT_MAX] = { 0 };
     char hash[SHA256_HEXLEN] = { 0 };
     uint32_t offset;

     if ( sscanf(args, "%s:%u %s %s %u", ip, &port, ident, hash, &offset) < 4 ) {
          printf("Missing arguments from command\n");
          return;
     }

     peer_t* peer = peers_find(peers, ip, port);
     if ( !peer ) {
          printf("Unable to request chunk, peer not in list\n");
          return;
     }

     bpkg_t* bpkg = pkg_find_by_ident(bpkgs, ident);
     if ( !bpkg ) {
          printf("Unable to request chunk, package is not managed\n");
          return;
     }

     mtree_node_t* chk_node =
          bpkg_find_node_from_hash_offset(bpkg->mtree->root, hash, offset);
     if ( !chk_node ) {
          printf(
               "Unable to request chunk, chunk hash does not belong to "
               "package\n");
          return;
     }

     pkt_t* pkt_tofind = pkt_prepare_request_pkt(bpkg, chk_node);

     pthread_mutex_lock(&reqs_q->lock);
     if ( pkt_fetch_from_peer(peer, pkt_tofind, bpkg, reqs_q) < 0 ) {
          printf("Failed to fetch packet from peer...\n");
     }
     else {
          printf("Successfully fetched packet from peer!\n");
     }
     pthread_mutex_unlock(&reqs_q->lock);
}
