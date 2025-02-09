#include <chk/pkgchk.h>
#include <peer_2_peer/package.h>
#include <peer_2_peer/packet.h>
#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/peer_handler.h>
#include <sys/socket.h>
#include <tree/merkletree.h>
#include <utilities/my_utils.h>

// Standard Linux Dependencies:
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define MAX_COMMAND_LENGTH (5520)

/**
 *@brief Execute command triggered peer connection: Try
 */
void cli_connect(char* ip, int port, peers_t* peers,
    bpkgs_t* bpkgs);

/**
 * @brief Add a package to manage.
 *
 *
 * @param filename, the name of the package to add.
 *
 * @returns -1 if unsuccessful.
 */
void cli_disconnect(char* ip, int port, peers_t* peers);
/**
 * @brief Add a package to manage.
 *
 *
 * @param filename, the name of the package to add.
 *
 * @returns -1 if unsuccessful.
 */
void cli_add_package(char* filename, bpkgs_t* bpkgs);

/**
 * @brief Remove a package that is being maintained.
 *
 * @param filename: the name of the package to remove.
 *
 * @returns -1 if unsuccessful.
 */
void cli_rem_package(char* ident, bpkgs_t* bpkgs);
/**
 * @briefReport the statuses of the packages loaded.
 *
 * @returns -1 if unsuccessful.
 */
void cli_report_packages(bpkgs_t* bpkgs);

/**
 * @brief Lists and pings all connected peers.
 *
 * @returns -1 if unsuccessful.
 */
void cli_list_peers(peers_t* peers);

/**
 * @brief Requests chunks related to a given hash.
 *
 * @param args: the string containging the hash, ip, port, offset, and
 * identifier.
 *
 * @returns -1 if unsuccessful.
 */
void cli_fetch(char* args, bpkgs_t* bpkgs, peers_t* peers);

/**
 * @brief Parses a command, identifying and running valid commands with
 * respective arguments.
 *
 * @param input: A string which was inputted into the CLI, allowing return of
 * the command.
 *
 * @returns Intger, 0 if valid command, -1 if invalid command.
 */
int cli_process_command(char* input, peers_t* peers,
    bpkgs_t* bpkgs);