#include <utilities/my_utils.h>
#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/packet.h>
#include <peer_2_peer/package.h>
#include <cli.h>

#ifndef BTIDE_H
#define BTIDE_H
// Shared data structures
peers_t* peers;
bpkgs_t* bpkgs;
request_q_t* reqs_q;
int server_fd = 0;
pthread_t server_thread;

// Signal handler for graceful shutdown
void signal_handler(int signum);

// Function for graceful shutdown
void graceful_shutdown();
// Function to initialize the server
void init_server(char* config_filename);

int main(int argc, char** argv);

void cli_run(request_q_t* reqs_q, peers_t* peers, bpkgs_t* bpkgs);

#endif