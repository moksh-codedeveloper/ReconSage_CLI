#ifndef TLS_MAIN_ENGINE_H
#define TLS_MAIN_ENGINE_H

#include <stdint.h>
#include <stddef.h>
#include "tls_struct.h"

#define MAX_TLS_RECORD_SIZE 16384 // TLS max record limit
#define HELLO_BUFFER_SIZE 512     // Plenty of room for our ClientHello

/*
 * Initializes the socket and starts the handshake state machine.
 */
int start_tls_handshake(int socket_fd, const char *target_hostname);

/*
 * Constructs and transmits the ClientHello packet.
 */
int send_client_hello(int socket_fd, const char *hostname, uint8_t *my_private_key);

int receive_server_hello(int socket_fd, uint8_t *server_public_key);

#endif // TLS_MAIN_ENGINE_H