#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <cli.h>
#include <config.h>
#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/peer_server.h>
#include <utilities/my_utils.h>

// Shared data structures
peers_t* peers = NULL;
bpkgs_t* bpkgs = NULL;
request_q_t* reqs_q = NULL;
int server_fd = 0;
pthread_t server_thread;

// Flag for signal handling
volatile sig_atomic_t stop = 0;


// Function for graceful shutdown
void graceful_shutdown() {
     debug_print("Shutting btide down now...\n");

     if ( server_fd > 0 ) {
          close(server_fd);
     }

     if ( server_thread ) {
          pthread_cancel(server_thread);
          pthread_join(server_thread, NULL);
     }

     if ( peers ) {
          cancel_all_peers(peers);
          peers_destroy(peers);
     }

     if ( reqs_q ) {
          reqs_destroy(reqs_q);
     }

     if ( bpkgs ) {
          pkgs_destroy(bpkgs);
     }

     debug_print("All memory for btide has been deallocated...\n\n");
     debug_print("Bye bye! :D\n");
     exit(EXIT_SUCCESS);
}

// Signal handler for graceful shutdown
void signal_handler(int signum) {
     stop = 1;
     debug_print("Signal %d received. Initiating graceful shutdown...\n", signum);
     graceful_shutdown();
}


// Function to initialize the server
void init_server(char* config_filename) {
     // Load configuration
     config_t* config = config_load(config_filename);
     if ( !config ) {
          perror("Failed to load configuration");
          graceful_shutdown();
          exit(EXIT_FAILURE);
     }

     uint32_t server_port = config->port;
     bpkgs = pkgs_init(config->directory);
     reqs_q = reqs_create();
     peers = peer_list_create(config->max_peers);

     server_fd = p2p_setup_server(server_port);

     create_p2p_server_thread(server_fd, server_port, &server_thread, reqs_q, peers, bpkgs);
}

int main(int argc, char** argv) {
     if ( argc < 2 ) {
          perror("Missing config filename command line argument.\n");
          exit(EXIT_FAILURE);
     }

     // Set up signal handler
     if ( signal(SIGINT, signal_handler) == SIG_ERR ) {
          perror("Error setting up signal handler");
          exit(EXIT_FAILURE);
     }

     // Initialize server and create server listening thread to accept incoming connections.
     init_server(argv[1]);

     // Run the command line interface and command handling thread in main.
     cli_run(reqs_q, peers, bpkgs);

     // Wait for request cancellation of peer communication and server listening threads,
     // and deallocate all shared memory pools prior to shutdown.
     graceful_shutdown();

     return 0;
}

void cli_run(request_q_t* reqs_q, peers_t* peers, bpkgs_t* bpkgs) {
     char input[MAX_COMMAND_LENGTH];
     while ( !stop ) {
          if ( fgets(input, sizeof(input), stdin) != NULL ) {
               debug_print("Input: '%s'\n", input);
               if ( cli_process_command(input, reqs_q, peers, bpkgs) == 0 ) {
                    break;
               }
          }
          else {
               if ( feof(stdin) ) {
                    break; // Exit on EOF
               }
          }
     }
     debug_print("Shutting down btide due to EOF or SIGINT...\n");
}
