#ifndef PCOMM_MANAGER_H
#define PCOMM_MANAGER_H

#include <my_utils.h>
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

typedef struct request_q{ 
    queue_t* queue;
    size_t count;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int ready;
}request_q_t;

typedef struct request{
    packet_t packet;
    peer_t peer;
}request_t;

typedef struct thread_args{
    peer_t* peer;
    request_q_t* requests;
    peers_t* peers;
}thread_args_t;


void req_init(request_q_t* requests);

void req_destroy(request_q_t* requests);

void req_enqueue(request_q_t* requests);

request_t req_nextup(request_q_t* requests);

request_t req_dequeue(request_q_t* requests);

void try_send(peer_t* peer, packet_t pkt);


void send_acp(peer_t* peer);


void send_ack(peer_t* peer);


void send_res(peer_t* peer, packet_t pkt);

void send_req(peer_t* peer, packet_t pkt);

void send_png(peer_t* peer);

void send_pog(peer_t* peer);

void send_dsn(peer_t* peer);



#endif