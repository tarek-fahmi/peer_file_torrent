#ifndef PEER_H
#define PPER_H


// Standard Linux Dependencies:
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// Additional Linux Dependencies:
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>

//Request queue structure

// Define the Peer and PeerList structures
typedef struct peer{
    char ip[INET_ADDRSTRLEN];
    int port;
    uint16_t sock_fd;
    pthread_t thread;
} peer_t;

typedef struct server{
    struct sockaddr_in addr_obj;
    uint16_t sock;
    int port;
}server_t;



typedef struct peers{
    peer_t** list; // Dynamic array

    size_t npeers_cur; // Current number of peers on local network.
    size_t npeers_max; // Max number of peers on local network.

    pthread_mutex_t lock; //Mutex lock for peer adding and removing.
} peers_t;



// Initialize the peer list
void peer_list_init(peers_t *peer_list, size_t max_peers);

void peer_init(peer_t* peer, char* ip, int port);

// Add a peer to the peer list
void peers_add(peers_t *peer_list, peer_t* peer);

// Remove a peer from the peer list
void peers_remove(peers_t *peer_list, const char *ip, uint16_t port);

peer_t* peers_find(peers_t *peer_list, const char *ip, uint16_t port, int sock_fd);

#endif