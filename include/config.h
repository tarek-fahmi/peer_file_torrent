// Local Dependencies:
#include <btide.h>
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

#define MAX_PEERS (2048)
#define MAX_PORT  (65535)
#define MAX_LINE_LENGTH (1024)
#define MAX_DIRECTORY_LENGTH (256)

#define MIN_PEERS (1)
#define MIN_PORT      (1024)


#define ERR_DIRECTORY (3)
#define ERR_PEERS (4)
#define ERR_PORT      (5)

//
// PART 2
//


typedef struct config_object{
    char directory[MAX_DIRECTORY_LENGTH];
    uint16_t max_peers;
    uint32_t port;
}config_t;



/**
 * @brief: Loads the config object so that it can later be accessed by the program.
 * 
 * @param config_filename: A string containing the name of the config name. Will be passed in 
 *       as the first additional argument when executing btide.
 * 
 * @return config_t: A config object containing the extracted information.
*/
config_t* config_load(char* config_filename, config_t* c_obj);


/**
 * @brief Parse the next argument in the config file, storing it in the config object.
 *      Checks if all config values are within allowed ranges, else exists with appropriate code.
 * 
 * @param start_ptr: A pointer to the start of the next config object to parse...
 * @param c_obj: Config object, of which to modify appropriate attribute as required...
*/
int parse_entry(char* line, config_t* c_obj);

/**
 * @brief Check and load the directory specified in the cofig file ifi 
 * @param start_ptr: A pointer to the start of the next config object to parse.
*/
int check_directory(char* path);


