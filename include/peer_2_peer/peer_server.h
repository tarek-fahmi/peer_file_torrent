#ifndef PEER_2_PEER_PEER_SERVER_H
#define PEER_2_PEER_PEER_SERVER_H

#include <config.h>
#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/package.h>
#include <stdint.h>

/* Arguments for the server thread */
typedef struct server_thr_args {
     struct sockaddr_in addr_obj;
     uint16_t server_port;
     int server_fd;
     peers_t* peers;
     bpkgs_t* bpkgs;
} server_thr_args_t;

/**
 * Handles server thread operations.
 * @param args_void Pointer to server thread arguments.
 * @return NULL
 */
void* server_thread_handler(void* args_void);

/**
 * Initializes a peer-to-peer server thread.
 * @param server_fd Server file descriptor.
 * @param server_port Server port number.
 * @param server_thread Pointer to server thread.
 * @param peers Pointer to peers.
 * @param bpkgs Pointer to bpkgs.
 */
void create_p2p_server_thread(int server_fd, int server_port,
     pthread_t* server_thread, peers_t* peers, bpkgs_t* bpkgs);

/**
 * Sets up the server.
 * @param port Server port number.
 * @return Server file descriptor.
 */
int p2p_setup_server(uint16_t port);

/**
 * Listens for incoming connections.
 * @param arg Pointer to server thread arguments.
 */
void p2p_server_listening(void* arg);

/**
 * Cleans up server thread resources.
 * @param arg Pointer to server thread arguments.
 */
void server_thread_cleanup(void* arg);

#endif