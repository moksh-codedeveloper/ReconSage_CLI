#ifndef TLS_ENC_DEC_H
#define TLS_ENC_DEC_H

#include <stdint.h>
#include <stddef.h>

/* --- Cryptographic Constants --- */
#define X25519_KEY_LEN      32
#define SHA256_HASH_LEN     32
#define CHACHA20_KEY_LEN    32
#define CHACHA20_NONCE_LEN  12
#define POLY1305_TAG_LEN    16

/* --- 1. Random Number Generation --- */
// Required for ClientHello random bytes and private key generation
int crypto_get_random_bytes(uint8_t *buffer, size_t length);

/* --- 2. X25519 Key Exchange --- */
// Generates your private/public keypair
int crypto_x25519_generate_keypair(uint8_t *public_key, uint8_t *private_key);

// Computes the shared secret using the server's public key
int crypto_x25519_compute_shared_secret(uint8_t *shared_secret,
                                        const uint8_t *my_private_key,
                                        const uint8_t *their_public_key);

/* --- 3. HKDF (RFC 5869) --- */
// Extracts a pseudo-random key from the raw shared secret
int crypto_hkdf_extract(uint8_t *prk,
                        const uint8_t *salt, size_t salt_len,
                        const uint8_t *ikm, size_t ikm_len);

// Expands the PRK into actual ChaCha keys and nonces
int crypto_hkdf_expand(uint8_t *out_key, size_t out_len,
                       const uint8_t *prk, size_t prk_len,
                       const uint8_t *info, size_t info_len);

/* --- 4. ChaCha20-Poly1305 (IETF Standard) --- */
// Encrypts plaintext and generates the MAC (authentication tag)
int crypto_chacha20_encrypt(uint8_t *ciphertext, uint8_t *mac_tag,
                            const uint8_t *plaintext, size_t pt_len,
                            const uint8_t *key, const uint8_t *nonce,
                            const uint8_t *aad, size_t aad_len);

// Decrypts ciphertext and verifies the MAC (drops if tampered)
int crypto_chacha20_decrypt(uint8_t *plaintext,
                            const uint8_t *ciphertext, size_t ct_len,
                            const uint8_t *mac_tag,
                            const uint8_t *key, const uint8_t *nonce,
                            const uint8_t *aad, size_t aad_len);

#endif // TLS_ENC_DEC_H