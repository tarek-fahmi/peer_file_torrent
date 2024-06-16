#include <chk/pkgchk.h>
#include <chk/pkg_helper.h>
#include <cli.h>
#include <peer_2_peer/package.h>
#include <peer_2_peer/packet.h>
#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/peer_handler.h>
#include <sys/socket.h>
#include <tree/merkletree.h>
#include <utilities/my_utils.h>
#include <netinet/in.h>
#include <stddef.h>
#include <unistd.h>
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
void cli_connect(char* ip, int port, peers_t* peers, bpkgs_t* bpkgs) {
     debug_print("Attempting to connect to %s:%d\n", ip, port);
     if ( !ip || port <= 0 ) {
          debug_print("Missing address or invalid port argument\n");
          return;
     }

     peer_t* pcheck = peers_find(peers, ip, port);
     if ( pcheck != NULL )
     {
          printf("Already connected to peer\n");
          fflush(stdout);
          return;
     }

     // Initialize the peer

     peer_t* peer = peer_create(ip, port);
     if ( !peer ) {
          perror("Memory allocation failed for peer structure\n");
          return;
     }

     peer->sock_fd = socket(AF_INET, SOCK_STREAM, 0);
     if ( peer->sock_fd < 0 ) {
          perror("Socket creation failed...");
          free(peer);
          return;
     }

     // Attempt to connect to the peer:
     struct sockaddr_in peer_addr = { .sin_family = AF_INET,
                                      .sin_port = htons(peer->port),
                                      .sin_addr.s_addr = inet_addr(ip) };

     if ( connect(peer->sock_fd, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) < 0 ) {
          fprintf(stdout, "Could not connect to request peer");
          fflush(stdout);
          close(peer->sock_fd);
          reqs_destroy(peer->reqs_q);
          free(peer);
          return;
     }

     debug_print("Successfully connected to peer %s:%d\n", ip, port);

     // Create the peer handling thread to handle bilateral communication with this peer concurrently.
     // Also add the peer to the shared list of managed peers on the heap.
     peers_add(peers, peer);
     peer_create_thread(peer, peers, bpkgs);
     printf("Connection established with peer\n");
     fflush(stdout);
}

/**
 * @brief Disconnect from a peer using IP and port
 *
 * @param ip Peer IP address
 * @param port Peer port number
 * @param peers Pointer to the peers list
 */
void cli_disconnect(char* ip, int port, peers_t* peers) {
     debug_print("Attempting to disconnect from %s:%d\n", ip, port);
     if ( !ip || port <= 0 ) {
          printf("Missing address and port argument\n");
          fflush(stdout);
          return;
     }

     peer_t* peer_target = peers_find(peers, ip, port);
     if ( peer_target == NULL ) {
          printf("Unknown peer, not connected\n");
          fflush(stdout);
          return;
     }

     // Preparing a packet to notify the peer about the disconnection
     payload_t emp = { 0 };
     pkt_t* dsn_pkt = pkt_create(PKT_MSG_DSN, 0, emp);

     // Creating a request for disconnection
     request_t* req = req_create(dsn_pkt);

     peers_remove(peers, ip, port);
     reqs_enqueue(peer_target->reqs_q, req);

     debug_print("Disconnection request for peer %s:%d has been queued\n", ip, port);

     printf("Disconnected from peer\n");
     fflush(stdout);
}

/**
 * @brief Add a package to manage
 *
 * @param filename Name of the package to add
 */
void cli_add_package(char* filename, bpkgs_t* bpkgs) {
     debug_print("Adding package: %s\n", filename);
     if ( !filename || strlen(filename) <= 1 ) {
          printf("Missing file argument\n");
          fflush(stdout);
          return;
     }

     if ( strncmp(filename, "./", 2) == 0 ) {
          filename += 2;
     }

     // If ./ is in the filename, trim the string and then save the filename as the package in
     // package directory.
     char filepath[512] = { 0 };
     snprintf(filepath, sizeof(filepath), "%s/%s", bpkgs->directory, filename);

     bpkg_t* bpkg = bpkg_load(filepath);

     if ( pkgs_add(bpkgs, bpkg) < 0 ) {
          perror("Failed to add new package to shared package resource manager\n");
     }
}

/**
 * @brief Remove a managed package
 *
 * @param ident Identifier of the package to remove
 */
void cli_rem_package(char* ident, bpkgs_t* bpkgs) {
     debug_print("Removing package with identifier: %s\n", ident);
     if ( !ident || strlen(ident) < 20 ) {
          printf("Missing identifier argument, please specify whole 1024 character or at least 20 characters\n");
          fflush(stdout);
          return;
     }
     // Remove the package from list of shared packages:
     int result = pkgs_rem(bpkgs, ident);

     if ( result < 0 ) {
          printf("Identifier provided does not match managed packages\n");
          fflush(stdout);
     }
     else {
          printf("Package has been removed\n");
          fflush(stdout);
     }
}

/**
 * @brief Report the statuses of the loaded packages
 */
void cli_report_packages(bpkgs_t* bpkgs) {
     debug_print("Reporting packages...\n");
     pthread_mutex_lock(&bpkgs->lock);

     if ( bpkgs->count == 0 ) {
          printf("No packages managed\n");
          fflush(stdout);
          pthread_mutex_unlock(&bpkgs->lock);
          return;
     }

     //Iterate through linked list of packages, and print details of each one.
     q_node_t* current = bpkgs->ls->head;
     int i = 1;

     while ( current != NULL ) {
          bpkg_t* bpkg_curr = (bpkg_t*)current->data;
          if ( !bpkg_curr || !bpkg_curr->mtree || !bpkg_curr->mtree->root ) {
               printf("Invalid package data\n");
               fflush(stdout);
               current = current->next;
               continue;
          }
          mtree_node_t* root = bpkg_curr->mtree->root;

          const char* status = ( strncmp(root->expected_hash, root->computed_hash, SHA256_HEXLEN) == 0 ) ? "COMPLETED" : "INCOMPLETE";
          printf("%d. %.32s, %s : %s\n", i, bpkg_curr->ident, bpkg_curr->filename, status);
          fflush(stdout);
          current = current->next;
          i++;
     }
     pthread_mutex_unlock(&bpkgs->lock);
}

/**
 * @brief List and ping all connected peers
 *
 * @param peers Pointer to the peers list
 */
void cli_list_peers(peers_t* peers) {
     debug_print("Listing peers...\n");
     pthread_mutex_lock(&peers->lock);

     if ( peers->npeers_cur <= 0 ) {
          printf("Not connected to any peers\n");
          fflush(stdout);
     }
     else {
          printf("Connected to:\n\n");
          fflush(stdout);

          payload_t pl = { 0 };
          pkt_t* pkt;
          request_t* req;

          // Iterate through dynamic array of peers, and print each existing peer.`
          int j = 1;
          for ( size_t i = 0; i < peers->npeers_max; i++ ) {
               peer_t* current = peers->list[i];
               if ( current != NULL ) {
                    printf("%d. %s:%d\n", j, current->ip, current->port);

                    pkt = pkt_create(PKT_MSG_PNG, 0, pl);
                    req = req_create(pkt);
                    reqs_enqueue(peers->list[i]->reqs_q, req);
                    fflush(stdout);
                    j++;
               }
          }
     }

     pthread_mutex_unlock(&peers->lock);
}

/**
 * @brief Request chunks related to a given hash
 *
 * @param args String containing the hash, IP, port, offset, and identifier
 * @param bpkgs Pointer to the packages manager
 * @param peers Pointer to the peers list
 */
void cli_fetch(char* args, bpkgs_t* bpkgs, peers_t* peers) {
     char ip[INET_ADDRSTRLEN];
     uint32_t port;
     char ident[IDENT_MAX + 1] = { 0 };
     char hash[SHA256_HEXLEN + 1] = { 0 };
     uint32_t offset = -1;

     if ( sscanf(args, "%[^:]:%i %1024s %64s %i", ip, &port, ident, hash, &offset) < 4 ) {
          printf("Missing or incorrect arguments from command\n");
          fflush(stdout);
          return;


          peer_t* peer = peers_find(peers, ip, port);
          if ( !peer ) {
               printf("Unable to request chunk, peer not in list\n");
               fflush(stdout);
               return;
          }

          bpkg_t* bpkg = pkg_find_by_ident(bpkgs, ident);
          if ( !bpkg ) {
               printf("Unable to request chunk, package is not managed\n");
               fflush(stdout);
               return;
          }

          debug_print("Requesting packet from peer...\n");

          mtree_node_t* chk_node = bpkg_find_node_from_hash(bpkg->mtree, hash, 1);

          if ( !chk_node ) {
               printf("Unable to request chunk, chunk hash does not belong to package\n");
               fflush(stdout);
               return;
          }

          // Create a request packet based on the chunk metadata, and enqueue it.
          pkt_t* pkt_tofind = pkt_prepare_request_pkt(bpkg, chk_node);
          debug_print("Requesting packet from peer...\n");
          pkt_fetch_from_peer(peer, pkt_tofind, bpkg);
     }
}

/**
 * @brief Parse and execute a command
 *
 * @param input Command input string
 * @param peers Pointer to the peers list
 * @param bpkgs Pointer to the packages manager
 */
int cli_process_command(char* input, peers_t* peers, bpkgs_t* bpkgs) {
     debug_print("Processing command: %s\n", input);
     char* command = strtok(input, " \n");
     char* arguments = strtok(NULL, "\n");

     if ( !command ) {
          printf("Invalid Input\n");
          fflush(stdout);
          return 1;
     }

     if ( strcmp(command, "CONNECT") == 0 ) {
          char ip[INET_ADDRSTRLEN];
          uint32_t port;
          if ( arguments && sscanf(arguments, "%15[^:]:%u", ip, &port) == 2 ) {
               cli_connect(ip, port, peers, bpkgs);
          }
          else {
               printf("Missing address and port argument\n");
               fflush(stdout);
          }
     }
     else if ( strcmp(command, "DISCONNECT") == 0 ) {
          char ip[INET_ADDRSTRLEN];
          uint32_t port;
          if ( arguments && sscanf(arguments, "%15[^:]:%u", ip, &port) == 2 ) {
               cli_disconnect(ip, port, peers);
          }
          else {
               printf("Missing address and port argument\n");
               fflush(stdout);
          }
     }
     else if ( strcmp(command, "ADDPACKAGE") == 0 ) {
          if ( arguments ) {
               cli_add_package(arguments, bpkgs);
          }
          else {
               printf("Missing file argument\n");
               fflush(stdout);
          }
     }
     else if ( strcmp(command, "REMPACKAGE") == 0 ) {
          if ( arguments ) {
               cli_rem_package(arguments, bpkgs);
          }
          else {
               printf("Missing identifier argument\n");
               fflush(stdout);
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
               cli_fetch(arguments, bpkgs, peers);
          }
          else {
               printf("Missing arguments from command\n");
               fflush(stdout);
          }
     }
     else if ( strcmp(command, "QUIT") == 0 ) {
          return 0;
     }
     else {
          printf("Invalid Input\n");
          fflush(stdout);
     }

     return 1;
}
