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


void pkt_marshall(pkt_t* pkt_o, uint8_t* data_marshalled) {
    uint8_t* curr_ptr = data_marshalled;

    memcpy(curr_ptr, &pkt_o->msg_code, sizeof(pkt_o->msg_code));  // Use the address of msg_code
    curr_ptr += sizeof(pkt_o->msg_code);

    memcpy(curr_ptr, &pkt_o->error, sizeof(pkt_o->error));  // Use the address of error
    curr_ptr += sizeof(pkt_o->error);

    payload_t* pl = pkt_o->payload;

    memcpy(curr_ptr, &pl->offset, sizeof(pl->offset));
    curr_ptr += sizeof(pl->offset);

    memcpy(curr_ptr, &pl->size, sizeof(pl->size));
    curr_ptr += sizeof(pl->size);

    memcpy(curr_ptr, pl->hash, sizeof(pl->hash));
    curr_ptr += sizeof(pl->hash);

    memcpy(curr_ptr, pl->ident, sizeof(pl->ident));
    curr_ptr += sizeof(pl->ident);

    memcpy(curr_ptr, pl->data, sizeof(pl->data));
}

void pkt_unmarshall(pkt_t* pkt_i, uint8_t* data_marshalled) {
    uint8_t* curr_ptr = data_marshalled;

    memcpy(&pkt_i->msg_code, curr_ptr, sizeof(pkt_i->msg_code));
    curr_ptr += sizeof(pkt_i->msg_code);

    memcpy(&pkt_i->error, curr_ptr, sizeof(pkt_i->error));
    curr_ptr += sizeof(pkt_i->error);

    memcpy(&pkt_i->payload->offset, curr_ptr, sizeof(pkt_i->payload->offset));
    curr_ptr += sizeof(pkt_i->payload->offset);

    memcpy(&pkt_i->payload->size, curr_ptr, sizeof(pkt_i->payload->size));
    curr_ptr += sizeof(pkt_i->payload->size);

    memcpy(pkt_i->payload->hash, curr_ptr, sizeof(pkt_i->payload->hash));
    curr_ptr += sizeof(pkt_i->payload->hash);

    memcpy(pkt_i->payload->ident, curr_ptr, sizeof(pkt_i->payload->ident));
    curr_ptr += sizeof(pkt_i->payload->ident);

    memcpy(pkt_i->payload->data, curr_ptr, sizeof(pkt_i->payload->data));
}

pkt_t* pkt_create(uint8_t msg, uint8_t err, payload_t* payload)
{
    pkt_t* pkt = (pkt_t*) my_malloc(sizeof(pkt_t));
    pkt->error = err;
    pkt->msg_code = msg;
    pkt->payload = payload;
}

payload_t* payload_create(uint32_t offset, uint16_t size, char* hash, char* ident, uint8_t* data)
{
    payload_t* pl = (payload_t*) my_malloc(sizeof(payload_t));

    pl->offset = offset;
    pl-> size = size;
    memcpy(pl->hash, hash, SHA256_HEXLEN);
    memcpy(pl->hash, hash, IDENT_MAX);
    memcpy(pl->data, data, DATA_MAX);
}

void payload_destroy(payload_t* payload)
{
    free(payload);
}

void pkt_destroy(pkt_t* pkt)
{
    payload_destroy(pkt->payload);
    free(pkt);
}