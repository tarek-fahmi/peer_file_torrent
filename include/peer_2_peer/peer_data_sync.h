#ifndef PEER_2_PEER_PEER_DATA_SYNC_H
#define PEER_2_PEER_PEER_DATA_SYNC_H

#include <utilities/my_utils.h>
#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/packet.h>
/* Peer and peer pool shared management resources: */

typedef struct{
    char ip[INET_ADDRSTRLEN];
    int port;
    uint16_t sock_fd;
    pthread_t thread;
} peer_t;

typedef struct{
    peer_t** list; // Dynamic array
    size_t npeers_cur; // Current number of peers on local network.
    size_t npeers_max; // Max number of peers on local network.
    pthread_mutex_t lock; //Mutex lock for peer adding and removing
} peers_t;

/* Peer and peer pool shared management resources: */

enum RequestStatus{
    WAITING = 0,
    FAILED = -1,
    SUCCESS = 1,  
};

typedef struct request{
    pkt_t* pkt;
    peer_t* peer;
    enum RequestStatus status;
    pthread_mutex_t lock;
    pthread_cond_t cond;
}request_t;

typedef struct request_q{ 
    queue_t* queue;
    size_t count;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int ready;
}request_q_t;

typedef struct bpkgs{
    queue_t* ls;
    uint8_t count;
    pthread_mutex_t* lock;
    char* directory;
}bpkgs_t;


/**
 * @brief Initialize the peer list, setting every element in the peer list to NULL.
 * @params: peer list and max number of peers for the program to reference.
*/
void peer_list_init(peers_t *peers, size_t max_peers);

void peer_init(peer_t* peer, char* ip, int port);

/**
 * @brief Add a peer to the list of peers, initializing its attributes based on the arguments provided.
 *         This will be called in the connect function...
*/
void peers_add(peers_t *peers, peer_t* new_peer);

// Remove a peer from the peer list
void peers_remove(peers_t *peers, char* ip, int port);

peer_t* peers_find(peers_t *peers, const char *ip, uint16_t port);

void peer_outgoing_requests_destroy(peer_t* peer, request_q_t* reqs_q);

request_q_t* reqs_q_init() ;

request_t* req_init(pkt_t* pkt, peer_t* peer);

void req_destroy(request_t* req);

void reqs_destroy(request_q_t* reqs_q);

request_t* reqs_q_nextup(request_q_t* reqs_q);

void req_enqueue(request_q_t* reqs_q, request_t* request);

request_t* req_dequeue(request_q_t* reqs_q);

#endif