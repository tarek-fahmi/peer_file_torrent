#include <cli.h>
#include <config.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <utilities/my_utils.h>

int check_directory(char* pathname) {
     struct stat statbuf;
     if (stat(pathname, &statbuf) == 0) {
          if (S_ISDIR(statbuf.st_mode)) {
               debug_print("Directory already exists.\n");
               return 0;  // Directory already exists
          } else {
               fprintf(stderr, "%s is not a directory.\n", pathname);
               return ERR_DIRECTORY;  // Path exists but is not a directory
          }
     } else {
          if (errno == ENOENT) {  // Directory does not exist
               if (mkdir(pathname, 0755) == -1) {
                    perror("Failed to create directory");
                    return ERR_DIRECTORY;  // Failed to create directory
               }
               debug_print("Created directory.\n");
               return 0;  // Successfully created the directory
          } else {
               perror("Failed to check directory");
               return ERR_DIRECTORY;  // Other errors when trying to stat the
                                      // directory
          }
     }
}

int parse_entry(char* line, config_t* c_obj) {
     char* key = strtok(line, ":");
     char* value = strtok(NULL, "\n");

     if (key == NULL || value == NULL) {
          return -1;  // Parsing error
     }

     if (strcmp(key, "directory") == 0) {
          strncpy(c_obj->directory, value, sizeof(c_obj->directory) - 1);
          c_obj->directory[sizeof(c_obj->directory) - 1] =
              '\0';  // Ensure null-termination
          return check_directory(c_obj->directory);
     } else if (strcmp(key, "max_peers") == 0) {
          int max_peers = atoi(value);
          if (max_peers < MIN_PEERS || max_peers > MAX_PEERS) {
               fprintf(stderr,
                       "Max peers (%d) outside of permitted range (%d - %d)\n",
                       max_peers, MIN_PEERS, MAX_PEERS);
               return ERR_PEERS;
          }
          c_obj->max_peers = max_peers;
     } else if (strcmp(key, "port") == 0) {
          int port = atoi(value);
          if (port < MIN_PORT || port > MAX_PORT) {
               fprintf(stderr,
                       "Port (%d) outside of permitted range (%d - %d)\n", port,
                       MIN_PORT, MAX_PORT);
               return ERR_PORT;
          }
          c_obj->port = port;
     } else {
          return -1;  // Unknown configuration key
     }

     return 0;
}

config_t* config_load(char* filename) {
     config_t* c_obj = (config_t*)my_malloc(sizeof(config_t));
     FILE* f_ptr = fopen(filename, "r");
     if (!f_ptr) {
          perror("Failed to open config");
          free(c_obj);
          return NULL;
     }

     char buffer[1024];
     while (fgets(buffer, sizeof(buffer), f_ptr)) {
          int parse_code = parse_entry(buffer, c_obj);
          if (parse_code != 0) {
               fclose(f_ptr);
               free(c_obj);
               return NULL;
          }
     }

     fclose(f_ptr);

     // Check if a config entry is missing.
     if (!(c_obj->directory[0]) || !(c_obj->max_peers) || !(c_obj->port)) {
          fprintf(stderr, "Config is incomplete\n");
          free(c_obj);
          return NULL;
     }
     debug_print("Config is completed and has been stored in memory...");
     return c_obj;
}
