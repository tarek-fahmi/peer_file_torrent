#ifndef CONFIG_H
#define CONFIG_H

// Local Dependencies:
#include <stdint.h>
#include <utilities/my_utils.h>

// Constants
#define MAX_PEERS (2048)             // Maximum number of peers
#define MAX_PORT (65535)             // Maximum port number
#define MAX_LINE_LENGTH (1024)       // Maximum length of a line in the configuration file
#define MAX_DIRECTORY_LENGTH (256)   // Maximum length of the directory path
#define MIN_PEERS (1)                // Minimum number of peers
#define MIN_PORT (1024)              // Minimum port number

// Error Codes
#define ERR_DIRECTORY (3)            // Error code for directory errors
#define ERR_PEERS (4)                // Error code for peers errors
#define ERR_PORT (5)                 // Error code for port errors

/**
 * @brief Structure to hold configuration data.
 */
typedef struct config_object {
     char directory[MAX_DIRECTORY_LENGTH];  // Directory path
     uint32_t max_peers;                    // Maximum number of peers
     uint32_t port;                         // Port number
} config_t;

/**
 * @brief Loads the configuration from a file.
 * 
 * @param filename Name of the configuration file.
 * @return Pointer to the loaded configuration object, or NULL if loading failed.
 */
config_t* config_load(char* filename);

/**
 * @brief Parses a single line of the configuration file.
 * 
 * @param line The line to parse.
 * @param c_obj Pointer to the configuration object to populate.
 * @return 0 on success, non-zero on failure.
 */
int parse_entry(char* line, config_t* c_obj);

/**
 * @brief Checks if the specified directory is valid.
 * 
 * @param pathname Path to the directory.
 * @return 0 if the directory is valid, non-zero if invalid.
 */
int check_directory(char* pathname);

#endif // CONFIG_H