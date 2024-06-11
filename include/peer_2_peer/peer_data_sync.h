#ifndef PEER_2_PEER_PEER_DATA_SYNC_H
#define PEER_2_PEER_PEER_DATA_SYNC_H

#include "utilities/my_utils.h"
#include <peer_2_peer/packet.h>

/* Enumeration for request status */
enum RequestStatus {
     WAITING = 0,
     FAILED = -1,
     SUCCESS = 1,
};

/* Data structure for request queue communication */
typedef struct request_q {
     queue_t* queue;
     size_t count;
     pthread_mutex_t lock;
     pthread_cond_t cond;
     int ready;
} request_q_t;

/* Structure representing each peer in the network */
typedef struct peer {
     char ip[INET_ADDRSTRLEN];
     int port;
     uint16_t sock_fd;
     pthread_t thread;
     request_q_t* reqs_q;
}peer_t;

/* Structure for managing peer communication requests */
typedef struct request {
     struct pkt_t* pkt;
     enum RequestStatus status;
     pthread_mutex_t lock;
     pthread_cond_t cond;
} request_t;

/* Structure for managing a dynamic array of peers */
typedef struct peers {
     struct peer** list;
     size_t npeers_cur;     // Current number of peers.
     size_t npeers_max;     // Maximum number of peers.
     pthread_mutex_t lock;  // Mutex for peer addition/removal.
} peers_t;

/* Peer management functions */

/**
 * @brief Creates a peer list with a specified maximum number of peers.
 * @param max_peers Maximum number of peers.
 * @return Pointer to the created peers_t structure.
 */
peers_t* peer_list_create(size_t max_peers);

/**
 * @brief Creates a peer with specified IP and port.
 * @param ip IP address of the peer.
 * @param port Port number of the peer.
 * @return Pointer to the created peer_t structure.
 */
peer_t* peer_create(const char* ip, int port);

/**
 * @brief Adds a new peer to the peers list.
 * @param peers Pointer to the peers_t structure.
 * @param new_peer Pointer to the new peer_t structure to add.
 */
void peers_add(peers_t* peers, peer_t* new_peer);

/**
 * @brief Removes a peer with specified IP and port from the peers list.
 * @param peers Pointer to peers_t.
 * @param ip IP address of the peer to remove.
 * @param port Port number of the peer to remove.
 */
void peers_remove(peers_t* peers, char* ip, int port);

/**
 * @brief Destroys a peer and frees associated resources.
 * @param peer Peer pointer.
 */
void peer_destroy(peer_t* peer);

/**
 * @brief Finds a peer with specified IP and port in the peers list.
 * @param peers Pointer to peers_t .
 * @param ip IP address of the peer to find.
 * @param port Port number of the peer to find.
 * @return Pointer to the found peer_t structure, or NULL if not found.
 */
peer_t* peers_find(peers_t* peers, const char* ip, uint16_t port);

/* Request queue management functions */

/**
 * @brief Creates a request queue.
 * @return Pointer to the created request_q_t structure.
 */
request_q_t* reqs_create();

/**
 * @brief Creates a request with a specified packet.
 * @param pkt Pointer to the packet.
 * @return Pointer to the created request_t structure.
 */
request_t* req_create(struct pkt_t* pkt);

/**
 * @brief Destroys a request and frees associated resources.
 * @param req Pointer to the request.
 */
void req_destroy(request_t* req);

/**
 * @brief Destroys a request queue and frees all associated resources.
 * @param reqs_q Pointer to the request queue.
 */
void reqs_destroy(request_q_t* reqs_q);

/**
 * @brief Retrieves the next request in the queue.
 * @param reqs_q Pointer to the request queue.
 * @return Pointer to the next request_t structure, or NULL if the queue is empty.
 */
request_t* reqs_nextup(request_q_t* reqs_q);

/**
 * @brief Enqueues a request into the request queue.
 * @param reqs_q Pointer to the request queue.
 * @param request Pointer to the request.
 */
void reqs_enqueue(request_q_t* reqs_q, request_t* request);

/**
 * @brief Dequeues a request from the request queue.
 * @param reqs_q Pointer to the request queue.
 * @return Pointer to the dequeued request_t structure, or NULL if the queue is empty.
 */
request_t* reqs_dequeue(request_q_t* reqs_q);

#endif