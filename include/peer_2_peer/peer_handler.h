#ifndef PEER_2_PEER_PEER_HANDLER_H
#define PEER_2_PEER_PEER_HANDLER_H

#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/packet.h>
#include <peer_2_peer/package.h>

typedef struct peer_thr_args {
    peer_t *peer;
    peers_t *peers;
    bpkg_t *bpkg;
    bpkgs_t *bpkgs;
    request_q_t *requests;
}peer_thr_args_t;
void cleanup_peer_handler(void *arg);

void peer_handler(void *args_void) ;

void peer_create_thread(peer_t* new_peer, request_q_t* reqs_q, peers_t* peers, bpkgs_t* bpkgs);

pkt_t* peer_try_receive(peer_t* peer) ;

void try_send(peer_t* peer, pkt_t* pkt_out);

void send_acp(peer_t* peer);

void send_ack(peer_t* peer);

void send_res(peer_t* peer, uint8_t err, pkt_t* pkt);

void send_req(peer_t* peer, pkt_t* pkt);

void send_png(peer_t* peer);

void send_pog(peer_t* peer);

void send_dsn(peer_t* peer);

int acp_wait_ack(peer_t *peer);

void process_pkt_in(peer_t* peer, pkt_t* pkt, bpkg_t* bpkgs, request_t* req_recent);

void process_pkt_out(peer_t* peer, pkt_t* pkt);

request_t* process_request_shared(peer_t* peer, request_q_t* reqs_q, peers_t* peers);

#endif