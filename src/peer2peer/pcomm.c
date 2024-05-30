// Local Dependencies:
#include <pkgchk.h>
#include <btide.h>
#include <my_utils.h>
#include <config.h>
#include <peer.h>
#include <pcomm.h>
#include <packet.h>
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

void pcomm_connect(peer_t* peer, request_q_t* reqs_q, peers_t* peers)
{
    check_err(peer->sock_fd, "Socket creation error..."); 

    struct sockaddr_in peer_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(peer->port),
    };
    int err = inet_pton(AF_INET, peer->ip, &peer_addr.sin_addr);
    check_err(err, "Invalid address/ address not supported");

    err = connect(peer->sock_fd, (struct sockaddr*)&peer_addr, sizoef (peer_addr));
    check_err(err, "Failed to connect to new peer...");
    debug_print("New connection successful!");
    return;
}

void req_init(request_q_t* req)
{
    pthread_mutex_init(&req->lock, NULL);
    pthread_cond_init(&req->ready, NULL);

    req->queue = (queue_t*) malloc(sizeof(queue_t));
        if (req->queue == NULL) {
        perror("Failed to allocate memory for queue...");
        exit(EXIT_FAILURE);
    }

    q_init(req->queue);
    
    req->count = 0;
    req->ready = 1;
}

void req_destroy(request_q_t* req)
{
    q_destroy(req->queue);
    pthread_mutex_destoy(&req->lock);
    pthread_cond_destroy(&req->cond);
    free(req);
    return;
}

request_t req_nextup(request_q_t* reqs_q)
{
    if (reqs_q->queue->head != NULL);
    return *(request_t*)reqs_q->queue->head->data;

}

void req_enqueue(request_q_t* req, request_t request)
{
    q_enqueue(req->queue, (void*)&request);
    req->count += 1;
}

request_t* req_dequeue(request_q_t* req_q)
{
    void* data = q_dequeue(req_q->queue);
    request_t* req = ((packet_t*)data);

    return req;
}



void try_send(peer_t* peer, packet_t pkt_out){
    uint8_t buffer[PAYLOAD_MAX];
    memcpy(buffer, packet_marshall(pkt_out), sizeof(buffer));
    ssize_t nsent;

    nsent = send(peer->sock_fd, buffer, sizeof(buffer), 0); 
    check_err(nsent, "Failed to send entire message");
    debug_print("Successfully sent packet to peer!");
}

void send_acp(peer_t* peer)
{
    
    packet_t pkt = {
        .msg_code = PKT_MSG_ACP
    };

    try_send(peer, pkt);
}

void send_ack(peer_t* peer)
{
    packet_t pkt = {
        .msg_code = PKT_MSG_ACP
    };

    try_send(peer, pkt);
}

void send_res(peer_t* peer, packet_t pkt)
{
    try_send(peer, pkt);
}

void send_req(peer_t* peer, packet_t pkt)
{
    try_send(peer, pkt);
}

void send_png(peer_t* peer)
{
        packet_t pkt = {
        .msg_code = PKT_MSG_PNG
    };

    try_send(peer, pkt);
}

void send_pog(peer_t* peer)
{
        packet_t pkt = {
        .msg_code = PKT_MSG_POG
    };

    try_send(peer, pkt);
}

void send_dsn(peer_t* peer)
{
        packet_t pkt = {
        .msg_code = PKT_MSG_DSN
    };

    try_send(peer, pkt);
}

