#include <peer_2_peer/package.h>
#include <peer_2_peer/packet.h>
#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/peer_handler.h>

void peer_cleanup_handler(void* arg) {
     peer_thr_args_t* args = (peer_thr_args_t*)arg;
     if ( args == NULL ) {
          debug_print("Cleanup handler received a NULL argument.\n");
          return;
     }

     if ( args->peer != NULL ) {
          debug_print("Starting cleanup for peer at port %d and IP %s.\n",
               args->peer->port, args->peer->ip);
          if ( args->peer->sock_fd >= 0 ) {
               int close_result = close(args->peer->sock_fd);
               args->peer->sock_fd = -1;
               if ( close_result == 0 ) {
                    debug_print("Socket closed successfully.\n");
               }
               else {
                    perror("Failed to close socket\n");
               }

               peer_outgoing_requests_destroy(args->peer, args->requests);
          }
          debug_print("Cleanup completed for peer.\n");
     }
     else {
          debug_print("No peer found to clean up.\n");
     }
     free(args);
     return;
}

void peer_handler(void* args_void) {
     pthread_cleanup_push(peer_cleanup_handler, args_void);

     peer_thr_args_t* args = (peer_thr_args_t*)args_void;
     peer_t* peer = args->peer;
     peers_t* peers = args->peers;
     bpkgs_t* bpkgs = args->bpkgs;

     request_q_t* reqs_q = args->requests;
     request_t* req_recent = NULL;

     acp_wait_ack(peer);

     while ( true ) {
          if ( req_recent ) {
               req_destroy(req_recent);
          }
          req_recent = peer_process_request_shared(peer, reqs_q, peers);

          pkt_t* pkt = peer_try_receive(peer);

          if ( pkt != NULL ) {
               process_pkt_in(peer, pkt, bpkgs, req_recent);
          }

          pthread_testcancel();
     }
     pthread_cleanup_pop(1);
}

void peer_create_thread(peer_t* new_peer, request_q_t* reqs_q, peers_t* peers,
     bpkgs_t* bpkgs) {
     peer_thr_args_t* args = my_malloc(sizeof(peer_thr_args_t));

     args->peer = new_peer;
     args->requests = reqs_q;
     args->peers = peers;
     args->bpkgs = bpkgs;

     // Create the thread
     int result =
          pthread_create(&new_peer->thread, NULL, ( (void*)&peer_handler ), args);
     if ( result != 0 ) {
          perror("Thread creation failed....\n");
          free(args);
          exit(EXIT_FAILURE);
     }
}

pkt_t* peer_try_receive(peer_t* peer) {
     if ( !peer || peer->sock_fd < 0 ) {
          debug_print("Invalid peer or socket not initialized.\n");
          return NULL;
     }

     struct timeval tv = { 3, 0 };  // Timeout set to 3 seconds.
     setsockopt(peer->sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

     uint8_t* buffer = malloc(PAYLOAD_MAX);
     if ( !buffer ) {
          perror("Failed to allocate buffer for receiving packet\n");
          return NULL;
     }

     ssize_t received = 0;
     debug_print("Starting to receive data...\n");
     while ( received < PAYLOAD_MAX ) {
          ssize_t n =
               recv(peer->sock_fd, buffer + received, PAYLOAD_MAX - received, 0);
          if ( n < 0 ) {
               perror("Receive failed or timed out\n");
               free(buffer);
               return NULL;
          }
          if ( n == 0 ) {
               perror("Connection closed by peer.\n");
               free(buffer);
               return NULL;
          }
          received += n;
     }
     debug_print("Data received successfully. Unmarshalling packet...\n");

     pkt_t* pkt = malloc(sizeof(pkt_t));
     if ( !pkt ) {
          perror("Failed to allocate packet\n");
          free(buffer);
          return NULL;
     }
     pkt_unmarshall(pkt, buffer);

     free(buffer);
     debug_print("Packet unmarshalled successfully.\n");
     return pkt;
}

request_t* peer_process_request_shared(peer_t* peer, request_q_t* reqs_q,
     peers_t* peers) {
     if ( !peer || !reqs_q ) {
          debug_print("Invalid arguments to peer_process_request_shared.\n");
          return NULL;
     }

     pthread_mutex_lock(&reqs_q->lock);
     request_t* req = reqs_nextup(reqs_q);

     if ( req != NULL && req->peer->port == peer->port &&
          strcmp(req->peer->ip, peer->ip) == 0 ) {
          debug_print("Processing request for peer at port %d and IP %s.\n",
               peer->port, peer->ip);
          reqs_dequeue(reqs_q);
          process_pkt_out(peer, req->pkt);
          debug_print(
               "Request processed successfully for peer at port %d and IP %s.\n",
               peer->port, peer->ip);
     }
     else {
          debug_print("No matching request found or invalid peer details.\n");
     }

     pthread_mutex_unlock(&reqs_q->lock);
     return req;  // Ensures that a value is returned in all cases.
}

void try_send(peer_t* peer, pkt_t* pkt_out) {
     if ( !peer || peer->sock_fd < 0 ) {
          debug_print("Failed to send packet: Invalid peer or socket.\n");
          return;
     }

     uint8_t buffer[PAYLOAD_MAX];
     pkt_marshall(pkt_out, buffer);

     int total = 0;
     int bytesleft = PAYLOAD_MAX;
     int n;

     debug_print("Attempting to send packet to peer at port %d.\n", peer->port);
     while ( total < PAYLOAD_MAX ) {
          n = send(peer->sock_fd, buffer + total, bytesleft, 0);
          if ( n == -1 ) {
               perror("Failed to send packet");
               debug_print(
                    "Failed to send packet to peer at port %d after %d bytes.\n",
                    peer->port, total);
               return;
          }
          total += n;
          bytesleft -= n;
     }

     if ( total == PAYLOAD_MAX ) {
          debug_print("Successfully sent entire packet to peer at port %d.\n",
               peer->port);
     }
     else {
          debug_print(
               "Sent incomplete packet to peer at port %i; sent %d of %d "
               "bytes.\n",
               peer->port, total, bytesleft);
     }
}

void send_acp(peer_t* peer) {
     pkt_t* pkt = pkt_create(PKT_MSG_ACP, 0, NULL);
     try_send(peer, pkt);
}

void send_ack(peer_t* peer) {
     pkt_t* pkt = pkt_create(PKT_MSG_ACK, 0, NULL);
     try_send(peer, pkt);
}

void send_res(peer_t* peer, uint8_t err, payload_t* payload) {
     pkt_t* pkt = pkt_create(PKT_MSG_RES, err, payload);
     try_send(peer, pkt);
}

void send_req(peer_t* peer, pkt_t* pkt) { try_send(peer, pkt); }

void send_png(peer_t* peer) {
     pkt_t* pkt = pkt_create(PKT_MSG_PNG, 0, NULL);
     try_send(peer, pkt);
}

void send_pog(peer_t* peer) {
     pkt_t* pkt = pkt_create(PKT_MSG_POG, 0, NULL);

     try_send(peer, pkt);
}

void send_dsn(peer_t* peer) {
     pkt_t* pkt = pkt_create(PKT_MSG_DSN, 0, NULL);
     try_send(peer, pkt);
}

int acp_wait_ack(peer_t* peer) {
     send_acp(peer);

     pkt_t* pkt = peer_try_receive(peer);

     if ( pkt != NULL && pkt->msg_code == PKT_MSG_ACK ) {
          return 1;
     }
     return 0;
}

void process_pkt_in(peer_t* peer, pkt_t* pkt_in, bpkgs_t* bpkgs,
     request_t* req_recent) {
     if ( !peer || !pkt_in || !bpkgs ) {
          debug_print(
               "Invalid input to process_pkt_in: NULL peer, packet, or package "
               "manager.\n");
          return;
     }

     int err = 0;
     payload_t* payload = NULL;

     debug_print("Processing incoming packet with message code: %d\n",
          pkt_in->msg_code);
     switch ( pkt_in->msg_code ) {
     case PKT_MSG_PNG:
          send_pog(peer);
          debug_print(
               "Sent POG in response to PNG from peer at port %d.\n",
               peer->port);
          break;

     case PKT_MSG_ACP:
          send_ack(peer);
          debug_print(
               "Sent ACK in response to ACP from peer at port %d.\n",
               peer->port);
          break;

     case PKT_MSG_REQ:
          payload = payload_get_res_for_req(pkt_in->payload, bpkgs);
          if ( payload == NULL ) {
               err = -1;
               debug_print(
                    "Failed to retrieve payload for request from peer at "
                    "port %d.\n",
                    peer->port);
          }
          send_res(peer, err, payload);
          debug_print(
               "Response sent to REQ from peer at port %d with error "
               "status %d.\n",
               peer->port, err);
          break;

     case PKT_MSG_DSN:
          send_dsn(peer);
          pthread_exit((void*)0);
          debug_print(
               "Disconnected and thread exited in response to DSN from "
               "peer at port %d.\n",
               peer->port);
          break;

     case PKT_MSG_RES:
          if ( pkt_install(pkt_in, peer, bpkgs) < 0 ) {
               req_recent->status = FAILED;
               debug_print(
                    "Failed to install packet from peer at port %d.\n",
                    peer->port);
          }
          else {
               req_recent->status = SUCCESS;
               pthread_cond_signal(&req_recent->cond);
               debug_print(
                    "Successfully received and installed packet from peer "
                    "at port %d.\n",
                    peer->port);
          }
          break;

     default:
          debug_print(
               "Received unrecognized packet type from peer at port %d.\n",
               peer->port);
          break;
     }
}

void process_pkt_out(peer_t* peer, pkt_t* pkt) {
     switch ( pkt->msg_code ) {
     case PKT_MSG_PNG:
          send_png(peer);
          break;
     case PKT_MSG_REQ:
          send_req(peer, pkt);
          break;
     case PKT_MSG_DSN:
          send_dsn(peer);
          pthread_cancel(peer->thread);
          pthread_join(peer->thread, NULL);
          peer = NULL;
          break;
     default:
          break;
     }
}

void cancel_all_peers(peers_t* peers) {
     if ( !peers ) return;

     pthread_mutex_lock(&peers->lock);

     for ( size_t i = 0; i < peers->npeers_max; ++i ) {
          if ( peers->list[i] ) {
               if ( peers->list[i]->thread ) {
                    pthread_cancel(peers->list[i]->thread);
                    pthread_join(peers->list[i]->thread, NULL);
                    debug_print(
                         "Cancelled and joined thread for peer at IP %s and "
                         "port %d.\n",
                         peers->list[i]->ip, peers->list[i]->port);
               }
               if ( peers->list[i]->sock_fd >= 0 ) {
                    close(peers->list[i]->sock_fd);
                    peers->list[i]->sock_fd = -1;
                    debug_print(
                         "Closed socket for peer at IP %s and port %d.\n",
                         peers->list[i]->ip, peers->list[i]->port);
               }
               free(peers->list[i]);
          }
     }

     pthread_mutex_unlock(&peers->lock);
}