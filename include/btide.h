#ifndef BTIDE_H
#define BTIDE_H

#include <utilities/my_utils.h>
#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/packet.h>
#include <peer_2_peer/package.h>
#include <cli.h>

/**
 * @brief Signal handler for graceful shutdown.
 * 
 * This function handles signals (e.g., SIGINT) to initiate a graceful shutdown of the server.
 * 
 * @param signum The signal number.
 */
void signal_handler(int signum);

/**
 * @brief Performs a graceful shutdown of the server.
 * 
 * This function ensures that all resources are cleaned up and the server is properly shut down.
 */
void graceful_shutdown();

/**
 * @brief Initializes the server.
 * 
 * This function initializes the server using the configuration specified in the given file.
 * 
 * @param config_filename The name of the configuration file.
 */
void init_server(char* config_filename);

/**
 * @brief The main entry point of the application.
 * 
 * This function serves as the main entry point of the application, handling command-line arguments and starting the server.
 * 
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @return The exit status of the application.
 */
int main(int argc, char** argv);

/**
 * @brief Runs the command-line interface (CLI) for peer-to-peer interaction.
 * 
 * This function starts the CLI, allowing users to interact with peers and packages.
 * 
 * @param peers A pointer to the peers data structure.
 * @param bpkgs A pointer to the packages data structure.
 */
void cli_run(peers_t* peers, bpkgs_t* bpkgs);

#endif // BTIDE_H