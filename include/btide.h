// Local Dependencies:
#include <pkgchk.h>
// Standard Linux Dependencies:
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
// Additional Linux Dependencies:
#include <fcntl.h>
#include <string.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
//
// PART 2
//

#define MAX_COMMAND_LENGTH (5220)


/**
 * @brief Attempts to connect to a peer within the network.
 * 
 * @param IP, the peer's IP address.
 * @param port, the port to connect via.
 * 
 * @returns -1 if unsuccessful.
*/
int btide_connect(char* ip, uint16_t port);

/**
 * @brief Disconnect from a peer wnd remove it from a peer list.
 * 
* @param IP, the peer's IP address.
 * @param port, the port to connect via.
 * 
 * @returns
*/
int btide_disconnect(char* ip, uint16_t port);

/**
 * @brief Add a package to manage.
 * 
 * 
 * @param filename, the name of the package to add.
 * 
 * @returns -1 if unsuccessful.
*/
int btide_add_package(char* filename);

/**
 * @brief Remove a package that is being maintained.
 * 
 * @param filename: the name of the package to remove.
 * 
 * @returns -1 if unsuccessful.
*/
int btide_rem_package(char* filename);

/**
 * @briefReport the statuses of the packages loaded.
 * 
 * @returns -1 if unsuccessful.
*/
int btide_report_packages();

/**
 * @brief Lists and pings all connected peers.
 * 
 * @returns -1 if unsuccessful.
*/
int btide_list_peers();

/**
 * @brief Requests chunks related to a given hash.
 * 
 * @param args: the string containging the hash, ip, port, offset, and identifier.
 * 
 * @returns -1 if unsuccessful.
*/
int btide_fetch(char* args);

/**
 * @brief Parses a command, identifying and running valid commands with respective arguments.
 * 
 * @param input: A string which was inputted into the CLI, allowing return of the command.
 * 
 * @returns Intger, 0 if valid command, -1 if invalid command.
*/
int btide_command_manager(char* input);