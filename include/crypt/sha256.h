#ifndef BTYDE_CRYPT_SHA256
#define BTYDE_CRYPT_SHA256

// Local Dependencies:=
#include <crypt/sha256.h>
#include <tree/merkletree.h>
#include <chk/pkgchk.h>
#include <chk/pkg_helper.h>
#include <utilities/my_utils.h>
#include <peer_2_peer/peer_handler.h>
#include <peer_2_peer/peer_server.h>
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


#define SHA256_CHUNK_SZ (64)
#define SHA256_INT_SZ (8)
#define SHA256_DFTLEN (1024)

//Original: https://github.com/LekKit/sha256/blob/master/sha256.h
struct sha256_compute_data {
	uint64_t data_size;
	uint32_t hcomps[SHA256_INT_SZ];
	uint8_t last_chunk[SHA256_CHUNK_SZ];
	uint8_t chunk_size;
};

void sha256_calculate_chunk(struct sha256_compute_data* data,
		uint8_t chunk[SHA256_CHUNK_SZ]);

void sha256_compute_data_init(struct sha256_compute_data* data);

void sha256_update(struct sha256_compute_data* data,
		void* bytes, uint32_t size); 

void sha256_finalize(struct sha256_compute_data* data, 
		uint8_t hash[SHA256_INT_SZ]);


void sha256_output_hex(struct sha256_compute_data* data, 
		char hexbuf[SHA256_CHUNK_SZ]);

void sha256_compute_chunk_hash(mtree_node_t* leaf);

void sha256_compute_internal_hash(mtree_node_t* internal)

#endif

