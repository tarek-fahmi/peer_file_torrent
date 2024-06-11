#ifndef CRYPT_SHA256_H
#define CRYPT_SHA256_H

#define SHA256_CHUNK_SZ (64)
#define SHA256_INT_SZ (8)

#include <stdint.h>
#include <tree/merkletree.h>

/**
 * @brief Structure to hold data for SHA-256 computation.
 */
struct sha256_compute_data {
    uint64_t data_size;                   ///< Size of the data processed
    uint32_t hcomps[SHA256_INT_SZ];       ///< Intermediate hash components
    uint8_t last_chunk[SHA256_CHUNK_SZ];  ///< Last chunk of data
    uint8_t chunk_size;                   ///< Size of the last chunk
};

/**
 * @brief Process a 64-byte chunk of data.
 * 
 * @param data Pointer to the SHA-256 computation data.
 * @param chunk 64-byte data chunk.
 */
void sha256_calculate_chunk(struct sha256_compute_data* data, uint8_t chunk[SHA256_CHUNK_SZ]);

/**
 * @brief Initialize the SHA-256 computation data.
 * 
 * @param data Pointer to the SHA-256 computation data.
 */
void sha256_compute_data_init(struct sha256_compute_data* data);

/**
 * @brief Update the SHA-256 computation with new data.
 * 
 * @param data Pointer to the SHA-256 computation data.
 * @param bytes Data to update with.
 * @param size Size of the data.
 */
void sha256_update(struct sha256_compute_data* data, void* bytes, uint32_t size);

/**
 * @brief Finalize the SHA-256 computation and produce the hash.
 * 
 * @param data Pointer to the SHA-256 computation data.
 * @param hash Output buffer for the hash.
 */
void sha256_finalize(struct sha256_compute_data* data, uint8_t hash[SHA256_INT_SZ]);

/**
 * @brief Output the computed hash as a hexadecimal string.
 * 
 * @param data Pointer to the SHA-256 computation data.
 * @param hexbuf Output buffer for the hexadecimal string.
 */
void sha256_output_hex(struct sha256_compute_data* data, char hexbuf[SHA256_CHUNK_SZ]);

/**
 * @brief Compute the SHA-256 hash for a Merkle tree leaf node.
 * 
 * @param leaf Pointer to the leaf node.
 */
void sha256_compute_chunk_hash(mtree_node_t* leaf);

/**
 * @brief Compute the SHA-256 hash for an internal Merkle tree node.
 * 
 * @param internal Pointer to the internal node.
 */
void sha256_compute_internal_hash(mtree_node_t* internal);

#endif