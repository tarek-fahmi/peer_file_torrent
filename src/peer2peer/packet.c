// Local Dependencies:
#include <pkgchk.h>
#include <btide.h>
#include <my_utils.h>
#include <config.h>
#include <peer.h>
#include <packet.h>
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


void packet_marshall(packet_t pkt_o){

    /* Marshall Primitive Attributes */

    uint8_t data_marshalled[PAYLOAD_MAX];
    uint8_t* curr_ptr = data_marshalled;
    packet_t pkt_o;

    memcpy(curr_ptr, pkt_o.msg_code, sizeof(pkt_o.msg_code));
    curr_ptr += sizeof(pkt_o.msg_code);

    memcpy(curr_ptr, pkt_o.error, sizeof(pkt_o.error));
    curr_ptr += sizeof(pkt_o.error);

    /* Marshall Payload */

    payload_t pl = pkt_o.payload;

    memcpy(curr_ptr, pl.offset, sizeof(pl.offset));
    curr_ptr += sizeof(pl.offset);

    memcpy(curr_ptr, pl.size, sizeof(pl.size));
    curr_ptr += sizeof(pl.size);

    memcpy(curr_ptr, pl.size, sizeof(pl.size));
    curr_ptr += sizeof(pl.size);

    memcpy(curr_ptr, pl.hash, sizeof(pl.hash));
    curr_ptr += sizeof(pl.hash);

    memcpy(curr_ptr, pl.ident, sizeof(pl.ident));
    curr_ptr += sizeof(pl.ident);

    memcpy(curr_ptr, pl.data, sizeof(pl.data));
    curr_ptr += sizeof(pl.data);

    return data_marshalled;
}

packet_t packet_unmarshall(const uint8_t *data_marshalled) {

    /* Unmarshall Pimitive Attributes */
    packet_t pkt_i;

    const uint8_t *curr_ptr = data_marshalled;

    memcpy(&pkt_i.msg_code, curr_ptr, sizeof(pkt_i.msg_code));
    curr_ptr += sizeof(pkt_i.msg_code);

    memcpy(&pkt_i.error, curr_ptr, sizeof(pkt_i.error));
    curr_ptr += sizeof(pkt_i.error);


    /* Unmarshall Payload */

    payload_t pl = pkt_i.payload;

    memcpy(pl.offset, curr_ptr, sizeof(pl.offset));
    curr_ptr += sizeof(pl.offset);

    memcpy(pl.size, curr_ptr, sizeof(pl.size));
    curr_ptr += sizeof(pl.size);

    memcpy(pl.hash, curr_ptr, sizeof(pl.hash));
    curr_ptr += sizeof(pl.hash);

    memcpy(pl.ident, curr_ptr, sizeof(pl.ident));
    curr_ptr += sizeof(pl.ident);

    curr_ptr += (sizeof(pl) - (sizeof(DATA_MAX) * sizeof(uint8_t)));
    memcpy(pl.data, curr_ptr, sizeof(pl.data));

}
