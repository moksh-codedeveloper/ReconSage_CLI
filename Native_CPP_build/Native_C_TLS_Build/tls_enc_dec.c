#include "tls_enc_dec.h"
#include <stdio.h>
#include <sodium.h>

// Should be called once when the tls engine initialises or starts.....
int init_crypto_engine(){
    if(sodium_init() < 0){
        printf("[-] FATAL: Failed to initialize libsodium\n");
        return -1;
    }
    return 0;
}

/* --- 1. Random Number Generation --- */
int crypto_get_random_bytes(uint8_t *buff, size_t length){
    randombytes_buf(buff, length);
    return 0;
}

/* --- 2. X25519 Key Exchange --- */
int crypto_x25519_generate_keypair(uint8_t *public_key, uint8_t *private_key){
    randombytes_buf(private_key, X25519_KEY_LEN);
    if(crypto_scalarmult_base(public_key, private_key) != 0){
        return -1;
    }
    return 0;
}

int crypto_x25519_compute_shared_secret(uint8_t *shared_secret, 
                                        const uint8_t *my_private_key, 
                                        const uint8_t *their_public_key){
    if(crypto_scalarmult(shared_secret, my_private_key, their_public_key) != 0){
        return -1;
    }
    return 0;
}

/* --- 3. HKDF (RFC 5869) --- */
int crypto_hkdf_extract(uint8_t *prk, const uint8_t *salt, size_t salt_len, const uint8_t *ikm, size_t ikm_len) {
    if (crypto_kdf_hkdf_sha256_extract(prk, salt, salt_len, ikm, ikm_len) != 0) {
        return -1;
    }
    return 0;
}

int crypto_hkdf_expand(uint8_t *out_key, size_t out_len, const uint8_t *prk, size_t prk_len, const uint8_t *info, size_t info_len) {
    if (crypto_kdf_hkdf_sha256_expand(out_key, out_len, (const char*)info, info_len, prk) != 0) {
        return -1;
    }
    return 0;
}

/* --- 4. ChaCha20-Poly1305 (IETF Standard) --- */
int crypto_chacha20_encrypt(uint8_t *ciphertext, uint8_t *mac_tag,
                            const uint8_t *plaintext, size_t pt_len,
                            const uint8_t *key, const uint8_t *nonce,
                            const uint8_t *aad, size_t aad_len) {
    
    if (crypto_aead_chacha20poly1305_ietf_encrypt_detached(
            ciphertext, mac_tag, NULL, 
            plaintext, pt_len, 
            aad, aad_len, 
            NULL, nonce, key) != 0) {
        return -1;
    }
    return 0;
}

int crypto_chacha20_decrypt(uint8_t *plaintext,
                            const uint8_t *ciphertext, size_t ct_len,
                            const uint8_t *mac_tag,
                            const uint8_t *key, const uint8_t *nonce,
                            const uint8_t *aad, size_t aad_len) {
    if (crypto_aead_chacha20poly1305_ietf_decrypt_detached(
            plaintext, NULL, 
            ciphertext, ct_len, mac_tag, 
            aad, aad_len, 
            nonce, key) != 0) {
        return -1; // MAC verification failed - tampered data!
    }
    return 0;
}