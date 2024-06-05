// Local Dependencies:
#include <chk/pkgchk.h>
#include <utilities/my_utils.h>
#include <tree/merkletree.h>
#include <pthread.h>
#include <chk/pkgchk.h>
#include <chk/pkg_helper.h>
#include <stddef.h>
#include <peer_2_peer/packet.h>
#include <peer_2_peer/package.h>
#include <peer_2_peer/peer_data_sync.h>

bpkg_t* pkg_find_by_ident(bpkgs_t* bpkgs, char* ident_qry)
{
    // Find any and all packages that share the first n characters of the ident...

    q_node_t* curr = bpkgs->ls->head;

    size_t ident_len = strlen(ident_qry);
    bpkg_t* res = NULL;


    while (curr != NULL)
    {
        bpkg_t* bpkg = (bpkg_t*) curr->data;
        if (ident_len == curr->data && 
            strncmp(ident_qry, ((bpkg_t*)curr->data)->ident, ident_len) == 0)
        {
            debug_print("Matching ident found!");
            res = bpkg;
            break;
        }

        curr = curr->next;
    }
    return res;
}

int pkg_download_payload(mtree_node_t* chk_node, payload_t* payload)
{
    chunk_t* chk= (chunk_t*)chk_node->chunk;

    if (payload->size == chk->size){
        chunk_node_update_data(chk_node, payload->data);
        debug_print("Successfully downloaded data into local bpkg!");
    }


    if (bpkg_check_chunk(chk_node) < 0)
    {
        debug_print("Chunk is invalid though:(...");
        return -1;
    }

    debug_print("Downloaded chunk is valid!");
    return 1;
}

int pkg_try_install_payload(bpkg_t* bpkg, payload_t* payload)
{
    mtree_node_t* chk_node = bpkg_get_node_from_hash(bpkg->mtree, payload->hash);

    if (strncmp(chk_node->expected_hash, payload->hash, SHA256_HEXLEN) == 0)
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


int pkt_install(pkt_t* pkt_in, peer_t* peer, bpkgs_t* bpkgs)
{
    bpkg_t* bpkg = pkg_find_by_ident(bpkgs, pkt_in->payload->ident);
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
pkt_t* pgk_prepare_request_pkt(bpkg_t* bpkg, mtree_node_t* node)
{
    size_t ident_len = strlen(bpkg->ident);
    chunk_t* chk = (chunk_t*)chk;

    payload_t* payload = payload_create(chk->offset, chk->size, 
                                        node->expected_hash, bpkg->ident, NULL);
    pkt_t* pkt = pkt_create(PKT_MSG_REQ, 0, payload);

    return pkt;
}

int pkt_search_and_acquire(bpkg_t* bpkg, peers_t* peers, request_q_t* reqs, pkt_t* pkt_tofind)
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

        
        pkt_t* pkt = pgk_prepare_request_pkt(bpkg, pkt_tofind);
        
        
        if (pkt == NULL)
        {
            debug_print("Failed to prepare Peer[%d]'s pkt...");
            continue;
        }

        req->peer = peer_curr;
        req->pkt = pkt;

        pthread_mutex_lock(&reqs->lock);
        reqs_enqueue(req);
        pthread_mutex_unlock(&reqs->lock);

        struct timespec ts;
        clock_gettime(0, &ts);
        ts.tv_sec += 3; 

        pthread_mutex_lock(&req->lock);
        int rc = 0;
        while (req->status == WAITING && rc != ETIMEDOUT) {
            rc = pthread_cond_timedwait(&req->cond, &req->lock, &ts);
        }
        pthread_mutex_unlock(&req->lock);

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

int pkgs_add(bpkgs_t* bpkgs, bpkgs_t* bpkg)
{
    pthread_mutex_lock(&bpkgs->lock);
    p_enqueue(bpkgs->ls, bpkgs);
    pthread_mutex_unlock(&bpkgs->lock);

    return 1;
}

int pkgs_rem(bpkgs_t* bpkgs, char* ident)
{
    pthread_mutex_lock(&bpkgs->lock);
    q_node_t* current = bpkgs->ls->head;
    q_node_t* tmp = current;

    int ident_len = strlen(ident);
    q_node_t* previous;
    bpkg_t* bpkg_curr;

    while (current != NULL)
    {
        bpkg_curr = (bpkg_t*) current->data;
        tmp = current;

        if (strlen(bpkg_curr->ident) >= ident_len && strncmp(ident, bpkg_curr->ident, ident_len) == 0)
        {
            if (previous != NULL)
            {
                previous->next = current->next;
                tmp = current->next;
                bpkg_destroy(bpkg_curr);
                q_node_destroy(current);
            }
        }
        previous = current;
        current = tmp;
        
    }

    pthread_mutex_unlock(&bpkgs->lock);
}

bpkgs_t* pkgs_init(char* directory)
{
    bpkgs_t* bpkgs = (bpkgs_t*) my_malloc(sizeof(bpkgs_t));
    
    if (pthread_mutex_init(bpkgs->lock, NULL) < 0)
    {
        perror("Failed to initialize mutex for shared package manager...\n");
        return NULL;
    }

    bpkgs->ls = q_init();
    bpkgs->directory = directory;
    bpkgs->count = 0;
}

payload_t* payload_get_res_for_req(payload_t* payload, bpkgs_t* bpkgs)
{
    bpkg_t* bpkg = pkg_find_by_ident(bpkgs, payload->ident);

    if (bpkg == NULL); return NULL;

    mtree_node_t* chk_node = bpkg_get_node_from_hash(bpkg->mtree, payload->hash);
    if (chk_node == NULL); return NULL;

    
    if (bpkg_validate_node_completion(chk_node) < 0)
    {
        debug_print("Local copy of requested chunk is incomplete...\n");
        return NULL;
    }

    chunk_t* chk = chk_node->chunk;
    return payload_create(chk->offset, chk->size, chk_node->computed_hash, bpkg->ident, chk->data);
}