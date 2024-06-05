#include <crypt/sha256.h>
#include <tree/merkletree.h>
#include <chk/pkgchk.h>
#include <chk/pkg_helper.h>
#include <utilities/my_utils.h>
#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/peer_server.h>
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

void* server_thread_handler(void* args_void) {
    server_thr_args_t* args = (server_thr_args_t*)args_void;
    p2p_server_listening(args->server_fd, args->server_port, args->reqs_q, args->peers);
    return NULL;
}

void create_p2p_server_thread(int server_fd, int server_port, request_q_t* reqs_q, peers_t* peers, config_t* config) {
    pthread_t server_thread;
    server_thr_args_t* args = (server_thr_args_t*)malloc(sizeof(server_thr_args_t));

    args->server_fd = server_fd;
    args->server_port = server_port;
    args->reqs_q = reqs_q;
    args->peers = peers;
    args->config = config;

    // Create the thread
    int result = pthread_create(&server_thread, NULL, server_thread_handler, args);
    if (result != 0) {
        perror("Server thread creation failed\n");
        free(args);
        exit(EXIT_FAILURE);
    }

    // Detach the thread to allow it to clean up after itself when it finishes
    result = pthread_detach(server_thread);
    if (result != 0) {
        perror("Failed to detach server thread\n");
        free(args);
        exit(EXIT_FAILURE);
    }

    debug_print("Server thread created successfully\n");
}

int p2p_setup_server(uint16_t port) {
    errno = 0;
    int server_sock_fd;

    if ((server_sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation for new peer failed...\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port),
    };

    if (bind(server_sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind to new peer failed...\n");
        close(server_sock_fd);
        exit(EXIT_FAILURE);
    }
    debug_print("Server successful...");

    if (listen(server_sock_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    debug_print("Server listening...\n");
    debug_print("Successfully connected to new port...\n");
    return server_sock_fd;
}

void* server_thread_handler(void* args_void) {
    server_thr_args_t* args = (server_thr_args_t*)args_void;
    p2p_server_listening(args->server_fd, args->server_port, args->reqs_q, args->peers);
    return NULL;
}

void create_server_thread(int server_fd, int server_port, request_q_t* reqs_q, peers_t* peers) {
    pthread_t server_thread;
    server_thr_args_t* args = (server_thr_args_t*)my_malloc(sizeof(server_thr_args_t));

    args->server_fd = server_fd;
    args->server_port = server_port;
    args->reqs_q = reqs_q;
    args->peers = peers;

    // Create the thread
    int result = pthread_create(&server_thread, NULL, server_thread_handler, args);
    if (result != 0) {
        perror("Server thread creation failed"\n);
        free(args);
        exit(EXIT_FAILURE);
    }

    // Detach the thread to allow it to clean up after itself when it finishes
    result = pthread_detach(server_thread);
    if (result != 0) {
        perror("Failed to detach server thread\n");
        free(args);
        exit(EXIT_FAILURE);
    }

    debug_print("Server thread created successfully\n");
}

void p2p_server_listening(int server_fd, int server_port, request_q_t* reqs_q, peers_t* peers) {
    while (1) {
        printf("waiting for connections\n");
        struct sockaddr_in peer_addr;
        socklen_t addrlen = sizeof(peer_addr);

        int new_sock_fd = accept(server_fd, (struct sockaddr *)&peer_addr, &addrlen);
        check_err(new_sock_fd, "Failed to accept connection\n");

        peer_t* new_peer = (peer_t*)my_malloc(sizeof(peer_t));
        peer_init(new_peer, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));
        new_peer->sock_fd = new_sock_fd;

        debug_print("Connected to new peer successfully!\n");
        peer_create_thread(new_peer, reqs_q, peers);
    }
}
