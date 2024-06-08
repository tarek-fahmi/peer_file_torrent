#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/peer_handler.h>
#include <peer_2_peer/packet.h>
#include <peer_2_peer/package.h>
#include <cli.h>
#include <chk/pkgchk.h>
#include <sys/socket.h>
#include <tree/merkletree.h>
#include <utilities/my_utils.h>

// Standard Linux Dependencies:
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <netinet/in.h>


/**
 *@brief Execute command triggered peer connection: Try
*/
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

    if (connect(peer->sock_fd, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) < 0)
    {
        perror("Failed to connect to peer...");
        close(peer->sock_fd);
        free(peer);
    }
    

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
    pkt_t* dsn_pkt = (pkt_t*) my_malloc(sizeof(pkt_t));
    dsn_pkt->msg_code = PKT_MSG_DSN;

    request_t* req = req_create(dsn_pkt, peer_target);
    reqs_enqueue(reqs_q, req);
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
void cli_add_package(char* filename, bpkgs_t* bpkgs)
{
    if (filename == NULL || strlen(filename) <= 1){
        perror("Missing file argument.\n");
        return;
    }
    
    char filepath[512];
    strcat(filepath, bpkgs->directory);
    strcat(filepath, filename);
    strcat(filepath, "/0");

    bpkg_t* bpkg = bpkg_load(filepath);
    if (bpkg == NULL)
    {
        return;
    }

   if (pkgs_add(bpkgs, bpkgs) < 0)
   {
    debug_print("Failed to add new package to shared package resource manager...");
    return;
   }

   return;
}

/**
 * @brief Remove a package that is being maintained.
 * 
 * @param filename: the name of the package to remove.
 * 
 * @returns -1 if unsuccessful.
*/
void cli_rem_package(char* ident, bpkgs_t* bpkgs)
{
    pkgs_rem(bpkgs, ident);
}

/**
 * @briefReport the statuses of the packages loaded.
 * 
 * @returns -1 if unsuccessful.
*/
void cli_report_packages(bpkgs_t* bpkgs)
{
    pthread_mutex_lock(&bpkgs->lock);

    q_node_t* current = bpkgs->ls->head;
    int i = 1;

    while (current != NULL)
    {
        bpkg_t* bpkg_curr = (bpkg_t*) current->data;
        mtree_node_t* root = bpkg_curr->mtree->root;

        if (strncmp(root->expected_hash, root->computed_hash, SHA256_HEXLEN) == 0)
        {
            printf("%d. [%1$.32s], %s : \n", i, bpkg_curr->ident, bpkg_curr->filename, "COMPLETE");
        }
        else
        {
            printf("%d. [%1$.32s], %s : \n", i, bpkg_curr->ident, bpkg_curr->filename, "INCOMPLETE");
        }
        current = current->next;
    }
    pthread_mutex_unlock(&bpkgs->lock);
}

/**
 * @brief Lists and pings all connected peers.
 * 
 * @returns -1 if unsuccessful.
*/
void cli_list_peers(peers_t* peers)
{
    pthread_mutex_lock(&peers->lock);
    
    if(peers->npeers_cur <= 0)
    {
        printf("Not connected to any peers\n");
        return -1;
    }
    else
    {
        printf("Connected to:\n\n");
    }

    for(int i=0; i<peers->npeers_max; i++)
    {
        int j = 1;
        peer_t* current = peers->list[i];
        if (current != NULL)
        {
            printf("$d. %s:%d\n");
            j++;
        }
    }
}

/**
 * @brief Requests chunks related to a given hash.
 * 
 * @param args: the string containging the hash, ip, port, offset, and identifier.
 * 
 * @returns -1 if unsuccessful.
*/
void cli_fetch(char* args, bpkgs_t* bpkgs, peers_t* peers, request_q_t* reqs_q)
{
    char ip[INET_ADDRSTRLEN];
    uint32_t port;
    char* ident = (char*) my_malloc(sizeof(char) * IDENT_MAX);
    char* hash = (char*) my_malloc(sizeof(char) *  SHA256_HEXLEN);
    uint32_t offset;

    if (sscanf(args, "%s:%u %s %s %s", ip, &port, ident, hash, &offset) < 4)
    {
        perror("Missing arguments from command\n");
        return;
    }

    peer_t* peer = peers_find(peers, ip, port);
    if (peer == NULL)
    {
        perror("Unable to request chunk, peer not in list\n");
        return;
    }

    bpkg_t* bpkg = pkg_find_by_ident(bpkgs, ident);
    if (bpkg == NULL)
    {
        perror("Unable to request chunk, package is not managed\n");
        return;
    }

    mtree_node_t *chk_node = bpkg_find_node_from_hash(bpkg->mtree, hash);

    pkt_t* pkt_tofind = pkt_prepare_request_pkt(bpkg, chk_node);

    if (pkt_fetch_from_peer(peer, pkt_tofind, bpkg, reqs_q) < 0)
    {
        debug_print("Failed to fetch packet from peer...\n");
        return;
    }
    else
    {
        debug_print("Successfully fetched packet from peer!\n");
        return;
    }
}

/**
 * @brief Parses a command, identifying and running valid commands with respective arguments.
 * 
 * @param input: A string which was inputted into the CLI, allowing return of the command.
 * 
 * @returns Intger, 0 if valid command, -1 if invalid command.
*/
void cli_process_command(char* input, request_q_t* reqs_q, peers_t* peers, bpkgs_t* bpkgs) 
{
    char* ip = (char*)my_malloc(INET_ADDRSTRLEN);
    uint32_t port;
    char* arguments;
    char* command = strtok_r(input, " ", &arguments);

    if (strcmp(command, "CONNECT") == 0) 
    {
        sscanf(arguments, "%s:%u", ip, &port);
        cli_connect(ip, port, reqs_q, peers, bpkgs);
    }
     else if (strcmp(command, "DISCONNECT") == 0) 
    {
        sscanf(arguments, "%s:%u", ip, &port);
        cli_disconnect(ip, port, peers, bpkgs);
    }
     else if (strcmp(command, "ADDPACKAGE") == 0) 
    {
        cli_add_package(arguments, bpkgs);
    }
     else if (strcmp(command, "REMPACKAGE") == 0) 
    {
        cli_rem_package(arguments, bpkgs);
    }
     else if (strcmp(command, "PACKAGES") == 0) 
    {
        cli_report_packages(bpkgs);
    }
     else if (strcmp(command, "PEERS") == 0) 
     {
        cli_list_peers(peers);
    }
     else if (strcmp(command, "FETCH") == 0) 
    {
        cli_fetch(arguments, bpkgs, peers);
    }
     else if (strcmp(command, "QUIT") == 0) 
    {
        free(ip);
        exit(EXIT_SUCCESS);
    } else {
        free(ip);
        return -1;
    }

    free(ip);
    return 0;
}