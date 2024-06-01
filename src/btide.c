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

int main(int argc, char** argv) {
    if (argc < 2) {
        perror("Missing config filename command line argument...");
        exit(EXIT_FAILURE);
    }

    config_t* config_obj = (config_t*)my_malloc(sizeof(config_t));
    config_load(argv[1], config_obj);

    int server_fd = p2p_setup_server(config_obj->port);

    request_q_t* reqs_q = (request_q_t*)my_malloc(sizeof(request_q_t));
    requests_init(reqs_q);

    peers_t* peers = (peers_t*)my_malloc(sizeof(peers_t));
    
    peer_list_init(peers, config_obj->max_peers);

    create_server_thread(server_fd, config_obj->port, reqs_q, peers, config_obj);

    char input[256];
    while (1) {
        printf("Enter command: ");
        if (fgets(input, sizeof(input), stdin) != NULL) {
            if (btide_process_command(input, reqs_q, peers) == -1) {
                printf("Invalid command\n");
            }
        }
    }

    close(server_fd);
    requests_destroy(reqs_q);
    free(peers);
    free(config_obj);

    return 0;
}
