#include <chk/pkg_helper.h>
#include <chk/pkgchk.h>
#include <peer_2_peer/package.h>
#include <peer_2_peer/packet.h>
#include <peer_2_peer/peer_data_sync.h>
#include <pthread.h>
#include <stddef.h>
#include <sys/time.h>
#include <tree/merkletree.h>
#include <utilities/my_utils.h>

int pkt_fetch_from_peer(peer_t* peer, pkt_t* pkt, bpkg_t* bpkg,
     request_q_t* reqs) {
     bool pkt_found = false;

     if ( !pkt ) {
          debug_print("Failed to prepare Peer[%i]'s pkt...", peer->port);
          return -1;
     }

     request_t* req = req_create(pkt, peer);
     if ( !req ) {
          debug_print("Failed to create request...");
          return -1;
     }

     req->peer = peer;
     req->pkt = pkt;

     pthread_mutex_lock(&reqs->lock);
     reqs_enqueue(reqs, req);
     pthread_mutex_unlock(&reqs->lock);

     struct timespec ts;
     clock_gettime(CLOCK_REALTIME, &ts);
     ts.tv_sec += 3;

     pthread_mutex_lock(&req->lock);
     int rc = 0;
     while ( req->status == WAITING && rc != ETIMEDOUT ) {
          rc = pthread_cond_timedwait(&req->cond, &req->lock, &ts);
     }
     pthread_mutex_unlock(&req->lock);

     if ( req->status == SUCCESS ) {
          pkt_found = true;
     }

     req_destroy(req);  // Ensure proper cleanup

     return pkt_found ? 0 : -1;
}
// For this file, request this chknode's data, from this peer.
pkt_t* pkt_prepare_request_pkt(bpkg_t* bpkg, mtree_node_t* node) {
     if ( !bpkg || !node ) {
          return NULL;
     }

     chunk_t* chk = (chunk_t*)node->chunk;
     if ( !chk ) {
          return NULL;
     }

     payload_t* payload = payload_create(
          chk->offset, chk->size, node->expected_hash, bpkg->ident, NULL);
     if ( !payload ) {
          return NULL;
     }

     pkt_t* pkt = pkt_create(PKT_MSG_REQ, 0, payload);
     if ( !pkt ) {
          payload_destroy(payload);  // Ensure proper cleanup
     }

     return pkt;
}

payload_t* payload_get_res_for_req(payload_t* payload, bpkgs_t* bpkgs) {
     if ( !payload || !bpkgs ) {
          return NULL;
     }

     bpkg_t* bpkg = pkg_find_by_ident(bpkgs, payload->ident);
     if ( !bpkg ) {
          return NULL;
     }

     mtree_node_t* chk_node =
          bpkg_find_node_from_hash(bpkg->mtree, payload->hash, CHUNK);
     if ( !chk_node ) {
          return NULL;
     }

     if ( strncmp(chk_node->expected_hash, chk_node->computed_hash,
          SHA256_HEXLEN) != 0 ) {
          debug_print("Local copy of requested chunk is incomplete...\n");
          return NULL;
     }

     chunk_t* chk = chk_node->chunk;
     return payload_create(chk->offset, chk->size, chk_node->computed_hash,
          bpkg->ident, chk->data);
}

int pkg_download_payload(mtree_node_t* chk_node, payload_t* payload) {
     if ( pkt_chk_update_data(chk_node, payload) < 0 ) {
          debug_print("Data installed is invalid...\n");
          return -1;  // Return -1 to indicate failure
     }
     debug_print("Successfully downloaded data into local bpkg!\n");

     if ( !check_chunk(chk_node) ) {
          debug_print("Chunk is invalid though:(...\n");
          return -1;  // Return -1 to indicate invalid chunk
     }

     debug_print("Downloaded chunk is valid!\n");
     return 1;  // Return 1 to indicate success
}

int pkg_try_install_payload(bpkg_t* bpkg, payload_t* payload) {
     mtree_node_t* chk_node =
          bpkg_find_node_from_hash(bpkg->mtree, payload->hash, CHUNK);

     if ( !chk_node ||
          strncmp(chk_node->expected_hash, payload->hash, SHA256_HEXLEN) != 0 ) {
          debug_print("Payload hash is incorrect...\n");
          return -1;  // Return -1 to indicate hash mismatch
     }

     debug_print("Received chunk should be valid...\n");
     if ( pkg_download_payload(chk_node, payload) < 0 ) {
          return -1;  // Return -1 to indicate failure
     }
     return 1;  // Return 1 to indicate success
}

int pkt_install(pkt_t* pkt_in, peer_t* peer, bpkgs_t* bpkgs) {
     bpkg_t* bpkg = pkg_find_by_ident(bpkgs, pkt_in->payload->ident);

     if ( bpkg == NULL ) {
          debug_print(
               "Specified package for installation not found on local "
               "disc...\n");
          return -1;
     }

     if ( pkt_in->error < 0 ) {
          debug_print("Peer on Port[%d] does not have the requested chunk...\n",
               peer->port);
          return -1;
     }

     if ( pkg_try_install_payload(bpkg, pkt_in->payload) < 0 ) {
          return -1;
     }
     debug_print("Chunk installed successfully!\n");
     return 1;
}

int pkt_chk_update_data(mtree_node_t* chk_node, payload_t* payload) {
     if ( chk_node->chunk->size >= payload->size &&
          chk_node->chunk->offset == payload->offset ) {
          memcpy(chk_node->chunk->data, payload->data, payload->size);
          debug_print("Successfully updated chunk data!\n");
          return 0;  // Indicate success
     }
     else {
          debug_print(
               "Failed to update chunk data due to mismatching metadata...\n");
          return -1;  // Indicate failure
     }
}

bpkg_t* pkg_find_by_ident(bpkgs_t* bpkgs, char* ident_qry) {
     q_node_t* curr = bpkgs->ls->head;

     size_t ident_len = strlen(ident_qry);
     bpkg_t* res = NULL;

     while ( curr != NULL ) {
          bpkg_t* bpkg = (bpkg_t*)curr->data;
          if ( strncmp(ident_qry, bpkg->ident, ident_len) == 0 ) {
               debug_print("Matching ident found!\n");
               res = bpkg;
               break;
          }

          curr = curr->next;
     }
     return res;
}

int pkgs_add(bpkgs_t* bpkgs, bpkg_t* bpkg) {
     pthread_mutex_lock(&bpkgs->lock);
     q_enqueue(bpkgs->ls, bpkg);
     bpkgs->count++;              
     pthread_mutex_unlock(&bpkgs->lock);

     return 1;
}

int pkgs_rem(bpkgs_t* bpkgs, char* ident) {
     pthread_mutex_lock(&bpkgs->lock);
     q_node_t* current = bpkgs->ls->head;
     q_node_t* previous = NULL;

     int ident_len = strlen(ident);

     while ( current != NULL ) {
          bpkg_t* bpkg_curr = (bpkg_t*)current->data;

          if ( strlen(bpkg_curr->ident) >= ident_len &&
               strncmp(ident, bpkg_curr->ident, ident_len) == 0 ) {
               if ( previous != NULL ) {
                    previous->next = current->next;
               }
               else {
                    bpkgs->ls->head = current->next;
               }
               if ( current == bpkgs->ls->tail ) {
                    bpkgs->ls->tail = previous;
               }
               bpkg_obj_destroy(bpkg_curr);
               free(current);
               bpkgs->count--;  // Decrement the package count
               pthread_mutex_unlock(&bpkgs->lock);
               return 1;  // Indicate success
          }
          previous = current;
          current = current->next;
     }

     pthread_mutex_unlock(&bpkgs->lock);
     return -1;  // Indicate failure if no match found
}

bpkgs_t* pkgs_init(char* directory) {
     bpkgs_t* bpkgs = (bpkgs_t*)my_malloc(sizeof(bpkgs_t));

     if ( !bpkgs ) {
          perror("Failed to allocate memory for bpkgs...\n");
          return NULL;
     }

     if ( pthread_mutex_init(&bpkgs->lock, NULL) < 0 ) {
          perror("Failed to initialize mutex for shared package manager...\n");
          free(bpkgs);  // Free allocated memory on failure
          return NULL;
     }

     bpkgs->ls = q_init();
     if ( !bpkgs->ls ) {
          perror("Failed to initialize package list...\n");
          pthread_mutex_destroy(&bpkgs->lock);
          free(bpkgs);
          return NULL;
     }

     bpkgs->directory = directory;
     bpkgs->count = 0;
     return bpkgs;  // Return the initialized structure
}

void pkgs_destroy(bpkgs_t* bpkgs) {
     if ( !bpkgs ) return;

     pthread_mutex_lock(&bpkgs->lock);

     q_node_t* current = bpkgs->ls->head;
     while ( current != NULL ) {
          q_node_t* next = current->next;
          bpkg_t* bpkg_curr = (bpkg_t*)current->data;

          bpkg_obj_destroy(bpkg_curr);
          free(current);

          current = next;
     }

     q_destroy(bpkgs->ls);
     pthread_mutex_unlock(&bpkgs->lock);
     pthread_mutex_destroy(&bpkgs->lock);
     free(bpkgs);
}