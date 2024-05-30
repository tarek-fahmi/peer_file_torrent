#ifndef NETPKT_H
#define NETPKT_H

// Local Dependencies:
#include <pkgchk.h>
#include <btide.h>
#include <my_utils.h>
#include <config.h>
#include <peer.h>
// Standard Linux Dependencies:
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// Additional Linux Dependencies:
#include <string.h>
#include <pthread.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define PAYLOAD_MAX (4092)
#define DATA_MAX (2998)
#define REQUESTS_MAX (1024)

#define PKT_MSG_ACK 0x0c
#define PKT_MSG_ACP 0x02
#define PKT_MSG_DSN 0x03
#define PKT_MSG_REQ 0x06
#define PKT_MSG_RES 0x07
#define PKT_MSG_PNG 0xFF
#define PKT_MSG_POG 0x00

typedef struct payload{
    uint32_t offset;
    uint16_t size;
    char hash[SHA256_HEXLEN];
    char ident[IDENT_MAX];
    uint8_t data[DATA_MAX];
}payload_t;

typedef struct packet{
    uint16_t msg_code;
    uint16_t error;
    payload_t payload;
}packet_t;

/**
 * Struct which handles the shared memory for network requests
*/


uint8_t* packet_marshall(packet_t pkt_o);

packet_t packet_unmarshall(uint8_t *data_unmarshalled);







#endif
