#include <crypt/sha256.h>
#include <tree/merkletree.h>
#include <chk/pkgchk.h>
#include <chk/pkg_helper.h>
#include <utilities/my_utils.h>
#include <peer_2_peer/peer_handler.h>
#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/packet.h>
#include <peer_2_peer/package.h>
#include <config.h>
#include <cli.h>
// Standard Linux Dependencies:
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
// Additional Linux Dependencies:
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/select.h>

typedef struct request_q{ 
    queue_t* queue;
    size_t count;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int ready;
}request_q_t;


/**
 * @brief Initialize the peer list, setting every element in the peer list to NULL.
 * @params: peer list and max number of peers for the program to reference.
*/
void peer_list_init(peers_t *peers, size_t max_peers) 
{
    peers->npeers_cur = 0;
    pthread_mutex_init(&peers->lock, NULL);
    peers->list = (peers_t**) my_malloc(sizeof(peer_t*) * max_peers);

    // Initialize all peers to NULL
    for(int i=0; i <max_peers; i++){
        peers->list[i] = NULL;
    }
}

void peer_init(peer_t* peer, char* ip, int port)
{
        peer->port = port;
        memcpy(peer->ip, ip, sizeof(peer->ip));
        
        pthread_create(&peer->thread);
}

/**
 * @brief Add a peer to the list of peers, initializing its attributes based on the arguments provided.
 *         This will be called in the connect function...
*/
void peers_add(peers_t *peers, peer_t* new_peer) 
{
    pthread_mutex_lock(&peers->lock);

    if(peers->npeers_cur >= peers->npeers_max - 1){
        debug_print("Cannot add peer: max peers connected...");
        pthread_mutex_unlock(&peers->lock);
        return;
    }

    for(int i = 0; i < peers->npeers_max; i++)
    {
        if (peers->list[i] == NULL)
        {
            peers->list[i] = new_peer;
            break;
        }
    }
    peers->npeers_cur += 1;
    pthread_mutex_unlock(&peers->lock);
}

// Remove a peer from the peer list
void peers_remove(peers_t *peers, char* ip, int port) 
{

    pthread_mutex_lock(&peers->lock);

    for (int i = 0; i < peers->npeers_max; i++) 
    {
        if (peers->list[i] != NULL && strcmp(peers->list[i]->ip, ip) == 0 && peers->list[i]->port == port){
            free(peers->list[i]);
            peers->list[i] = NULL;
            peers->npeers_cur -= 1;
            break;
        }
    }
    pthread_mutex_unlock(&peers->lock);
}

peer_t* peers_find(peers_t *peers, const char *ip, uint16_t port)
{

    peer_t* peer_target = NULL;
    pthread_mutex_lock(&peers->lock);

    for (int i=0; i < peers->npeers_max; i++){
        if(peers->list[i] != NULL && peers->list[i]->port == port && (strcmp(peers->list[i]->ip, ip) == 0) ){
            
            peer_target = peers->list[i];
            break;
        }
    }

    pthread_mutex_unlock(&peers->lock);
    return peer_target;
}

void peer_outgoing_requests_destroy(peer_t* peer, request_q_t* reqs_q)
{
    pthread_mutex_lock(&reqs_q->lock);
    q_node_t* current = reqs_q->queue->head;
    q_node_t* prev = NULL;

    while (current != NULL)
    {
        request_t req = *(request_t*)current->data;

        if (strcmp(req.peer->ip, peer->ip) == 0 && req.peer->port == peer->port)
        {
            q_node_t* temp = current;

            current = current->next;

            if (prev != NULL)
            {
                prev->next = current;
            }
            else
            {
                reqs_q->queue->head = current;
            }

            if (current == NULL)
            {
                reqs_q->queue->tail = prev;
            }

            free(temp);
            reqs_q->count -= 1;
        }
        else
        {
            prev = current;
            current = current->next;
        }
    }

    pthread_mutex_unlock(&reqs_q->lock);
}

request_q_t* reqs_q_init() 
{
    request_q_t* req= (request_q_t*)mymalloc(sizeof(request_q_t));
    req->queue = q_init(req->queue); 

    pthread_mutex_init(&req->lock, NULL);
    pthread_cond_init(&req->lock, NULL);
    req->count = 0;
    req->ready = 1;
}

request_t* req_init(packet_t* pkt, peer_t* peer) 
{
    if (pkt == NULL || peer == NULL) return;

    request_t* req= (request_t*)mymalloc(sizeof(request_t));
    
    req->packet = pkt;
    req->peer = peer;
    req->status = WAITING;
    pthread_mutex_init(&req->lock);
    pthread_cond_init(&req->cond);
    return req;
}

void req_destroy(request_t* req)
{
    
}

void reqs_destroy(request_q_t* reqs_q)
{
    q_destroy(reqs_q->queue);
    pthread_mutex_destroy(&reqs_q->lock);
    pthread_cond_destroy(&reqs_q->cond);
    free(reqs_q);
    return;
}

request_t* reqs_q_nextup(request_q_t* reqs_q)
{
    if (reqs_q->queue->head != NULL);
    return (request_t*)reqs_q->queue->head->data;
    return;
}

void req_enqueue(request_q_t* reqs_q, request_t* request)
{
    if (reqs_q == NULL || request == NULL) return;
    q_enqueue(reqs_q->queue, (void*)&request);
    reqs_q->count += 1;
}

request_t* req_dequeue(request_q_t* reqs_q)
{
    if (reqs_q == NULL) return;
    void* data = q_dequeue(reqs_q->queue);
    request_t* req = ((request_t*)data);

    return req;
}

void peer_outgoing_requests_destroy(peer_t* peer, request_q_t* reqs_q)
{
    pthread_mutex_lock(&reqs_q->lock);
    q_node_t* current = reqs_q->queue->head;
    q_node_t* prev = NULL;

    while (current != NULL)
    {
        request_t req = *(request_t*)current->data;

        if (strcmp(req.peer->ip, peer->ip) == 0 && req.peer->port == peer->port)
        {
            q_node_t* temp = current;

            current = current->next;

            if (prev != NULL)
            {
                prev->next = current;
            }
            else
            {
                reqs_q->queue->head = current;
            }

            if (current == NULL)
            {
                reqs_q->queue->tail = prev;
            }

            free(temp);
            reqs_q->count -= 1;
        }
        else
        {
            prev = current;
            current = current->next;
        }
    }

    pthread_mutex_unlock(&reqs_q->lock);
}