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

void peer_handler(void *args_void) {
    peer_thr_args_t *args = (peer_thr_args_t*) args_void;
    peer_t* peer = args->peer;
    request_q_t *reqs_q = args->requests;
    peers_t* peers = args->peers;

    while (true) {
        pkt_t* pkt = peer_try_receive(peer);

        if (pkt != NULL) {
            handle_pkt(peer, pkt);
        }
        process_request_shared(peer, reqs_q, peers);
    }
}

