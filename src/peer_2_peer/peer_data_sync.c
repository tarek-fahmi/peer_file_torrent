#include <cli.h>
#include <config.h>
#include <peer_2_peer/packet.h>
#include <utilities/my_utils.h>

/**
 * @brief Initialize the peer list, setting every element in the peer list to
 * NULL.
 * @params: peer list and max number of peers for the program to reference.
 */
peers_t* peer_list_create(size_t max_peers) {
    // Allocate memory for the peers_t structure
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

peer_t* peer_create(const char* ip, int port) {
    peer_t* peer = my_malloc(sizeof(peer_t));
    if ( !peer ) {
        perror("Failed to allocate memory for peer");
        return NULL;
    }

    peer->port = port + 0;

    strncpy(peer->ip, ip, INET_ADDRSTRLEN);
    peer->ip[INET_ADDRSTRLEN - 1] = '\0';  // Ensure null termination

    return peer;
}

void peers_remove(peers_t* peers, char* ip, int port) {
    if ( !peers || !ip ) {
        perror("Invalid arguments to peers_remove");
        return;
    }

    pthread_mutex_lock(&peers->lock);

    for ( size_t i = 0; i < peers->npeers_max; i++ ) {
        if ( peers->list[i] != NULL && strcmp(peers->list[i]->ip, ip) == 0 &&
            peers->list[i]->port == port ) {
            free(peers->list[i]);
            peers->list[i] = NULL;
            peers->npeers_cur -= 1;
            break;
        }
    }

    pthread_mutex_unlock(&peers->lock);
}

peer_t* peers_find(peers_t* peers, const char* ip, uint16_t port) {
    if ( !peers || !ip ) {
        perror("Invalid arguments to peers_find");
        return NULL;
    }

    peer_t* peer_target = NULL;
    pthread_mutex_lock(&peers->lock);

    for ( size_t i = 0; i < peers->npeers_max; i++ ) {
        if ( peers->list[i] != NULL && peers->list[i]->port == port &&
            strcmp(peers->list[i]->ip, ip) == 0 ) {
            peer_target = peers->list[i];
            break;
        }
    }

    pthread_mutex_unlock(&peers->lock);
    return peer_target;
}

/**
 * @brief Add a peer to the list of peers, initializing its attributes based on
 * the arguments provided. This will be called in the connect function...
 */
void peers_add(peers_t* peers, peer_t* new_peer) {
    if ( !peers || !new_peer ) {
        perror("Invalid arguments to peers_add");
        return;
    }

    pthread_mutex_lock(&peers->lock);

    if ( peers->npeers_cur >= peers->npeers_max ) {
        perror("Cannot add peer: max peers connected...\n");
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

request_q_t* reqs_create() {
    request_q_t* req_queue = my_malloc(sizeof(request_q_t));

    req_queue->queue = q_init();
    pthread_mutex_init(&req_queue->lock, NULL);
    pthread_cond_init(&req_queue->cond, NULL);
    req_queue->count = 0;
    req_queue->ready = 1;

    return req_queue;
}

request_t* req_create(pkt_t* pkt, peer_t* peer) {
    if ( pkt == NULL || peer == NULL ) {
        return NULL;
    }

    request_t* req = my_malloc(sizeof(request_t));
    req->pkt = pkt;
    req->peer = peer;
    req->status = WAITING;

    pthread_mutex_init(&req->lock, NULL);
    pthread_cond_init(&req->cond, NULL);

    return req;
}

void reqs_destroy(request_q_t* reqs_q) {
    if ( reqs_q == NULL ) {
        return;
    }

    pthread_mutex_lock(&reqs_q->lock);
    q_destroy(
        reqs_q->queue);  // Assume q_destroy handles all elements in the queue
    pthread_cond_destroy(&reqs_q->cond);
    pthread_mutex_unlock(&reqs_q->lock);
    pthread_mutex_destroy(&reqs_q->lock);
    free(reqs_q);
}

request_t* reqs_nextup(request_q_t* reqs_q) {
    if ( reqs_q == NULL || reqs_q->queue->head == NULL ) {
        return NULL;
    }

    pthread_mutex_lock(&reqs_q->lock);
    request_t* next_request = (request_t*)reqs_q->queue->head->data;
    pthread_mutex_unlock(&reqs_q->lock);

    return next_request;
}

void reqs_enqueue(request_q_t* reqs_q, request_t* request) {
    if ( reqs_q == NULL || request == NULL ) return;

    pthread_mutex_lock(&reqs_q->lock);
    q_enqueue(reqs_q->queue, (void*)request);
    reqs_q->count += 1;
    pthread_mutex_unlock(&reqs_q->lock);
}

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

void peer_outgoing_requests_destroy(peer_t* peer, request_q_t* reqs_q) {
    if ( !peer || !reqs_q ) return;

    pthread_mutex_lock(&reqs_q->lock);
    q_node_t* current = reqs_q->queue->head;
    q_node_t* prev = NULL;

    while ( current != NULL ) {
        request_t* req = (request_t*)current->data;
        if ( req && req->peer == peer ) {
            q_node_t* temp = current;
            current = current->next;
            if ( prev ) {
                prev->next = current;
            }
            else {
                reqs_q->queue->head = current;
            }
            if ( current == NULL ) {
                reqs_q->queue->tail = prev;
            }
            req_destroy(req);
            free(temp);
            reqs_q->count--;
        }
        else {
            prev = current;
            current = current->next;
        }
    }

    pthread_mutex_unlock(&reqs_q->lock);
}

void peers_destroy(peers_t* peers) {
    if ( !peers ) return;

    pthread_mutex_lock(&peers->lock);

    for ( size_t i = 0; i < peers->npeers_max; ++i ) {
        if ( peers->list[i] ) {
            if ( peers->list[i]->sock_fd >= 0 ) {
                close(peers->list[i]->sock_fd);
            }
            free(peers->list[i]);
        }
    }

    free(peers->list);
    pthread_mutex_unlock(&peers->lock);
    pthread_mutex_destroy(&peers->lock);
}

void req_destroy(request_t* req) {
    if ( !req ) return;

    if ( req->pkt ) {
        free(req->pkt);
    }

    pthread_mutex_destroy(&req->lock);
    pthread_cond_destroy(&req->cond);
    free(req);
}