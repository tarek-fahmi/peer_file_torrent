#include <chk/pkg_helper.h>
#include <chk/pkgchk.h>
#include <cli.h>
#include <config.h>
#include <crypt/sha256.h>
#include <peer_2_peer/package.h>
#include <peer_2_peer/packet.h>
#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/peer_handler.h>
#include <tree/merkletree.h>
#include <utilities/my_utils.h>

// Standard Linux Dependencies:
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Additional Linux Dependencies:
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>

int check_directory(char* pathname) {
     DIR* dir = opendir(pathname);

     if (dir) {
          closedir(dir);
          return 0;
     } else if (errno == ENOENT || errno == ENOTDIR) {
          if (mkdir(pathname, 0755)) {
               perror("Failed to create config directory");
               return ERR_DIRECTORY;
          }
     } else {
          perror("Failed to open config directory");
          return ERR_DIRECTORY;
     }

     return 0;
}

int parse_entry(char* line, config_t* c_obj) {
     if (sscanf(line, "directory:%256s", c_obj->directory)) {
          return check_directory(c_obj->directory);
     } else if (sscanf(line, "max_peers:%i", &c_obj->max_peers)) {
          if (c_obj->max_peers > MAX_PEERS || c_obj->max_peers < MIN_PEERS) {
               fprintf(stderr,
                       "Max peers (%d) outside of permitted range (%d - %d)\n",
                       c_obj->max_peers, MIN_PEERS, MAX_PEERS);
               return ERR_PEERS;
          }
     } else if (sscanf(line, "port:%i", &c_obj->port)) {
          if (c_obj->port > MAX_PORT || c_obj->port < MIN_PORT) {
               fprintf(stderr,
                       "Port (%d) outside of permitted range (%d - %d)\n",
                       c_obj->port, MIN_PORT, MAX_PORT);
               return ERR_PORT;
          }
     } else {
          return 0;
     }

     return 0;
}

config_t* config_load(char* filename) {
     config_t* c_obj = (config_t*)my_malloc(sizeof(config_t));
     check_directory(filename);
     FILE* f_ptr = fopen(filename, "r");
     if (!f_ptr) {
          perror("Failed to open config");
          return NULL;
     }

     char buffer[1024];
     while (fgets(buffer, sizeof(buffer), f_ptr)) {
          int parse_code = parse_entry(buffer, c_obj);
          if (parse_code != 0) {
               fclose(f_ptr);
               return NULL;
          }
     }

     fclose(f_ptr);

     // Check if a config entry is missing.
     if (!(c_obj->directory[0]) || !(c_obj->max_peers) || !(c_obj->port)) {
          fprintf(stderr, "Config is incomplete\n");
          return NULL;
     }
     return c_obj;
}
