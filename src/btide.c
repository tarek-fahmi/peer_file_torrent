// Local Dependencies:
#include <pkgchk.h>
#include <btide.h>
#include <my_utils.h>
#include <config.h>
#include <pcomm.h>
#include <peer.h>
#include <p2p.h>
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
#include <arpa/inet.h>
//
// PART 2
//

//TODO: Add functions declared in header file.

typedef struct server_thr_args{
    request_q_t requests_in;
    request_q_t requests_out;
    config_t config;
}server_thr_args;


//TODO: Make sure to free heap memory (config, ...), + Check if types passed into functions match those declared in structures.
/**
 * @brief Takes a command a string, and parses it, identifying and running valid commands.
 * 
 * @param input: A string which was inputted into the CLI, allowing return of the command.
 * 
 * @returns Intger, 0 if valid command, -1 if invalid command.
*/
int btide_command_manager(char* input){
    
    char* ip = (char*) malloc(sizeof INET_ADDRSTRLEN);
    uint32_t port;

    char* arguments;
    char* command = strtok_r(input, " ", &arguments);

    if (strcmp(command, "CONNECT") == 0)
    { 
        sscanf(arguments, "%s:%u", ip, &port);
        btide_connect(ip, port);
    }
    else if (strcmp(command, "DISCONNECT") == 0)
    {
        sscanf(arguments, "%s:%u", ip, &port);
        btide_disconnect(ip, port);
    }
    else if (strcmp(command, "ADDPACKAGE") == 0)
    {
        
        btide_add_package(arguments);
    }
    else if (strcmp(command, "REMPACKAGE") == 0)
    {
        btide_rem_package(arguments);
    }
    else if (strcmp(command, "PACKAGES") == 0)
    {
        btide_report_packages();
    }
    else if (strcmp(command, "PEERS") == 0)
    {
        btide_list_peers();
    }
    else if (strcmp(command, "FETCH") == 0)
    {
        //TODO: Make below function handle minimum argument number.
        int offset;
        char* ident;
        char* filename;
        char* hash;

        btide_fetch(arguments);
    }
    else if (strcmp(command, "QUIT") == 0)
    {
     exit(EXIT_SUCCESS);
    }
    else
    { //Unknown Command inputted, return error.P
        free(ip);
        return -1;
    }
    return 0; // Command parsed and executed successfully.

}

int main(int argc, char** argv) 
{
    //TODO: Main server listening thread, CLI loop, shared peers and requests.
    if (argc < 2)
    {
        perror("Missing config filename command line argument...");
        exit(EXIT_FAILURE);
    }

    config_t* config_obj = (config_t*) malloc(sizeof(config_t));
    if (!config_obj)
    {
        free(config_obj);
        perror("Failed to allocate memory for config...");
        exit(EXIT_FAILURE);
    }
    
    config_load(argv[1], config_obj);
    int server_fd = p2p_setup_server(config_obj->port);

    
    
    request_q_t* reqs_q = (request_q_t*)malloc(sizeof(request_q_t));
    if (!reqs_q)
    {
        free(reqs_q);
        perror("Failed to allocate memory for requests queue...");
        exit(EXIT_FAILURE);
    }

    requests_init(reqs_q);
    peers_t* peers = (peers_t*) malloc(sizeof(peers_t));



    free(config_obj);
}

