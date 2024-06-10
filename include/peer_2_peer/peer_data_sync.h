#include "peer_2_peer/packet.h"
#include "utilities/my_utils.h"

#ifndef PEER_2_PEER_PEER_DATA_SYNC_H
#define PEER_2_PEER_PEER_DATA_SYNC_H

/* Peer and peer pool shared management resources: */

#define DATA_MAX (2998)

typedef struct {
     char ip[INET_ADDRSTRLEN];
     int port;
     uint16_t sock_fd;
     pthread_t thread;
} peer_t;

typedef struct {
     peer_t** list;         // Dynamic array
     size_t npeers_cur;     // Current number of peers on local network.
     size_t npeers_max;     // Max number of peers on local network.
     pthread_mutex_t lock;  // Mutex lock for peer adding and removing
} peers_t;

/* Peer and peer pool shared management resources: */

enum RequestStatus {
     WAITING = 0,
     FAILED = -1,
     SUCCESS = 1,
};

typedef struct request {
     struct pkt_t* pkt;
     peer_t* peer;
     enum RequestStatus status;
     pthread_mutex_t lock;
     pthread_cond_t cond;
} request_t;

typedef struct request_q {
     queue_t* queue;  // Make sure queue_t is declared somewhere
     size_t count;
     pthread_mutex_t lock;
     pthread_cond_t cond;
     int ready;
} request_q_t;

typedef struct bpkgs {
     queue_t* ls;  // Make sure queue_t is declared somewhere
     uint8_t count;
     pthread_mutex_t lock;
     char* directory;
} bpkgs_t;

/**
 * @brief Initialize the peer list, setting every element in the peer list to
 * NULL.
 * @params: peer list and max number of peers for the program to reference.
 */
peers_t* peer_list_create(size_t max_peers);

peer_t* peer_create(const char* ip, int port);

/**
 * @brief Add a peer to the list of peers, initializing its attributes based on
 * the arguments provided. This will be called in the connect function...
 */
void peers_add(peers_t* peers, peer_t* new_peer);

// Remove a peer from the peer list
void peers_remove(peers_t* peers, char* ip, int port);

void peers_destroy(peers_t* peers);

peer_t* peers_find(peers_t* peers, const char* ip, uint16_t port);

void peer_outgoing_requests_destroy(peer_t* peer, request_q_t* reqs_q);

request_q_t* reqs_create();

request_t* req_create(struct pkt_t* pkt, peer_t* peer);

void req_destroy(request_t* req);

void reqs_destroy(request_q_t* reqs_q);

request_t* reqs_nextup(request_q_t* reqs_q);

void reqs_enqueue(request_q_t* reqs_q, request_t* request);

request_t* reqs_dequeue(request_q_t* reqs_q);

#endif