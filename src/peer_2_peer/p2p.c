#include <crypt/sha256.h>
#include <tree/merkletree.h>
#include <chk/pkgchk.h>
#include <chk/pkg_helper.h>
#include <utilities/my_utils.h>
#include <peer_2_peer/peer_handler.h>
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

void process_request_shared(peer_t* peer, request_q_t* reqs_q, peers_t* peers) {
    pthread_mutex_lock(&reqs_q->lock);
    request_t* req = req_nextup(reqs_q);

    if (req != NULL && req->peer->port == peer->port) {
        uint16_t code = req->packet->msg_code;

        switch (code) {
            case PKT_MSG_PNG:
                send_png(peer);
                break;
            case PKT_MSG_ACP:
                send_acp(peer);
                break;
            case PKT_MSG_REQ:
                try_send(peer, req->packet);
                break;
            case PKT_MSG_DSN:
                send_dsn(peer);
                break;
        }

        req_dequeue(reqs_q);
    }

    pthread_mutex_unlock(&reqs_q->lock);
}

void peer_handler(void *args_void) {
    peer_thr_args_t *args = (peer_thr_args_t*) args_void;
    peer_t* peer = args->peer;
    request_q_t *reqs_q = args->requests;
    peers_t* peers = args->peers;

    while (true) {
        packet_t* pkt = peer_try_receive(peer);

        if (pkt != NULL) {
            handle_packet(peer, pkt);
        }
        process_request_shared(peer, reqs_q, peers);
    }
}