#ifndef PEER_2_PEER_PACKAGE_H
#define PEER_2_PEER_PACKAGE_H

#include <peer_2_peer/packet.h>
#include <chk/pkgchk.h>
#include <tree/merkletree.h>

bpkg_t* pkg_find_by_ident(bpkgs_t* bpkgs, char* ident_qry);

int pkg_download_payload(mtree_node_t* chk_node, payload_t* payload);

int pkg_try_install_payload(bpkg_t* bpkg, payload_t* payload);

int pkt_install(pkt_t* pkt_in, peer_t* peer, bpkgs_t* bpkgs);

// For this file, request this chknode's data, from this peer.
pkt_t* pgk_prepare_request_pkt(bpkg_t* bpkg, mtree_node_t* node);

int pkt_search_and_acquire(bpkg_t* bpkg, peers_t* peers, request_q_t* reqs, pkt_t* pkt_tofind);

payload_t* payload_get_res_for_req(payload_t* payload, bpkgs_t* bpkgs);


#endif