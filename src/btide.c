#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <cli.h>
#include <config.h>
#include <btide.h>
#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/peer_handler.h>
#include <peer_2_peer/peer_server.h>
#include <utilities/my_utils.h>

// Shared data structures
peers_t* peers = NULL;
bpkgs_t* bpkgs = NULL;
config_t* config = NULL;
int server_fd = 0;
pthread_t server_thread;

// Flag for signal handling
volatile sig_atomic_t stop = 0;

/**
 * @brief Performs a graceful shutdown of the server.
 *
 * This function ensures that all resources are cleaned up and the server is properly shut down.
 */
void graceful_shutdown() {
     debug_print("Shutting btide down now...\n");

     if ( peers ) {
          cancel_all_peers(peers);
     }

     if ( server_thread ) {
          pthread_cancel(server_thread);
          pthread_join(server_thread, NULL);
     }

     if ( bpkgs ) {
          pkgs_destroy(bpkgs);
     }

     if ( config ) {
          free(config);
     }

     debug_print("All memory for btide has been deallocated...\n\n");
     debug_print("Bye bye! :D\n");
     exit(EXIT_SUCCESS);
}

/**
 * @brief Initializes the server.
 *
 * This function initializes the server using the configuration specified in the given file.
 *
 * @param config_filename The name of the configuration file.
 */
void signal_handler(int signum) {
     stop = 1;
     debug_print("Signal %d received. Initiating graceful shutdown...\n", signum);
     graceful_shutdown();
}

// Function to initialize the server
void init_server(char* config_filename) {
     debug_print("Initializing server with config file: %s\n", config_filename);

     // Load configuration
     config = config_load(config_filename);
     if ( !config ) {
          fprintf(stderr, "Failed to load configuration");
          fflush(stderr);
          graceful_shutdown();
          exit(EXIT_FAILURE);
     }

     uint32_t server_port = config->port;
     debug_print("Server port: %d\n", server_port);

     bpkgs = pkgs_init(config->directory);
     peers = peer_list_create(config->max_peers);

     server_fd = p2p_setup_server(server_port);

     create_p2p_server_thread(server_fd, server_port, &server_thread, peers, bpkgs);
}

/**
 * @brief The main entry point of the application.
 *
 * This function serves as the main entry point of the application, handling command-line arguments and starting the server.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @return The exit status of the application.
 */
int main(int argc, char** argv) {
     if ( argc < 2 ) {
          perror("Missing config filename command line argument.\n");
          exit(EXIT_FAILURE);
     }

     debug_print("Starting btide with config file: %s\n", argv[1]);

     if ( signal(SIGINT, signal_handler) == SIG_ERR ) {
          perror("Error setting up signal handler");
          exit(EXIT_FAILURE);
     }

     // Intititalize server, shared attributes, and main cli thread. Else run a graceful shutdown

     init_server(argv[1]);

     cli_run(peers, bpkgs);

     graceful_shutdown();

     return 0;
}

/**
 * @brief Runs the command-line interface (CLI) for peer-to-peer interaction.
 *
 * This function starts the CLI, allowing users to interact with peers and packages.
 *
 * @param peers A pointer to the peers data structure.
 * @param bpkgs A pointer to the packages data structure.
 */
void cli_run(peers_t* peers, bpkgs_t* bpkgs) {
     char input[MAX_COMMAND_LENGTH];
     while ( !stop ) {
          if ( fgets(input, sizeof(input), stdin) != NULL ) {
               debug_print("Input: '%s'\n", input);
               if ( cli_process_command(input, peers, bpkgs) == 0 ) {
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

