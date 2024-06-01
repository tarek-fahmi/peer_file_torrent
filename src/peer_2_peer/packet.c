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
// Standard Linux Dependencies:
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
// Additional Linux Dependencies:
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/select.h>

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


void packet_marshall(packet_t* packet, uint8_t* data_marshalled){
    uint8_t* curr_ptr = data_marshalled;
    packet_t pkt_o = *packet;

    memcpy(curr_ptr, &pkt_o.msg_code, sizeof(pkt_o.msg_code));
    curr_ptr += sizeof(pkt_o.msg_code);

    memcpy(curr_ptr, &pkt_o.error, sizeof(pkt_o.error));
    curr_ptr += sizeof(pkt_o.error);

    payload_t pl = pkt_o.payload;

    memcpy(curr_ptr, &pl.offset, sizeof(pl.offset));
    curr_ptr += sizeof(pl.offset);

    memcpy(curr_ptr, &pl.size, sizeof(pl.size));
    curr_ptr += sizeof(pl.size);

    memcpy(curr_ptr, pl.hash, sizeof(pl.hash));
    curr_ptr += sizeof(pl.hash);

    memcpy(curr_ptr, pl.ident, sizeof(pl.ident));
    curr_ptr += sizeof(pl.ident);

    memcpy(curr_ptr, pl.data, sizeof(pl.data));
}


void packet_unmarshall(packet_t* pkt_i, uint8_t *data_marshalled) {
    uint8_t* curr_ptr = data_marshalled;

    memcpy(&pkt_i->msg_code, curr_ptr, sizeof(pkt_i->msg_code));
    curr_ptr += sizeof(pkt_i->msg_code);

    memcpy(&pkt_i->error, curr_ptr, sizeof(pkt_i->error));
    curr_ptr += sizeof(pkt_i->error);

    memcpy(&pkt_i->payload.offset, curr_ptr, sizeof(pkt_i->payload.offset));
    curr_ptr += sizeof(pkt_i->payload.offset);

    memcpy(&pkt_i->payload.size, curr_ptr, sizeof(pkt_i->payload.size));
    curr_ptr += sizeof(pkt_i->payload.size);

    memcpy(pkt_i->payload.hash, curr_ptr, sizeof(pkt_i->payload.hash));
    curr_ptr += sizeof(pkt_i->payload.hash);

    memcpy(pkt_i->payload.ident, curr_ptr, sizeof(pkt_i->payload.ident));
    curr_ptr += sizeof(pkt_i->payload.ident);

    memcpy(pkt_i->payload.data, curr_ptr, sizeof(pkt_i->payload.data));
}


