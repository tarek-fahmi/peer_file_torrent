#ifndef PEER_HANDLER_H
#define PEER_HANDLER_H

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

void cleanup_peer_handler(void *arg);

void peer_handler(void *args_void) ;

void peer_create_thread(peer_t* new_peer, request_q_t* reqs_q, peers_t* peers, bpkgs_t* bpkgs);

packet_t* try_receive(peer_t* peer) ;

void try_send(peer_t* peer, packet_t* pkt_out);

void send_acp(peer_t* peer);

void send_ack(peer_t* peer);

void send_res(peer_t* peer, packet_t* pkt);

void send_req(peer_t* peer, packet_t* pkt);

void send_png(peer_t* peer);

void send_pog(peer_t* peer);

void send_dsn(peer_t* peer);

int acp_wait_ack(peer_t *peer);

void process_packet_in(peer_t* peer, packet_t* pkt, bpkg_t* bpkgs, request_t* req_recent);

void process_packet_out(peer_t* peer, packet_t* pkt);

request_t* process_request_shared(peer_t* peer, request_q_t* reqs_q, peers_t* peers);

#endif