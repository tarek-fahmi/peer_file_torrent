#ifndef PEER_2_PEER_PKT_H
#define PEER_2_PEER_PKT_H

#include <chk/pkgchk.h>
#include <crypt/sha256.h>
#include "peer_2_peer/peer_data_sync.h"
#include <peer_2_peer/packet.h>

#define PAYLOAD_MAX (4096)
#define DATA_MAX (2998)
#define REQUESTS_MAX (1024)

#define PKT_MSG_ACK 0x0c
#define PKT_MSG_ACP 0x02
#define PKT_MSG_DSN 0x03
#define PKT_MSG_REQ 0x06
#define PKT_MSG_RES 0x07
#define PKT_MSG_PNG 0xFF
#define PKT_MSG_POG 0x00


/* Payload data structure. This is statically allocated and contains details of
** Chunk packet metadata and raw data.
*/
typedef struct {
    uint32_t offset;
    uint8_t data[DATA_MAX];
    uint16_t size;
    char hash[SHA256_HEXLEN];
    char ident[IDENT_MAX];
} __attribute__(( packed )) res_t;

typedef struct {
    uint32_t offset;
    uint8_t data[DATA_MAX];
    uint32_t size;
    char hash[SHA256_HEXLEN];
    char ident[IDENT_MAX - 2];
} __attribute__(( packed )) req_t;

typedef union payload_t {
    res_t res;
    req_t req;
}payload_t;

/* Packet data structure. This is dynamically allocated and contains details of
** Chunk packet metadata and raw data.
*/
typedef struct __attribute__(( packed )) pkt_t {
    uint16_t msg_code;
    uint16_t error;
    payload_t payload;
} __attribute__(( packed )) pkt_t;

/**
 * @brief Convert packet to byte array
 * @param pkt Pointer to the packet
 * @param data_marshalled Byte array to store marshalled data
 */
void pkt_marshall(pkt_t* pkt, uint8_t* data_marshalled);

/**
 * @brief Convert byte array back to packet
 * @param pkt_i Pointer to the packet
 * @param data_marshalled Byte array with marshalled data
 */
void pkt_unmarshall(pkt_t* pkt_i, uint8_t* data_marshalled);

/**
 * @brief Create a new request payload
 * @param msg Message code
 * @param err Error code
 * @param payload Packet payload
 * @return Pointer to the new packet
 */
payload_t payload_create_res(uint32_t offset, uint16_t size, char* hash, char* ident, uint8_t* data);

/**
 * @brief Create a new response payload
 * @param msg Message code
 * @param err Error code
 * @param payload Packet payload
 * @return Pointer to the new packet
 */
payload_t payload_create_req(uint32_t offset, uint32_t size, char* hash, char* ident, uint8_t* data);

/**
 * @brief Free packet memory
 * @param pkt Pointer to the packet
 */
void pkt_destroy(pkt_t* pkt);

/**
 * @brief Create a new packet
 * @param msg Message code
 * @param err Error code
 * @param payload Packet payload
 * @return Pointer to the new packet
 */
pkt_t* pkt_create(uint16_t msg, uint16_t err, payload_t payload);

#endif