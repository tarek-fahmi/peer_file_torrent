#include <crypt/sha256.h>
#include <tree/merkletree.h>
#include <chk/pkgchk.h>
#include <chk/pkg_helper.h>
#include <utilities/my_utils.h>
#include <peer_2_peer/peer_handler.h>
#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/packet.h>
#include <peer_2_peer/package.h>
#include <config.h>
#include <cli.h>
// Standard Linux Dependencies:
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
// Additional Linux Dependencies:
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/select.h>

static int parse_entry(char* line, config_t* c_obj){

        if (sscanf(line, "directory:%256s", c_obj->directory)){
            if (!check_directory(c_obj->directory))
            {
                return(ERR_DIRECTORY);
            } 
        }

        else if (sscanf(line, "max_peers:%d", &c_obj->max_peers)){
            if (c_obj->max_peers > MAX_PEERS || c_obj->max_peers < MIN_PEERS){
                fprintf(stderr, "Max peers (%d) outside of permitted range (%d - %d)\n", c_obj->max_peers, MIN_PEERS, MAX_PEERS);
                return(ERR_PEERS);
            }  
        }   

        else if (sscanf(line, "port:%hu", &c_obj->port)){
            if (c_obj->port > MAX_PORT || c_obj->port < MIN_PORT)
            {
                fprintf(stderr, "Port (%d) outside of permitted range (%d - %d)\n", c_obj->port, MIN_PORT, MAX_PORT);
                return(ERR_PORT);
            }
        }
        
        else{
            return 0;
        }
}

static int check_directory(char* pathname){
    DIR* dir = opendir(pathname);

    if (dir){
        closedir(dir);
        return 0;
    }

    else if (errno == ENOENT || errno == ENOTDIR) {

        if (mkdir(pathname, 0755))
        {
            perror("Failed to create config directory...");
            return -1;
        }

    }

    else
    {
        closedir(dir);
        perrror(strerror(errno));
        return -1;
    }
}

int config_load(char* filename, config_t* c_obj){

    //TODO: Make sure to close files and direcories when finished, and free c_obj upon failure.

    FILE* f_ptr = fopen(filename, "r");

    if (!f_ptr){
        free(c_obj);
        close(f_ptr);
        perror("Failed to open config...");
        exit(EXIT_FAILURE);
    }

    //Parse each line of file:
    char* buffer[1024];
    while (fgets(buffer, sizeof(buffer), f_ptr))
    {
        int parse_code = parse_entry(buffer, c_obj);
        if (parse_code != 0)
        {
            close(f_ptr);
            free(c_obj);
            exit(parse_code);
        }
        
    }

    //Finished parsing file, close file object.
    close(f_ptr);
    

    //Check if a config entry is missing.
    if (!(c_obj->directory) || !(c_obj->max_peers) || !(c_obj->port))
        {
            free(c_obj);
            perror("Config is incomplete...");
            exit(EXIT_FAILURE);
        }

   return c_obj;
}