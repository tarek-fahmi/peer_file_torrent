#ifndef PEER_SERVER_H
#define PEER_SERVER_H

// Local Dependencies:=
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



typedef struct server{
    struct sockaddr_in addr_obj;
    uint16_t sock;
    int port;
}server_t;

void* server_thread_handler(void* args_void);

void create_p2p_server_thread(int server_fd, int server_port, request_q_t* reqs_q, peers_t* peers, config_t* config);

int p2p_setup_server(uint16_t port);

void create_server_thread(int server_fd, int server_port, request_q_t* reqs_q, peers_t* peers);

#endif

#endif