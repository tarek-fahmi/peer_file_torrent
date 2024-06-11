#include <string.h> 
#include <peer_2_peer/packet.h>
#include <utilities/my_utils.h>
#include <stdlib.h>
#include <math.h>

#define PAYLOAD_MAX  (4096)
#define DATA_MAX (2998)
#define REQUESTS_MAX (1024)

#define PKT_MSG_ACK 0x0c
#define PKT_MSG_ACP 0x02
#define PKT_MSG_DSN 0x03
#define PKT_MSG_REQ 0x06
#define PKT_MSG_RES 0x07
#define PKT_MSG_PNG 0xFF
#define PKT_MSG_POG 0x00


/**
 * @brief Convert packet to byte array
 * @param pkt Pointer to the packet
 * @param data_marshalled Byte array to store marshalled data
 */
void pkt_marshall(pkt_t* pkt, uint8_t* data_marshalled) {
     size_t offset = 0;

     // Copy message code
     memcpy(data_marshalled + offset, &pkt->msg_code, sizeof(pkt->msg_code));
     offset += sizeof(pkt->msg_code);

     // Copy error code
     memcpy(data_marshalled + offset, &pkt->error, sizeof(pkt->error));
     offset += sizeof(pkt->error);

     if ( pkt->msg_code == PKT_MSG_REQ ) {
          // Copy payload offset
          memcpy(data_marshalled + offset, &pkt->payload.req.offset, sizeof(pkt->payload.req.offset));
          offset += sizeof(pkt->payload.req.offset);

          // Copy payload data
          memcpy(data_marshalled + offset, pkt->payload.req.data, sizeof(pkt->payload.req.data));
          offset += sizeof(pkt->payload.req.data);

          // Copy payload size
          memcpy(data_marshalled + offset, &pkt->payload.req.size, sizeof(pkt->payload.req.size));
          offset += sizeof(pkt->payload.req.size);

          // Copy payload identifier
          memcpy(data_marshalled + offset, pkt->payload.req.ident, sizeof(pkt->payload.req.ident));
          offset += sizeof(pkt->payload.req.ident);

          // Copy payload hash
          memcpy(data_marshalled + offset, pkt->payload.req.hash, sizeof(pkt->payload.req.hash));
     }
     else {
          // Copy payload offset
          memcpy(data_marshalled + offset, &pkt->payload.res.offset, sizeof(pkt->payload.res.offset));
          offset += sizeof(pkt->payload.res.offset);

          // Copy payload data
          memcpy(data_marshalled + offset, pkt->payload.res.data, sizeof(pkt->payload.res.data));
          offset += sizeof(pkt->payload.res.data);

          // Copy payload size
          memcpy(data_marshalled + offset, &pkt->payload.res.size, sizeof(pkt->payload.res.size));
          offset += sizeof(pkt->payload.res.size);

          // Copy payload identifier
          memcpy(data_marshalled + offset, pkt->payload.res.ident, sizeof(pkt->payload.res.ident));
          offset += sizeof(pkt->payload.res.ident);

          // Copy payload hash
          memcpy(data_marshalled + offset, pkt->payload.res.hash, sizeof(pkt->payload.res.hash));
     }
}
/** @brief Convert byte array back to packet
* @param pkt_i Pointer to the packet
* @param data_marshalled Byte array with marshalled data
**/

void pkt_unmarshall(pkt_t* pkt_i, uint8_t* data_marshalled) {
     size_t offset = 0;

     // Extract message code
     memcpy(&pkt_i->msg_code, data_marshalled + offset, sizeof(pkt_i->msg_code));
     offset += sizeof(pkt_i->msg_code);

     // Extract error code
     memcpy(&pkt_i->error, data_marshalled + offset, sizeof(pkt_i->error));
     offset += sizeof(pkt_i->error);

     if ( pkt_i->msg_code == PKT_MSG_REQ ) {
          // Extract payload offset
          memcpy(&pkt_i->payload.req.offset, data_marshalled + offset, sizeof(pkt_i->payload.req.offset));
          offset += sizeof(pkt_i->payload.req.offset);

          // Extract payload data
          memcpy(pkt_i->payload.req.data, data_marshalled + offset, sizeof(pkt_i->payload.req.data));
          offset += sizeof(pkt_i->payload.req.data);

          // Extract payload size
          memcpy(&pkt_i->payload.req.size, data_marshalled + offset, sizeof(pkt_i->payload.req.size));
          offset += sizeof(pkt_i->payload.req.size);

          // Extract payload identifier
          memcpy(pkt_i->payload.req.ident, data_marshalled + offset, sizeof(pkt_i->payload.req.ident));
          offset += sizeof(pkt_i->payload.req.ident);

          // Extract payload hash
          memcpy(pkt_i->payload.req.hash, data_marshalled + offset, sizeof(pkt_i->payload.req.hash));
     }
     else {
          // Extract payload offset
          memcpy(&pkt_i->payload.res.offset, data_marshalled + offset, sizeof(pkt_i->payload.res.offset));
          offset += sizeof(pkt_i->payload.res.offset);

          // Extract payload data
          memcpy(pkt_i->payload.res.data, data_marshalled + offset, sizeof(pkt_i->payload.res.data));
          offset += sizeof(pkt_i->payload.res.data);

          // Extract payload size
          memcpy(&pkt_i->payload.res.size, data_marshalled + offset, sizeof(pkt_i->payload.res.size));
          offset += sizeof(pkt_i->payload.res.size);

          // Extract payload identifier
          memcpy(pkt_i->payload.res.ident, data_marshalled + offset, sizeof(pkt_i->payload.res.ident));
          offset += sizeof(pkt_i->payload.res.ident);

          // Extract payload hash
          memcpy(pkt_i->payload.res.hash, data_marshalled + offset, sizeof(pkt_i->payload.res.hash));
     }
}
/**
 * @brief Create a new packet
 * @param msg Message code
 * @param err Error code
 * @param payload Packet payload
 * @return Pointer to the new packet
 */
pkt_t* pkt_create(uint16_t msg, uint16_t err, payload_t payload) {
     pkt_t* pkt = (pkt_t*)my_malloc(sizeof(pkt_t));
     if ( !pkt ) {
          return NULL;
     }
     memset(pkt, 0, sizeof(pkt_t)); // Zero out the memory
     pkt->msg_code = msg;
     pkt->error = err;
     pkt->payload = payload;
     return pkt;
}

/**
 * @brief Create a new response payload
 * @param offset Data offset
 * @param size Data size
 * @param hash Hash string
 * @param ident Identifier string
 * @param data Pointer to data
 * @return New payload
 */
payload_t payload_create_res(uint32_t offset, uint16_t size, char* hash, char* ident, uint8_t* data) {
     payload_t pl;
     memset(&pl, 0, sizeof(payload_t)); // Default payload content is 0.
     pl.res.offset = offset;
     pl.res.size = size;
     if ( hash ) {
          strncpy(pl.res.hash, hash, SHA256_HEXLEN);
     }
     if ( ident ) {
          strncpy(pl.res.ident, ident, IDENT_MAX);
     }
     if ( data && size <= DATA_MAX ) {
          memcpy(pl.res.data, data, size);
     }
     return pl;
}

/**
 * @brief Create a new request payload
 * @param offset Data offset
 * @param size Data size
 * @param hash Hash string
 * @param ident Identifier string
 * @param data Pointer to data
 * @return New payload
 */
payload_t payload_create_req(uint32_t offset, uint32_t size, char* hash, char* ident, uint8_t* data) {
     payload_t pl;
     memset(&pl, 0, sizeof(payload_t)); // Default payload content is 0.
     pl.req.offset = offset;
     pl.req.size = size;
     if ( hash ) {
          strncpy(pl.req.hash, hash, SHA256_HEXLEN);
     }
     if ( ident ) {
          strncpy(pl.req.ident, ident, IDENT_MAX - 2);
     }
     if ( data && size <= DATA_MAX ) {
          memcpy(pl.req.data, data, size);
     }
     return pl;
}

/**
 * @brief Free packet memory
 * @param pkt Pointer to the packet
 */
void pkt_destroy(pkt_t* pkt) {
     if ( pkt ) {
          free(pkt);
     }
}

