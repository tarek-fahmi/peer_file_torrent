#include "peer_2_peer/peer_data_sync.h"
#include <peer_2_peer/packet.h>
#include "utilities/my_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/* Peer management functions to manage local memory of currently connected peers and their threads/requests: */

/**
 * @brief Creates a peer list with a specified maximum number of peers.
 * @param max_peers Maximum number of peers.
 * @return Pointer to the created peers_t structure.
 */
peers_t* peer_list_create(size_t max_peers) {
    peers_t* peers = (peers_t*)my_malloc(sizeof(peers_t));
    if ( !peers ) {
        perror("Failed to allocate memory for peers list");
        return NULL;
    }
    peers->npeers_cur = 0;
    peers->npeers_max = max_peers;

    if ( pthread_mutex_init(&peers->lock, NULL) != 0 ) {
        perror("Mutex init failed");
        free(peers);
        return NULL;
    }

    peers->list = (peer_t**)my_malloc(sizeof(peer_t*) * max_peers);
    if ( !peers->list ) {
        perror("Failed to allocate memory for peer list array");
        pthread_mutex_destroy(&peers->lock);
        free(peers);
        return NULL;
    }

    for ( size_t i = 0; i < max_peers; i++ ) {
        peers->list[i] = NULL;
    }

    return peers;
}

/**
 * @brief Creates a peer with specified IP and port.
 * @param ip IP address of the peer.
 * @param port Port number of the peer.
 * @return Pointer to the created peer_t structure.
 */
peer_t* peer_create(const char* ip, int port) {
    peer_t* peer = malloc(sizeof(peer_t));
    if ( !peer ) {
        perror("Failed to allocate memory for peer");
        return NULL;
    }
    strncpy(peer->ip, ip, INET_ADDRSTRLEN);
    peer->port = port;
    peer->sock_fd = -1;
    peer->reqs_q = reqs_create();
    return peer;
}

/**
 * @brief Destroys a peer and frees associated resources.
 * @param peer Peer pointer.
 */
void peer_destroy(peer_t* peer) {
    if ( !peer ) return;

    if ( peer->reqs_q ) {
        reqs_destroy(peer->reqs_q);
        peer->reqs_q = NULL;
    }

    if ( pthread_equal(pthread_self(), peer->thread) == 0 ) {
        pthread_cancel(peer->thread);
        pthread_join(peer->thread, (void**)0);
    }
    else if ( pthread_equal(pthread_self(), peer->thread) ) {
        pthread_exit(NULL);
    }
}

/**
 * @brief Adds a new peer to the peers list.
 * @param peers Pointer to the peers_t structure.
 * @param new_peer Pointer to the new peer_t structure to add.
 */
void peers_add(peers_t* peers, peer_t* new_peer) {
    if ( !peers || !new_peer ) {
        return;
    }

    pthread_mutex_lock(&peers->lock);
    if ( peers->npeers_cur >= peers->npeers_max ) {
        printf("Cannot add peer: max peers connected...\n");
    }
    else {
        for ( size_t i = 0; i < peers->npeers_max; i++ ) {
            if ( peers->list[i] == NULL ) {
                peers->list[i] = new_peer;
                peers->npeers_cur++;
                break;
            }
        }
    }
    pthread_mutex_unlock(&peers->lock);
}

/**
 * @brief Removes a peer with specified IP and port from the peers list.
 * @param peers Pointer to peers_t.
 * @param ip IP address of the peer to remove.
 * @param port Port number of the peer to remove.
 */
void peers_remove(peers_t* peers, char* ip, int port) {
    if ( !peers || !ip ) {
        perror("Invalid arguments to peers_remove");
        return;
    }

    pthread_mutex_lock(&peers->lock);

    for ( size_t i = 0; i < peers->npeers_max; i++ ) {
        if ( peers->list[i] != NULL && strcmp(peers->list[i]->ip, ip) == 0 && peers->list[i]->port == port ) {
            peers->list[i] = NULL;
            peers->npeers_cur -= 1;
            break;
        }
    }
    pthread_mutex_unlock(&peers->lock);
}

/**
 * @brief Finds a peer with specified IP and port in the peers list.
 * @param peers Pointer to peers_t .
 * @param ip IP address of the peer to find.
 * @param port Port number of the peer to find.
 * @return Pointer to the found peer_t structure, or NULL if not found.
 */
peer_t* peers_find(peers_t* peers, const char* ip, uint16_t port) {
    if ( !peers || !ip ) {
        perror("Invalid arguments to peers_find");
        return NULL;
    }

    peer_t* peer_target = NULL;
    pthread_mutex_lock(&peers->lock);

    for ( size_t i = 0; i < peers->npeers_max; i++ ) {
        if ( peers->list[i] != NULL && peers->list[i]->port == port && strcmp(peers->list[i]->ip, ip) == 0 ) {
            peer_target = peers->list[i];
            break;
        }
    }

    pthread_mutex_unlock(&peers->lock);
    return peer_target;
}

/**
 * @brief Destroys the peers list and frees all associated resources.
 * @param peers Pointer to the peers_t.
 */
request_q_t* reqs_create() {
    request_q_t* req_queue = my_malloc(sizeof(request_q_t));

    req_queue->queue = q_init();
    pthread_mutex_init(&req_queue->lock, NULL);
    pthread_cond_init(&req_queue->cond, NULL);
    req_queue->count = 0;
    req_queue->ready = 1;

    return req_queue;
}

/**
 * @brief Creates a request with a specified packet.
 * @param pkt Pointer to the packet.
 * @return Pointer to the created request_t structure.
 */
request_t* req_create(struct pkt_t* pkt) {
    if ( pkt == NULL ) {
        return NULL;
    }

    request_t* req = my_malloc(sizeof(request_t));
    req->pkt = pkt;
    req->status = WAITING;

    pthread_mutex_init(&req->lock, NULL);
    pthread_cond_init(&req->cond, NULL);

    return req;
}

/**
 * @brief Destroys a request and frees associated resources.
 * @param req Pointer to the request.
 */
void req_destroy(request_t* req) {
    if ( !req ) return;

    if ( req->pkt ) {
        // If pkt_t has dynamically allocated members, they should be freed here.
        free(req->pkt);
    }

    pthread_mutex_destroy(&req->lock);
    pthread_cond_destroy(&req->cond);
    free(req);
}

/**
 * @brief Destroys a request queue and frees all associated resources.
 * @param reqs_q Pointer to the request queue.
 */
void reqs_destroy(request_q_t* reqs_q) {
    if ( reqs_q == NULL ) {
        return;
    }

    pthread_mutex_lock(&reqs_q->lock);
    while ( !q_empty(reqs_q->queue) ) {
        request_t* req = (request_t*)q_dequeue(reqs_q->queue);
        req_destroy(req);
    }
    q_destroy(reqs_q->queue);
    pthread_cond_destroy(&reqs_q->cond);
    pthread_mutex_unlock(&reqs_q->lock);
    pthread_mutex_destroy(&reqs_q->lock);
    free(reqs_q);
}

/**
 * @brief Retrieves the next request in the queue.
 * @param reqs_q Pointer to the request queue.
 * @return Pointer to the next request_t structure, or NULL if the queue is empty.
 */
request_t* reqs_nextup(request_q_t* reqs_q) {
    if ( reqs_q == NULL || reqs_q->queue->head == NULL ) {
        return NULL;
    }

    pthread_mutex_lock(&reqs_q->lock);
    request_t* next_request = (request_t*)reqs_q->queue->head->data;
    pthread_mutex_unlock(&reqs_q->lock);

    return next_request;
}

/**
 * @brief Enqueues a request into the request queue.
 * @param reqs_q Pointer to the request queue.
 * @param request Pointer to the request.
 */
void reqs_enqueue(request_q_t* reqs_q, request_t* request) {
    if ( reqs_q == NULL || request == NULL ) return;

    pthread_mutex_lock(&reqs_q->lock);
    q_enqueue(reqs_q->queue, (void*)request);
    reqs_q->count += 1;
    pthread_mutex_unlock(&reqs_q->lock);
}

/**
 * @brief Dequeues a request from the request queue.
 * @param reqs_q Pointer to the request queue.
 * @return Pointer to the dequeued request_t structure, or NULL if the queue is empty.
 */
request_t* reqs_dequeue(request_q_t* reqs_q) {
    if ( reqs_q == NULL ) {
        return NULL;
    }

    pthread_mutex_lock(&reqs_q->lock);
    request_t* req = (request_t*)q_dequeue(reqs_q->queue);
    if ( req != NULL ) {
        reqs_q->count -= 1;
    }
    pthread_mutex_unlock(&reqs_q->lock);

    return req;
}
