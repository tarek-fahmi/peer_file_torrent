#include <peer_2_peer/package.h>
#include <peer_2_peer/packet.h>
#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/peer_handler.h>
#include <sys/time.h>

/**
 * @brief Main function for the peer handler thread.
 * @param args_void Pointer to the arguments for the peer handler thread.
 */
void peer_handler(void* args_void) {
     pthread_cleanup_push(peer_cleanup_handler, args_void);

     peer_thr_args_t* args = (peer_thr_args_t*)args_void;
     peer_t* peer = args->peer;
     peers_t* peers = args->peers;
     bpkgs_t* bpkgs = args->bpkgs;


     acp_wait_ack(peer);
     // Continuously check whether a request has been enqueued or a peer has sent a packet:

     while ( true ) {

          // Request queue check:
          request_t* req = peer_process_request_shared(peer);

          // Incoming packet chacket:
          pkt_t* pkt = peer_try_receive(peer);

          if ( pkt != NULL ) {
               debug_print("Received packet from peer. Processing now...\n");
               process_pkt_in(peer, pkt, bpkgs, req, peers);
          }
          else {
               debug_print("Could not process packet from peer.\n");
          }

          pthread_testcancel();
     }
     pthread_cleanup_pop(1);
}

/**
 * @brief Attempts to receive a packet from a peer.
 * @param peer Pointer to the peer.
 * @return Pointer to the received packet, or NULL if receiving failed.
 */
pkt_t* peer_try_receive(peer_t* peer) {
     if ( !peer || peer->sock_fd < 0 ) {
          debug_print("Invalid peer or socket not initialized.\n");
          return NULL;
     }

     struct timeval tv = { 3, 0 };  // Timeout set to 3 seconds.
     if ( setsockopt(peer->sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0 ) {
          perror("Failed to set socket options\n");
          return NULL;
     }

     uint8_t buffer[sizeof(pkt_t)];
     ssize_t received = 0;
     ssize_t n;
     debug_print("Starting to receive data...\n");

     // Continue to read until the entire packet has been received
     while ( received < sizeof(pkt_t) ) {
          n = recv(peer->sock_fd, buffer + received, sizeof(pkt_t) - received, 0);
          if ( n < 0 ) {
               debug_print("Receive failed or timed out\n");
               return NULL;
          }
          if ( n == 0 ) {
               debug_print("Connection closed by peer.\n");
               return NULL;
          }
          received += n;
          debug_print("Received %ld bytes, total %ld bytes\n", n, received);
     }

     debug_print("Data received successfully. Unmarshalling packet...\n");

     pkt_t* pkt = malloc(sizeof(pkt_t));
     if ( !pkt ) {
          perror("Failed to allocate packet\n");
          return NULL;
     }

     pkt_unmarshall(pkt, buffer);

     debug_print("Packet unmarshalled successfully. Msg code: %d\n", pkt->msg_code);
     return pkt;
}

/**
 * @brief Processes an incoming packet from a peer.
 * @param peer Pointer to the peer.
 * @param pkt_in Pointer to the incoming packet.
 * @param bpkgs Pointer to the package manager.
 * @param req_recent Pointer to the recent request.
 */
void process_pkt_in(peer_t* peer, pkt_t* pkt_in, bpkgs_t* bpkgs, request_t* req_recent, peers_t* peers) {
     if ( !peer || !pkt_in || !bpkgs ) {
          debug_print("Invalid input to process_pkt_in: NULL peer, packet, or package manager.\n");
          return;
     }

     debug_print("Processing incoming packet with message code: %d\n", pkt_in->msg_code);
     switch ( pkt_in->msg_code ) {
     case PKT_MSG_PNG: //Pong request:
          send_pog(peer);
          debug_print("Sent POG in response to PNG from peer at port %d.\n", peer->port);
          break;

     case PKT_MSG_ACP: //Accept connection:
          send_ack(peer);
          debug_print("Sent ACK in response to ACP from peer at port %d.\n", peer->port);
          break;

     case PKT_MSG_REQ: //Request packet:
          send_res_pkts(peer, pkt_in, bpkgs);
          debug_print("Response sent to REQ from peer at port %d with error status.\n", peer->port);
          break;

     case PKT_MSG_DSN: //Peer disconnecting:
          printf("Disconnected from peer\n");
          fflush(stdout);
          peers_remove(peers, peer->ip, peer->port);
          pkt_destroy(pkt_in);
          peer_destroy(peer);
          break;

     case PKT_MSG_RES: //Response packet:
          if ( pkt_install(pkt_in, peer, bpkgs) < 0 ) {
               req_recent->status = FAILED;
               debug_print("Failed to install packet from peer at port %d.\n", peer->port);
          }
          else {
               req_recent->status = SUCCESS;
               pthread_cond_signal(&req_recent->cond);
               debug_print("Successfully received and installed packet from peer at port %d.\n", peer->port);
          }
          break;

     default:
          debug_print("Received unrecognized packet type from peer at port %d.\n", peer->port);
          break;
     }
     pkt_destroy(pkt_in);
}

/**
 * @brief Processes an outgoing packet for a peer.
 * @param peer Pointer to the peer.
 * @param pkt Pointer to the packet to send.
 */
void process_pkt_out(peer_t* peer, request_t* req) {
     pkt_t* pkt = req->pkt;
     switch ( pkt->msg_code ) {
     case PKT_MSG_PNG:
          send_png(peer);
          break;
     case PKT_MSG_REQ:
          send_req(peer, pkt);
          break;
     case PKT_MSG_DSN:
          send_dsn(peer);
          req_destroy(req);
          peer_destroy(peer);
          peer = NULL;
          break;
     default:
          break;
     }
     req_destroy(req);
}

/**
 * @brief Processes a shared request for a peer.
 * @param peer Pointer to the peer.
 * @return Pointer to the processed request.
 */
request_t* peer_process_request_shared(peer_t* peer) {
     if ( !peer || !peer->reqs_q ) {
          debug_print("Invalid arguments to peer_process_request_shared.\n");
          return NULL;
     }

     // Try to dequeue and then process the request.

     request_t* req = reqs_dequeue(peer->reqs_q);


     if ( req != NULL ) {
          debug_print("Processing request for peer at port %d and IP %s.\n", peer->port, peer->ip);

     }

     if ( req != NULL ) {
          process_pkt_out(peer, req);
          debug_print("Request processed successfully for peer at port %d and IP %s.\n", peer->port, peer->ip);
     }
     else {
          debug_print("No request found for peer at port %d and IP %s.\n", peer->port, peer->ip);
     }



     return req;
}

/**
 * @brief Cleans up resources used by the peer handler thread.
 * @param arg Pointer to the arguments for the peer handler thread.
 */
void peer_cleanup_handler(void* arg) {
     peer_thr_args_t* args = (peer_thr_args_t*)arg;

     if ( args == NULL ) {
          debug_print("Cleanup handler received a NULL argument.\n");
          return;
     }
     peer_t* peer = args->peer;
     debug_print("Starting cleanup for peer at port %d and IP %s.\n", peer->port, peer->ip);

     if ( peer == NULL ) {
          debug_print("No peer found to clean up.\n");
          free(args);
          return;
     }
     else if ( peer->sock_fd >= 0 ) {
          close(peer->sock_fd);
          peer->sock_fd = -1;
     }

     if ( pthread_equal(pthread_self(), peer->thread) == 0 ) {
          pthread_cancel(peer->thread);
          pthread_join(peer->thread, (void**)0);
     }

     free(args);
     free(peer);
     peer = NULL;
}
/**
 * @brief Creates a new peer handler thread.
 * @param new_peer Pointer to the peer.
 * @param peers Pointer to the list of peers.
 * @param bpkgs Pointer to the package manager.
 */
void peer_create_thread(peer_t* new_peer, peers_t* peers, bpkgs_t* bpkgs) {
     peer_thr_args_t* args = malloc(sizeof(peer_thr_args_t));
     if ( !args ) {
          perror("Failed to allocate memory for peer_thr_args_t");
          exit(EXIT_FAILURE);
     }

     args->peer = new_peer;
     args->peers = peers;
     args->bpkgs = bpkgs;

     int result = pthread_create(&new_peer->thread, NULL, (void*)&peer_handler, args);
     if ( result != 0 ) {
          perror("Thread creation failed...");
          free(args);
          exit(EXIT_FAILURE);
     }
}
/**
 * @brief Sends a packet to a peer.
 * @param peer Pointer to the peer.
 * @param pkt_out Pointer to the packet to send.
 */
void try_send(peer_t* peer, pkt_t* pkt_out) {
     if ( !peer || peer->sock_fd < 0 ) {
          debug_print("Failed to send packet: Invalid peer or socket.\n");
          return;
     }

     uint8_t buffer[4096];
     pkt_marshall(pkt_out, buffer);

     int total = 0;
     int bytesleft = sizeof(pkt_t);
     int n;

     // Continuously send packet data until the entire packet goes through:
     debug_print("Attempting to send packet to peer at port %d.\n", peer->port);
     while ( total < PAYLOAD_MAX ) {
          n = send(peer->sock_fd, buffer + total, bytesleft, 0);
          if ( n == -1 ) {
               perror("Failed to send packet");
               debug_print("Failed to send packet to peer at port %d after %d bytes.\n", peer->port, total);
               return;
          }
          total += n;
          bytesleft -= n;
     }

     // If a full packet was recieved:
     if ( total == PAYLOAD_MAX ) {
          debug_print("Successfully sent entire packet to peer at port %d.\n", peer->port);
     }
     else {
          debug_print("Sent incomplete packet to peer at port %d; sent %d of %d bytes.\n", peer->port, total, bytesleft);
     }
}

void send_res_pkts(peer_t* peer, pkt_t* pkt_in, bpkgs_t* bpkgs)
{
     bpkg_t* bpkg = pkg_find_by_ident(bpkgs, pkt_in->payload.req.ident);
     uint16_t err = 0;

     if ( !bpkg ) {
          err = -1;
          send_res(peer, err, ( payload_t ) { 0 });
          return;
     }

     mtree_node_t* chk_node = NULL;

     // Search for chunk containing packet requested from user:
     if ( pkt_in->payload.req.offset > 0 ) {
          chk_node = bpkg_find_node_from_hash_offset(bpkg->mtree->root, pkt_in->payload.req.hash, pkt_in->payload.req.offset);
     }
     else {
          chk_node = bpkg_find_node_from_hash(bpkg->mtree, pkt_in->payload.req.hash, CHUNK);
     }

     if ( !chk_node || strncmp(chk_node->expected_hash, chk_node->computed_hash, SHA256_HEXLEN) != 0 ) {
          debug_print("Local copy of requested chunk is incomplete or not found...\n");
          err = -1;
          send_res(peer, err, ( payload_t ) { 0 });
          return;
     }


     // Continuously send packets of max size DATA_MAX until entire chunk has been sent.
     chunk_t* chk = chk_node->chunk;
     uint32_t curr_offset = chk->offset;
     uint32_t remaining_size = chk->size;
     uint8_t data[DATA_MAX];

     while ( remaining_size > 0 ) {
          uint32_t chunk_size = ( remaining_size > DATA_MAX ) ? DATA_MAX : remaining_size;

          memcpy(data, bpkg->mtree->f_data + curr_offset, chunk_size);
          payload_t payload = payload_create_res(curr_offset, chunk_size, chk_node->expected_hash, bpkg->ident, data);

          send_res(peer, err, payload);

          curr_offset += chunk_size;
          remaining_size -= chunk_size;
     }
}

/**
 * @brief Sends an ACP packet to a peer.
 * @param peer Pointer to the peer.
 */
void send_acp(peer_t* peer) {
     payload_t empty_payload = { 0 };
     pkt_t* pkt = pkt_create(PKT_MSG_ACP, 0, empty_payload);
     try_send(peer, pkt);
     pkt_destroy(pkt);
}

/**
 * @brief Sends an ACK packet to a peer.
 * @param peer Pointer to the peer.
 */
void send_ack(peer_t* peer) {
     payload_t empty_payload = { 0 };
     pkt_t* pkt = pkt_create(PKT_MSG_ACK, 0, empty_payload);
     try_send(peer, pkt);
     pkt_destroy(pkt);
}

/**
 * @brief Sends a RES packet to a peer.
 * @param peer Pointer to the peer.
 * @param err Error code to send.
 * @param payload Payload to send.
 */
void send_res(peer_t* peer, uint8_t err, payload_t payload) {
     pkt_t* pkt = pkt_create(PKT_MSG_RES, err, payload);
     try_send(peer, pkt);
     pkt_destroy(pkt);
}

/**
 * @brief Sends a REQ packet to a peer.
 * @param peer Pointer to the peer.
 * @param pkt Pointer to the packet to send.
 */
void send_req(peer_t* peer, pkt_t* pkt) {
     try_send(peer, pkt);
     pkt_destroy(pkt);
}

void send_png(peer_t* peer) {
     payload_t empty_payload;
     memset(&empty_payload, 0, sizeof(payload_t));
     pkt_t* pkt = pkt_create(PKT_MSG_PNG, 0, empty_payload);
     try_send(peer, pkt);
     pkt_destroy(pkt);
}

void send_png_all(peers_t* peers)
{
     pthread_mutex_lock(&peers->lock);
     for ( int i = 0; i < peers->npeers_cur; i++ )
     {
          if ( !peers->list[i] ) continue;

          payload_t empty_payload = { 0 };
          if ( peers->list[i]->reqs_q ) {
               pkt_t* pkt = pkt_create(PKT_MSG_PNG, 0, empty_payload);
               request_t* req = req_create(pkt);
               reqs_enqueue(peers->list[i]->reqs_q, req);
          }
     }
     pthread_mutex_unlock(&peers->lock);
}

void send_pog(peer_t* peer) {
     payload_t empty_payload;
     memset(&empty_payload, 0, sizeof(payload_t));
     pkt_t* pkt = pkt_create(PKT_MSG_POG, 0, empty_payload);
     try_send(peer, pkt);
     pkt_destroy(pkt);
}

void send_dsn(peer_t* peer) {
     payload_t empty_payload;
     memset(&empty_payload, 0, sizeof(payload_t));
     pkt_t* pkt = pkt_create(PKT_MSG_DSN, 0, empty_payload);
     try_send(peer, pkt);
     pkt_destroy(pkt);
}


int acp_wait_ack(peer_t* peer) {
     debug_print("Waiting for ACK from peer at port %d...\n", peer->port);
     send_acp(peer);

     pkt_t* pkt = peer_try_receive(peer);

     if ( pkt != NULL && pkt->msg_code == PKT_MSG_ACK ) {
          pkt_destroy(pkt);
          return 1;
     }
     else if ( pkt ) {
          pkt_destroy(pkt);
     }
     return 0;
}


/**
 * @brief Cancels all peer threads.
 * @param peers Pointer to the list of peers.
 */
void cancel_all_peers(peers_t* peers) {
     if ( !peers ) return;

     pthread_mutex_lock(&peers->lock);

     for ( size_t i = 0; i < peers->npeers_max; ++i ) {
          if ( peers->list[i] ) {
               payload_t pl = { 0 };
               pkt_t* pkt = pkt_create(PKT_MSG_DSN, 0, pl);
               request_t* req = req_create(pkt);
               reqs_enqueue(peers->list[i]->reqs_q, req);
               peers->list[i] = NULL;
          }
     }

     pthread_mutex_unlock(&peers->lock);
     pthread_mutex_destroy(&peers->lock);
     free(peers->list);
     free(peers);
}