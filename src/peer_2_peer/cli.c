#include <netinet/in.h>
#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/peer_handler.h>
#include <chk/pkgchk.h>
#include <sys/socket.h>
#include <tree/merkletree.h>
#include <utilities/my_utils.h>
// Standard Linux Dependencies:
#include <stddef.h>
#include <stdint.h>

void cli_connect(char* ip, int port, request_q_t* reqs_q, peers_t* peers, bpkgs_t* bpkgs)
{
    peer_t* peer = (peer_t*) my_malloc(sizeof(peer_t));
    peer_init(peer, ip, port);
    peer->sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    check_err(peer->sock_fd, "Socket creation error...");

    struct sockaddr_in peer_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(peer->port),
    };

    int err;
    err = inet_pton(AF_INET, peer->ip, &peer_addr.sin_addr);
    check_err(err, "Invalid address/ address not supported");

    err = connect(peer->sock_fd, (struct sockaddr*)&peer_addr, sizeof(peer_addr));
    check_err(err, "Failed to connect to new peer...");

    debug_print("New connection successful!");

    peer_create_thread(peer, reqs_q, peers, bpkgs);
}

/**
 * @brief Add a package to manage.
 * 
 * 
 * @param filename, the name of the package to add.
 * 
 * @returns -1 if unsuccessful.
*/
void cli_disconnect(char* ip, int port, peers_t* peers, request_q_t* reqs_q)
{   
    peer_t* peer_target = peers_find(peers, ip, port);
    packet_t* dsn_pkt = (packet_t*) my_malloc(sizeof(packet_t));
    dsn_pkt->msg_code = PKT_MSG_DSN;

    request_t* req = req_init(dsn_pkt, peer_target);
    req_enqueue(reqs_q, req);
    debug_print("Disconnection request for peer %s has been queued");
}


/**
 * @brief Add a package to manage.
 * 
 * 
 * @param filename, the name of the package to add.
 * 
 * @returns -1 if unsuccessful.
*/
int cli_add_package(char* filename, bpkgs_t* bpkgs)
{
    if (filename == NULL || strlen(filename <= 1)){
        perror("Missing file argument.\n");
        return -1;
    }
    
    char filepath[512];
    strcat(filepath, bpkgs->directory);
    strcat(filepath, filename);

    bpkg_t* bpkg = bpkg_load(filename);
    if (bpkg == NULL)
    {
        return -1;
    }

    pkg



}

/**
 * @brief Remove a package that is being maintained.
 * 
 * @param filename: the name of the package to remove.
 * 
 * @returns -1 if unsuccessful.
*/
int cli_rem_package(char* filename)
{

}

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
int cli_process_command(char* input, request_q_t* reqs_q, peers_t* peers, bpkgs_t* bpkgs) {
    char* ip = (char*)my_malloc(INET_ADDRSTRLEN);
    uint32_t port;
    char* arguments;
    char* command = strtok_r(input, " ", &arguments);

    if (strcmp(command, "CONNECT") == 0) {
        sscanf(arguments, "%s:%u", ip, &port);
    } else if (strcmp(command, "DISCONNECT") == 0) {
        sscanf(arguments, "%s:%u", ip, &port);
        cli_disconnect(ip, port);
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
