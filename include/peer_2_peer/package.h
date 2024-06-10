#ifndef PEER_2_PEER_PACKAGE_H
#define PEER_2_PEER_PACKAGE_H

#include <chk/pkgchk.h>
#include <peer_2_peer/packet.h>
#include <peer_2_peer/peer_data_sync.h>
#include <tree/merkletree.h>

int pkt_fetch_from_peer(peer_t* peer, pkt_t* pkt, bpkg_t* bpkg,
                        request_q_t* reqs);

// For this file, request this chknode's data, from this peer.
pkt_t* pkt_prepare_request_pkt(bpkg_t* bpkg, mtree_node_t* node);

payload_t* payload_get_res_for_req(payload_t* payload, bpkgs_t* bpkgs);

int pkg_download_payload(mtree_node_t* chk_node, payload_t* payload);

int pkg_try_install_payload(bpkg_t* bpkg, payload_t* payload);

int pkt_install(pkt_t* pkt_in, peer_t* peer, bpkgs_t* bpkgs);

int pkt_chk_update_data(mtree_node_t* chk_node, payload_t* payload);

bpkg_t* pkg_find_by_ident(bpkgs_t* bpkgs, char* ident_qry);

int pkgs_add(bpkgs_t* bpkgs, bpkg_t* bpkg);

int pkgs_rem(bpkgs_t* bpkgs, char* ident);

bpkgs_t* pkgs_init(char* directory);

void pkgs_destroy(bpkgs_t* bpkgs);

#endif