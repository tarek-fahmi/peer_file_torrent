#ifndef PEER_2_PEER_PEER_SERVER_H
#define PEER_2_PEER_PEER_SERVER_H

#include <config.h>
#include <peer_2_peer/peer_data_sync.h>
#include <stdint.h>

typedef struct server_thr_args {
     struct sockaddr_in addr_obj;
     uint16_t server_port;
     int server_fd;
     request_q_t* reqs_q;
     peers_t* peers;
     bpkgs_t* bpkgs;
} server_thr_args_t;

void* server_thread_handler(void* args_void);

pthread_t create_p2p_server_thread(int server_fd, int server_port,
                                   request_q_t* reqs_q, peers_t* peers,
                                   bpkgs_t* bpkgs);

int p2p_setup_server(uint16_t port);

void p2p_server_listening(int server_fd, int server_port, request_q_t* reqs_q,
                          peers_t* peers, bpkgs_t* bpkgs);
#endif