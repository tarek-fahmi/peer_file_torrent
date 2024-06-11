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
int server_fd = 0;
pthread_t server_thread;

void graceful_shutdown() {
     debug_print("Shutting btide down now...\n");
     cancel_all_peers(peers);
     if (server_fd > 0) {
          pthread_cancel(server_thread);
          pthread_join(server_thread, NULL);
     }
     reqs_destroy(reqs_q);
     peers_destroy(peers);
     debug_print("All memory for btide has been deallocated...\n\n");
     debug_print("Bye bye! :D");
     exit(EXIT_SUCCESS);
}

// Function to initialize the server
void init_server(char* config_filename) {
     // Load configuration
     config_t* config = config_load(config_filename);
     if (!config) {
          graceful_shutdown();
     }
     uint32_t server_port = config->port;
     bpkgs = pkgs_init(config->directory);
     reqs_q = reqs_create();
     peers = peer_list_create(config->max_peers);

     server_fd = p2p_setup_server(server_port);

     create_p2p_server_thread(server_fd, server_port, &server_thread, reqs_q,
                              peers, bpkgs);
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

     // Initialize server and create server listening thread to accept incoming
     // connections.
     init_server(argv[1]);

     // Run the command line interface and command handling thread in main.
     cli_run(reqs_q, peers, bpkgs);

     // Wait request cancellation of peer communication and server listening
     // threads, and deallocate all shared memory pools prior to shutdown.
     graceful_shutdown();

     return 0;
}