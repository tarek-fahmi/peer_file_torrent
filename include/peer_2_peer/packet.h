#ifndef PEER_2_PEER_pkt_H
#define PEER_2_PEER_pkt_H

#include <crypt/sha256.h>
#include <peer_2_peer/peer_data_sync.h>
#include <chk/pkgchk.h>

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

typedef struct{
    uint16_t msg_code;
    uint16_t error;
    payload_t* payload;
}pkt_t;


void pkt_marshall(pkt_t* pkt, uint8_t* data_marshalled);

void pkt_unmarshall(pkt_t* pkt_i, uint8_t *data_marshalled);

pkt_t* pkt_create(uint8_t msg, uint8_t err, payload_t* payload);

void pkt_destroy(pkt_t* pkt);

payload_t* payload_create(uint32_t offset, uint16_t size, char* hash, char* ident, uint8_t* data);

void payload_destroy(payload_t* payload);



#endif