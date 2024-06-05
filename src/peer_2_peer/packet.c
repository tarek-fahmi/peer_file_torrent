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


void pkt_marshall(pkt_t* pkt_o, uint8_t* data_marshalled){
    uint8_t* curr_ptr = data_marshalled;
    pkt_t pkt_o = *pkt;

    memcpy(curr_ptr, pkt_o->msg_code, sizeof(pkt_o->msg_code));
    curr_ptr += sizeof(pkt_o->msg_code);

    memcpy(curr_ptr, pkt_o->error, sizeof(pkt_o->error));
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


void pkt_unmarshall(pkt_t* pkt_i, uint8_t *data_marshalled) {
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

void pkt_destroy(pkt_t* pkt)
{
    payload_destroy(pkt->payload);
    free(pkt);
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

void process_pkt_in(peer_t* peer, pkt_t* pkt_in, bpkg_t* bpkgs, request_t* req_recent)
{
    int err = 0;
    switch(pkt_in->msg_code)
    {
        case PKT_MSG_PNG:
            send_pog(peer);
            break;

        case PKT_MSG_ACP:
            send_ack(peer);
            break;

        case PKT_MSG_REQ:
            payload_t* payload = payload_get_res_for_req(pkt_in->payload, bpkgs);
            if (payload == NULL); err = -1;

            send_res(peer, err, payload);
            debug_print("Send P[%d] the requested chunk if exists...", peer->port);
            break;

        case PKT_MSG_DSN:
            send_dsn(peer);
            pthread_exit((void* )0);
            break;

        case PKT_MSG_RES:

            if (pkg_install(pkt_in, peer, bpkgs) < 0)
            {
                req_recent->status = FAILED;
                debug_print("Failed to install pkt...\n");
                break;
            }

            req_recent->status = SUCCESS;
            pthread_cond_signal(&req_recent->cond);
            break;
   
        default:
            break;
    }
}

void process_pkt_out(peer_t* peer, pkt_t* pkt)
{
    switch(pkt->msg_code){
        case PKT_MSG_PNG:
            send_png(peer);
            break;
        case PKT_MSG_REQ:
            send_req(peer, pkt);
            break;
        case PKT_MSG_DSN:
            send_dsn(peer);
            pthread_cancel(&peer->thread);
            pthread_join(&peer->thread, NULL);
            peer = NULL;
            break;
        default:
            break;
    }
}