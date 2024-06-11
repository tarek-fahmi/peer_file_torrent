#ifndef PEER_2_PEER_PEER_HANDLER_H
#define PEER_2_PEER_PEER_HANDLER_H

#include <peer_2_peer/package.h>
#include <peer_2_peer/packet.h>
#include <peer_2_peer/peer_data_sync.h>

/**
 * @brief Arguments for peer handler thread.
 */
typedef struct peer_thr_args {
     peer_t* peer;       /** Pointer to the peer. */
     peers_t* peers;     /** Pointer to the list of peers*/
     bpkg_t* bpkg;       /** Pointer to the package. */
     bpkgs_t* bpkgs;     /**  Pointer to the package manager */
} peer_thr_args_t;

/**
 * @brief Cleans up resources used by the peer handler thread.
 * @param arg Pointer to the arguments for the peer handler thread.
 */
void peer_cleanup_handler(void* arg);

/**
 * @brief Main function for the peer handler thread.
 * @param args_void Pointer to the arguments for the peer handler thread.
 */
void peer_handler(void* args_void);

/**
 * @brief Creates a new peer handler thread.
 * @param new_peer Pointer to the peer.
 * @param peers Pointer to the list of peers.
 * @param bpkgs Pointer to the package manager.
 */
void peer_create_thread(peer_t* new_peer, peers_t* peers, bpkgs_t* bpkgs);

/**
 * @brief Attempts to receive a packet from a peer.
 * @param peer Pointer to the peer.
 * @return Pointer to the received packet, or NULL if receiving failed.
 */
pkt_t* peer_try_receive(peer_t* peer);

/**
 * @brief Sends a packet to a peer.
 * @param peer Pointer to the peer.
 * @param pkt_out Pointer to the packet to send.
 */
void try_send(peer_t* peer, pkt_t* pkt_out);

/**
 * @brief Sends an ACP packet to a peer.
 * @param peer Pointer to the peer.
 */
void send_acp(peer_t* peer);

/**
 * @brief Sends an ACK packet to a peer.
 * @param peer Pointer to the peer.
 */
void send_ack(peer_t* peer);

/**
 * @brief Sends a RES packet to a peer.
 * @param peer Pointer to the peer.
 * @param err Error code to send.
 * @param payload Payload to send.
 */
void send_res(peer_t* peer, uint8_t err, payload_t payload);


/**
 * @brief Sends a  series of (or one, as required ) RES packets to a peer.
 * @param peer Pointer to the peer.
 * @param err Error code to send.
 * @param payload Payload to send.
 */
void send_res_pkts(peer_t* peer, pkt_t* pkt_in, bpkgs_t* bpkgs);

/**
 * @brief Sends a REQ packet to a peer.
 * @param peer Pointer to the peer.
 * @param pkt Pointer to the packet to send.
 */
void send_req(peer_t* peer, pkt_t* pkt);

/**
 * @brief Sends a PNG packet to a peer.
 * @param peer Pointer to the peer.
 */
void send_png(peer_t* peer);

/**
 * @brief Sends a POG packet to a peer.
 * @param peer Pointer to the peer.
 */
void send_pog(peer_t* peer);

/**
 * @brief Sends a DSN packet to a peer.
 * @param peer Pointer to the peer.
 */
void send_dsn(peer_t* peer);

/**
 * @brief Sends a PNG packet to all peers.
 * @param peer Pointer to the peer.
 */
void send_png_all(peers_t* peers);

/**
 * @brief Waits for an ACK packet from a peer.
 * @param peer Pointer to the peer.
 * @return 1 if ACK received, 0 otherwise.
 */
int acp_wait_ack(peer_t* peer);

/**
 * @brief Processes an incoming packet from a peer.
 * @param peer Pointer to the peer.
 * @param pkt_in Pointer to the incoming packet.
 * @param bpkgs Pointer to the package manager.
 * @param req_recent Pointer to the recent request.
 */
void process_pkt_in(peer_t* peer, pkt_t* pkt_in, bpkgs_t* bpkgs, request_t* req_recent, peers_t* peers);

/**
 * @brief Processes an outgoing packet for a peer.
 * @param peer Pointer to the peer.
 * @param pkt Pointer to the packet to send.
 */
void process_pkt_out(peer_t* peer, request_t* req);

/**
 * @brief Processes a shared request for a peer.
 * @param peer Pointer to the peer.
 * @return Pointer to the processed request.
 */
request_t* peer_process_request_shared(peer_t* peer);

/**
 * @brief Cancels all peer threads.
 * @param peers Pointer to the list of peers.
 */
void cancel_all_peers(peers_t* peers);

#endif