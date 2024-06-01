#ifndef PEER_2_PEER_H
#define PEER_2_PEER_H

// Local Dependencies:=
#include <crypt/sha256.h>
#include <tree/merkletree.h>
#include <chk/pkgchk.h>
#include <chk/pkg_helper.h>
#include <utilities/my_utils.h>
#include <peer_2_peer/peer_handler.h>
#include <peer_2_peer/peer_server.h>
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

typedef struct{
    queue_t* ls;
    uint8_t count;
    pthread_mutex_t* lock;
}bpkgs_t;


bpkg_t* pkg_find_by_ident(bpkgs_t* bpkgs, char* ident_qry);

int pkg_download_payload(mtree_node_t* chk_node, payload_t payload);

int pkg_try_install_payload(bpkg_t* bpkg, payload_t payload);

int pkt_install(packet_t* pkt_in, peer_t* peer, bpkgs_t* bpkgs);

// For this file, request this chknode's data, from this peer.
packet_t* pgk_prepare_request_packet(bpkg_t* bpkg, mtree_node_t* node);

int pkt_search_and_acquire(bpkg_t* bpkg, peers_t* peers, request_q_t* reqs, packet_t* pkt_tofind);
#endif