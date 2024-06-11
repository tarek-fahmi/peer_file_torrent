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


/* Packet fetching and handling for peer communication and package management */

/**
 * @brief Fetch a packet from a specified peer.
 *
 * @param peer Pointer to the peer
 * @param pkt Packet to fetch
 * @param bpkg Associated package
 * @return int 0 on success, -1 on failure
 */
int pkt_fetch_from_peer(peer_t* peer, pkt_t* pkt, bpkg_t* bpkg) {
     bool pkt_found = false;
     request_q_t* reqs = peer->reqs_q;

     if ( !pkt ) {
          debug_print("Failed to prepare Peer[%i]'s pkt...", peer->port);
          return -1;
     }

     request_t* req = req_create(pkt);
     if ( !req ) {
          debug_print("Failed to create request...");
          return -1;
     }

     debug_print("Enqueuing request for Peer[%i]...", peer->port);
     pthread_mutex_lock(&reqs->lock);
     reqs_enqueue(reqs, req);
     pthread_mutex_unlock(&reqs->lock);

     // Wait 3s for the peer handler recieve ACP packet:
     struct timespec ts;
     clock_gettime(CLOCK_REALTIME, &ts);
     ts.tv_sec += 3;

     pthread_mutex_lock(&req->lock);
     int rc = 0;
     while ( req->status == WAITING && rc != ETIMEDOUT ) {
          rc = pthread_cond_timedwait(&req->cond, &req->lock, &ts);
     }
     pthread_mutex_unlock(&req->lock);

     // Check connection success status:
     if ( req->status == SUCCESS ) {
          pkt_found = true;
          debug_print("Request for Peer[%i] completed successfully.", peer->port);
     }
     else {
          debug_print("Request for Peer[%i] timed out or failed.", peer->port);
     }

     req_destroy(req);

     return pkt_found ? 0 : -1;
}


/**
 * @brief Prepare a request packet for a chunk.
 *
 * @param bpkg Pointer to the package
 * @param node Node in the Merkle tree
 * @return pkt_t* Pointer to the prepared packet
 */
pkt_t* pkt_prepare_request_pkt(bpkg_t* bpkg, mtree_node_t* node) {
     if ( !bpkg || !node ) {
          return NULL;
     }


     chunk_t* chk = (chunk_t*)node->chunk;
     if ( !chk ) {
          return NULL;
     }

     // Prepare a request for package chunk:
     payload_t payload = payload_create_req(chk->offset, chk->size, node->expected_hash, bpkg->ident, NULL);

     pkt_t* pkt = pkt_create(PKT_MSG_REQ, 0, payload);
     return pkt;
}

/**
 * @brief Download payload data to a chunk node.
 *
 * @param chk_node Chunk node to update
 * @param payload Payload data
 * @return int 0 on success, -1 on failure
 */

 /**
  * @brief Attempt to install payload data into a package.
  *
  * @param bpkg Target package
  * @param payload Payload data
  * @return int 1 on success, -1 on failure
  */
int pkg_try_install_payload(bpkg_t* bpkg, payload_t payload) {
     mtree_node_t* chk_node = bpkg_find_node_from_hash(bpkg->mtree, payload.res.hash, CHUNK);

     if ( chk_node ) debug_print("Received chunk should be valid...\n");

     if ( pkt_chk_update_data(bpkg->mtree, chk_node, payload) < 0 ) {
          return -1;
     }
     return 1;
}

/**
 * @brief Install an incoming packet's payload.
 *
 * @param pkt_in Incoming packet
 * @param peer Source peer
 * @param bpkgs Package manager
 * @return int 1 on success, -1 on failure
 */
int pkt_install(pkt_t* pkt_in, peer_t* peer, bpkgs_t* bpkgs) {
     bpkg_t* bpkg = pkg_find_by_ident(bpkgs, pkt_in->payload.res.ident);

     if ( bpkg == NULL ) {
          debug_print("Specified package for installation not found on local disc...\n");
          return -1;
     }

     if ( pkt_in->error < 0 ) {
          debug_print("Peer on Port[%d] does not have the requested chunk...\n", peer->port);
          return -1;
     }

     if ( pkg_try_install_payload(bpkg, pkt_in->payload) < 0 ) {
          return -1;
     }
     debug_print("Chunk installed successfully!\n");
     return 1;
}

/**
 * @brief Update chunk node data with payload content.
 *
 * @param chk_node Target chunk node
 * @param payload Payload data
 * @return int 0 on success, -1 on failure
 */
int pkt_chk_update_data(mtree_t* mtree, mtree_node_t* chk_node, payload_t payload) {
     if ( update_chunk_node(mtree, chk_node, payload.res.data, payload.res.size, payload.res.offset) ) {
          ;

          debug_print("Successfully updated chunk data!\n");
          return 0;
     }
     else {
          debug_print("Failed to update chunk data due to mismatching metadata...\n");
          return -1;
     }
}

/* Functions for managing packages within shared thread resources */

/**
 * @brief Find a package by its identifier.
 *
 * @param bpkgs Package manager
 * @param ident_qry Identifier to search for
 * @return bpkg_t* Pointer to the found package, or NULL if not found
 */
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

/**
 * @brief Add a package to the package manager. Mutexed to ensure thread safety
 *
 * @param bpkgs Package manager
 * @param bpkg Package to add
 * @return int 1 on success, -1 on failure
 */
int pkgs_add(bpkgs_t* bpkgs, bpkg_t* bpkg) {
     pthread_mutex_lock(&bpkgs->lock);
     q_enqueue(bpkgs->ls, bpkg);
     bpkgs->count++;
     pthread_mutex_unlock(&bpkgs->lock);
     return 1;
}

/**
 * @brief Remove a package by its identifier.
 *
 * @param bpkgs Package manager
 * @param ident Identifier of the package to remove
 * @return int 1 on success, -1 on failure
 */
int pkgs_rem(bpkgs_t* bpkgs, char* ident) {
     pthread_mutex_lock(&bpkgs->lock);
     q_node_t* current = bpkgs->ls->head;
     q_node_t* previous = NULL;

     int ident_len = strlen(ident);

     while ( current != NULL ) {
          bpkg_t* bpkg_curr = (bpkg_t*)current->data;

          if ( strlen(bpkg_curr->ident) >= ident_len && strncmp(ident, bpkg_curr->ident, ident_len) == 0 ) {
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

/**
 * @brief Initialize the package manager.
 *
 * @param directory Directory for package storage
 * @return bpkgs_t* Pointer to the initialized package manager
 */
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

/**
 * @brief Destroy the package manager and free resources.
 *
 * @param bpkgs Package manager to destroy
 */
void pkgs_destroy(bpkgs_t* bpkgs) {
     if ( !bpkgs ) return;

     pthread_mutex_lock(&bpkgs->lock);

     q_node_t* current = bpkgs->ls->head;
     while ( current != NULL ) {
          q_node_t* next = current->next;
          bpkg_t* bpkg_curr = (bpkg_t*)current->data;

          bpkg_obj_destroy(bpkg_curr);
          bpkg_curr = NULL;

          current = next;
     }
     q_destroy(bpkgs->ls);
     pthread_mutex_unlock(&bpkgs->lock);
     pthread_mutex_destroy(&bpkgs->lock);
     free(bpkgs);
}
