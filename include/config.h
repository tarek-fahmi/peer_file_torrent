#ifndef CONFIG_H
#define CONFIG_H

// Local Dependencies:
#include <utilities/my_utils.h>
#include <stdint.h>

#define MAX_PEERS            (2048)
#define MAX_PORT             (65535)
#define MAX_LINE_LENGTH      (1024)
#define MAX_DIRECTORY_LENGTH (256)
#define MIN_PEERS            (1)
#define MIN_PORT             (1024)
#define ERR_DIRECTORY        (3)
#define ERR_PEERS            (4)
#define ERR_PORT             (5)

typedef struct config_object{
    char directory[MAX_DIRECTORY_LENGTH];
    uint16_t max_peers;
    uint32_t port;
}config_t;

static int parse_entry(char* line, config_t* c_obj);

static int check_directory(char* pathname);

int config_load(char* filename, config_t* c_obj);

#endif