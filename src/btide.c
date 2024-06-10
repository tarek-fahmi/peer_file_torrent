#include <cli.h>
#include <config.h>
#include <peer_2_peer/packet.h>
#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/peer_handler.h>
#include <peer_2_peer/peer_server.h>

// Shared data structures
peers_t* peers;
bpkgs_t* bpkgs;
request_q_t* reqs_q;
int server_fd;
pthread_t server_thread;

// Function to initialize the server
void init_server(char* config_filename) {
     // Load configuration
     config_t* config = config_load(config_filename);
     uint32_t server_port = config->port;
     bpkgs = pkgs_init(config->directory);
     reqs_q = reqs_create();
     peers = peer_list_create(config->max_peers);

     server_fd = p2p_setup_server(server_port);

     server_thread =
         create_p2p_server_thread(server_fd, server_port, reqs_q, peers, bpkgs);
}

// Signal handler for graceful shutdown
void signal_handler(int signum) {
     close(server_fd);
     reqs_destroy(reqs_q);
     peers_destroy(peers);
     pkgs_destroy(bpkgs);
     exit(signum);
}

int main(int argc, char** argv) {
     if (argc < 2) {
          perror("Missing config filename command line argument.");
          exit(EXIT_FAILURE);
     }

     // Set up signal handler
     signal(SIGINT, signal_handler);

     // Initialize server
     init_server(argv[1]);

     char input[256];
     while (1) {
          printf("Enter command: ");
          if (fgets(input, sizeof(input), stdin) != NULL) {
               cli_process_command(input, reqs_q, peers, bpkgs);
          }
     }

     // Wait for threads to finish
     cancel_all_peers(peers);
     pthread_cancel(server_thread);
     pthread_join(server_thread, NULL);
     reqs_destroy(reqs_q);
     peers_destroy(peers);

     return 0;
}