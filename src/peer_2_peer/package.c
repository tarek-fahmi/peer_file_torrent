// Local Dependencies:
#include <pkgchk.h>
#include <btide.h>
#include <my_utils.h>
#include <pkg_helper.h>
#include <config.h>
#include <peer.h>
#include <pkg_helper.h>
#include <pkgchk.h>
#include <p2p.h>
#include <pcomm.h>
#include <peer2peer.h>
#include <packet.h>
// Standard Linux Dependencies:
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
// Additional Linux Dependencies:
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

typedef struct{
    queue_t* ls;
    uint8_t count;
    pthread_mutex_t* lock;
}bpkgs_t;



bpkg_t* pkg_find_by_ident(bpkgs_t* bpkgs, char* ident_qry)
{
    // Find any and all packages that share the first n characters of the ident...

    q_node_t* curr = bpkgs->ls->head;

    size_t ident_len = strlen(ident_qry);
    bpkg_t* res = NULL;


    while (curr != NULL)
    {
        bpkg_t* bpkg = curr->data;
        if (strncmp(ident_qry, ((bpkg_t*)curr->data)->ident, ident_len) == 0)
        {
            debug_print("Matching ident found!");
            res = bpkg;
            break;
        }

        curr = curr->next;
    }
    return res;
}

int pkg_download_payload(mtree_node_t* chk_node, payload_t payload)
{
    chunk_t* chk= (chunk_t*)chk_node->value;

    if (payload.size == chk->size){
        memcpy(chk->data, payload.data, chk->size);
        debug_print("Successfully downloaded data into local bpkg!");
    }

    sha256_compute_chunk_hash(chk_node);

    if (bpkg_check_chunk(chk_node) < 0)
    {
        debug_print("Chunk is invalid though:(...");
        return -1;
    }

    debug_print("Downloaded chunk is valid!");
    return 1;
}

int pkg_try_install_payload(bpkg_t* bpkg, payload_t payload)
{
    mtree_node_t* chk_node = bpkg_get_node_from_hash(bpkg->mtree, payload.hash);

    if (strncmp(chk_node->expected_hash, payload.hash, SHA256_HEXLEN) == 0)
    {
        debug_print("Recieved chunk should be valid...");
        if (pkg_download_payload(chk_node, payload) < 0)
        {
            return -1;
        }
        return 1;
    }
    debug_print("Payload hash is incorrect...");
}


int pkt_install(packet_t* pkt_in, peer_t* peer, bpkgs_t* bpkgs)
{
    bpkg_t* bpkg = pkg_find_by_ident(bpkgs, pkt_in->payload.ident);
    if (bpkg == NULL){
        debug_print("Specified package for installation not found on local disc...");
        return -1;
    }

    if (pkt_in->error < 0)
    {
        debug_print("Peer on Port[%d] does not have the requested chunk...");
        return -1;
    }

    if (pkt_try_install_payload(bpkg, pkt_in->payload) < 0)
    {
        return -1;
    }
    debug_print("Chunk installed succesfully!");
    return 1;
}

// For this file, request this chknode's data, from this peer.
packet_t* pgk_prepare_request_packet(bpkg_t* bpkg, mtree_node_t* node)
{
    packet_t* pkt_out = (packet_t*) my_mallc(sizeof(packet_t*));
    size_t ident_len = strlen(bpkg->ident);
    chunk_t* chk = (chunk_t*)chk;

    payload_t pl = pkt_out->payload;
    pkt_out->msg_code = PKT_MSG_REQ;
    pkt_out->payload.size = chk->size;
    pkt_out->payload.offset = chk->offset;

    strncpy(pl.ident, bpkg->ident, ident_len);
    strncpy(pl.hash, node->expected_hash, SHA256_HEXLEN);

    return pkt_out;
}

int pkt_search_and_acquire(bpkg_t* bpkg, peers_t* peers, request_q_t* reqs, packet_t* pkt_tofind)
{
    bool pkt_found = 0;
    peer_t* tmp;
    request_t* req = req_init(pkt_tofind, &tmp);

    for(int i=0; i < peers->npeers_max; i++)
    {
        peer_t* peer_curr = peers->list[i];

        if (peer_curr == NULL && peer_curr->port <= 0)
        {
            continue;
        }

        
        packet_t* pkt = pgk_prepare_request_packet(bpkg, pkt_tofind);
        
        
        if (pkt == NULL)
        {
            debug_print("Failed to prepare Peer[%d]'s packet...");
            continue;
        }

        req->peer = peer_curr;
        req->packet = pkt;

        pthread_mutex_lock(&reqs->lock);
        reqs_enqueue(req);
        pthread_mutex_unlock(&reqs->lock);

        struct timespec ts;
        clock_gettime(0, &ts);
        ts.tv_sec += 3; 

        pthread_mutex_lock(&req->mutex);
        int rc = 0;
        while (req->status == WAITING && rc != ETIMEDOUT) {
            rc = pthread_cond_timedwait(&req->cond, &req->mutex, &ts);
        }
        pthread_mutex_unlock(&req->mutex);

        if (req->status == SUCCESS) {
            pkt_found = true;
            break;
        }

    if (!pkt_found) {
        free(req);
        return -1;
    }

    free(req);
    return 0;
    
    }
}
