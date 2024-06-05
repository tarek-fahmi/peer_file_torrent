#ifndef CLI_H
#define CLI_H

#include <sys/select.h>
#include <peer_2_peer/peer_data_sync.h>

void cli_connect(char* ip, int port, request_q_t* reqs_q, peers_t* peers, bpkgs_t* bpkgs);


/**
 * @brief Add a package to manage.
 * 
 * 
 * @param filename, the name of the package to add.
 * 
 * @returns -1 if unsuccessful.
*/
void cli_disconnect(char* ip, int port, request_q_t* reqs);



/**
 * @brief Add a package to manage.
 * 
 * 
 * @param filename, the name of the package to add.
 * 
 * @returns -1 if unsuccessful.
*/
int cli_add_package(char* filename);
/**
 * @brief Remove a package that is being maintained.
 * 
 * @param filename: the name of the package to remove.
 * 
 * @returns -1 if unsuccessful.
*/
int cli_rem_package(char* filename);
/**
 * @briefReport the statuses of the packages loaded.
 * 
 * @returns -1 if unsuccessful.
*/
int cli_report_packages();

/**
 * @brief Lists and pings all connected peers.
 * 
 * @returns -1 if unsuccessful.
*/
int cli_list_peers();

/**
 * @brief Requests chunks related to a given hash.
 * 
 * @param args: the string containging the hash, ip, port, offset, and identifier.
 * 
 * @returns -1 if unsuccessful.
*/
int cli_fetch(char* args);

/**
 * @brief Parses a command, identifying and running valid commands with respective arguments.
 * 
 * @param input: A string which was inputted into the CLI, allowing return of the command.
 * 
 * @returns Intger, 0 if valid command, -1 if invalid command.
*/
int cli_command_manager(char* input);
int cli_process_command(char* input, request_q_t reqs_q, peers_t peers, bpkgs_t bpkgs) {
    char* ip = (char*)my_malloc(INET_ADDRSTRLEN);
    uint32_t port;
    char* arguments;
    char* command = strtok_r(input, " ", &arguments);

    if (strcmp(command, "CONNECT") == 0) {
        sscanf(arguments, "%s:%u", ip, &port);
    } else if (strcmp(command, "DISCONNECT") == 0) {
        sscanf(arguments, "%s:%u", ip, &port);
        cli_disconnect(ip, port, reqs_q);
    } else if (strcmp(command, "ADDPACKAGE") == 0) {
        cli_add_package(arguments);
    } else if (strcmp(command, "REMPACKAGE") == 0) {
        cli_rem_package(arguments);
    } else if (strcmp(command, "PACKAGES") == 0) {
        cli_report_packages();
    } else if (strcmp(command, "PEERS") == 0) {
        cli_list_peers();
    } else if (strcmp(command, "FETCH") == 0) {
        cli_fetch(arguments);
    } else if (strcmp(command, "QUIT") == 0) {
        free(ip);
        exit(EXIT_SUCCESS);
    } else {
        free(ip);
        return -1;
    }

    free(ip);
    return 0;
}

#endif
