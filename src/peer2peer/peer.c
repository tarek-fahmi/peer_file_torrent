// Local Dependencies:
#include <pkgchk.h>
#include <btide.h>
#include <my_utils.h>
#include <config.h>
#include <peer.h>
// Standard Linux Dependencies:
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// Additional Linux Dependencies:
#include <string.h>
#include <pthread.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>



/**
 * @brief Initialize the peer list, setting every element in the peer list to NULL.
 * @params: peer list and max number of peers for the program to reference.
*/
void peer_list_init(peers_t *peers, size_t max_peers) {
    peers->npeers_cur = 0;
    pthread_mutex_init(&peers->lock, NULL);
    peers->list = (peers_t**) malloc(sizeof(peer_t*) * max_peers);

    // Initialize all peers to NULL
    for(int i=0; i <max_peers; i++){
        peers->list[i] = NULL;
    }
}

void peer_init(peer_t* peer, char* ip, int port)
{
        peer->port = port;
        memcpy(peer->ip, ip, sizeof(peer->ip));
        pthread_t thr;
        peer->thread = thr;
}

/**
 * @brief Add a peer to the list of peers, initializing its attributes based on the arguments provided.
 *         This will be called in the connect function...
*/
void peers_add(peers_t *peers, peer_t* new_peer) {
    pthread_mutex_lock(&peers->lock);

    if(peers->npeers_cur >= peers->npeers_max - 1){
        debug_print("Cannot add peer: max peers connected...");
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
void peers_remove(peers_t *peers, const char *ip, uint16_t port) {

    pthread_mutex_lock(&peers->lock);

    for (int i = 0; i < peers->npeers_max; i++) 
    {
        if (strcmp(peers->list[i]->ip, ip) == 0 && peers->list[i]->port == port){
            peers->list[i] = NULL;
            break;
        }
    }
    peers->npeers_cur -= 1;
    pthread_mutex_unlock(&peers->lock);
}

peer_t* peers_find(peers_t *peers, const char *ip, uint16_t port){

    peer_t* peer_target = NULL;
    pthread_mutex_lock(&peers->lock);

    for (int i=0; i < peers->npeers_max; i++){
        if(peers->list[i]->port == port && (strcmp(peers->list[i]->ip, ip) == 0) ){
            
            peer_target = peers->list[i];
            break;
        }
    }

    pthread_mutex_unlock(&peers->lock);
    return peer_target;
}
