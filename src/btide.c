// Local Dependencies:
#include <pkgchk.h>
#include <btide.h>
#include <utils.h>
#include <config.h>
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

//TODO: Add functions declared in header file.


//TODO: Make sure to free heap memory (config, ...), + Check if types passed into functions match those declared in structures.
/**
 * @brief Takes a command a string, and parses it, identifying and running valid commands.
 * 
 * @param input: A string which was inputted into the CLI, allowing return of the command.
 * 
 * @returns Intger, 0 if valid command, -1 if invalid command.
*/
int btide_command_manager(char* input){
    
    char* ip;
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
    else if (strcmp(command, "PACKAGES") == 0)
    {
        btide_list_peers();
    }
    else if (strcmp(command, "PEERS") == 0)
    {
        //TODO: Make below function handle minimum argument number.
        btide_fetch(arguments);
    }

    else if (strcmp(command, "FETCH") == 0)
    {
        int offset;
        char* ident;
        char* filename;
        char* hash;
    }
    else if (strcmp(command, "QUIT") == 0)
    {
        exit(EXIT_SUCCESS);
    }

}



int main(int argc, char** argv) 
{
    if (argc < 2)
    {
        perror("Missing config filename command line argument...");
        exit(EXIT_FAILURE);
    }

    config_t* config = (config_t*) malloc(sizeof(config_t));

    if (!config)
    {
        free(config);
        perror("Failed to allocate memory for config...");
        exit(EXIT_FAILURE);
    }

    config = config_load(argv[1]);

    char command[MAX_COMMAND_LENGTH];
    while (fgets(command, MAX_COMMAND_LENGTH, stdin))
    {
        btide_command_manager(command);
        
    }

    free(config);
}

