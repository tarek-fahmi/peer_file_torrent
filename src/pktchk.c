#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <peer_2_peer/package.h>
#include <peer_2_peer/packet.h>
#include <crypt/sha256.h>

#define TEST_PKG_FILE "test.pkt"

void create_test_response_packet(const char* bpkg_filepath) {
    // Load the bpkg file
    bpkg_t* bpkg = bpkg_load(bpkg_filepath);
    if ( !bpkg ) {
        fprintf(stderr, "Failed to load bpkg file: %s\n", bpkg_filepath);
        exit(EXIT_FAILURE);
    }

    mtree_node_t* chk_node = bpkg->mtree->chk_nodes[6];
    chunk_t* chk = chk_node->chunk;
    payload_t payload = payload_create(chk->offset, chk->size, chk_node->expected_hash, bpkg->ident, chk->data);
    pkt_t* pkt = pkt_create(PKT_MSG_RES, 0, payload);

    // Print packet data
    printf("Packet Data:\n");
    printf("Message Code: %u\n", pkt->msg_code);
    printf("Error Code: %u\n", pkt->error);
    printf("Payload Offset: %u\n", pkt->payload.offset);
    printf("Payload Size: %u\n", pkt->payload.size);
    printf("Payload Hash: %s\n", pkt->payload.hash);
    printf("Payload Identifier: %s\n", pkt->payload.ident);
    printf("Payload Data: %s\n", pkt->payload.data);

    uint8_t buffer[sizeof(pkt_t)];
    pkt_marshall(pkt, buffer);

    // Write the buffer to the test file
    FILE* file = fopen(TEST_PKG_FILE, "wb");
    if ( !file ) {
        perror("Failed to open test file");
        exit(EXIT_FAILURE);
    }
    fwrite(buffer, sizeof(uint8_t), sizeof(buffer), file);
    fclose(file);

    // Clean up
    pkt_destroy(pkt);
    bpkg_obj_destroy(bpkg);

    printf("Test response packet created and written to %s\n", TEST_PKG_FILE);
}

int main(int argc, char* argv[]) {
    if ( argc != 2 ) {
        fprintf(stderr, "Usage: %s <bpkg_filepath>\n", argv[0]);
    }

    create_test_response_packet(argv[1]);

    return EXIT_SUCCESS;
}