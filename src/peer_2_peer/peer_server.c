#include <peer_2_peer/package.h>
#include <peer_2_peer/packet.h>
#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/peer_handler.h>
#include <peer_2_peer/peer_server.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int p2p_setup_server(uint16_t port) {
     errno = 0;
     int server_sock_fd;

     if ( ( server_sock_fd = socket(AF_INET, SOCK_STREAM, 0) ) == -1 ) {
          perror("Socket creation for new peer failed...\n");
          exit(EXIT_FAILURE);
     }

     struct sockaddr_in server_addr = {
         .sin_family = AF_INET,
         .sin_addr.s_addr = INADDR_ANY,
         .sin_port = htons(port),
     };

     if ( bind(server_sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0 ) {
          perror("Bind to new peer failed...\n");
          close(server_sock_fd);
          exit(EXIT_FAILURE);
     }
     debug_print("Server successfully bound to: port: %d\n", port);

     if ( listen(server_sock_fd, 3) < 0 ) {
          perror("Listen failed");
          close(server_sock_fd);
          exit(EXIT_FAILURE);
     }

     debug_print("Server listening on port: %d\n", port);
     return server_sock_fd;
}

void* server_thread_handler(void* args_void) {
     debug_print("Server thread handler started\n");
     p2p_server_listening(args_void);
     return NULL;
}

void create_p2p_server_thread(int server_fd, int server_port, pthread_t* server_thread, peers_t* peers, bpkgs_t* bpkgs) {
     server_thr_args_t* args = (server_thr_args_t*)my_malloc(sizeof(server_thr_args_t));
     if ( !args ) {
          perror("Failed to allocate memory for server_thr_args_t");
          exit(EXIT_FAILURE);
     }

     args->server_fd = server_fd;
     args->server_port = server_port;
     args->peers = peers;
     args->bpkgs = bpkgs;

     int result = pthread_create(server_thread, NULL, server_thread_handler, args);
     if ( result != 0 ) {
          perror("Server thread creation failed\n");
          free(args);
          exit(EXIT_FAILURE);
     }

     debug_print("Server thread created successfully on port: %d\n", server_port);
}

void server_thread_cleanup(void* arg) {
     server_thr_args_t* args = (server_thr_args_t*)arg;
     if ( args == NULL ) {
          debug_print("Cleanup handler received a NULL argument.\n");
          return;
     }

     if ( args->server_fd >= 0 ) {
          if ( close(args->server_fd) == 0 ) {
               debug_print("Socket closed successfully.\n");
          }
          else {
               perror("Failed to close socket\n");
          }
     }
     debug_print("Cleanup completed Server.\n");
     free(arg);
}

void p2p_server_listening(void* arg) {
     server_thr_args_t* args = (server_thr_args_t*)arg;
     int server_fd = args->server_fd;
     peers_t* peers = args->peers;
     bpkgs_t* bpkgs = args->bpkgs;

     pthread_cleanup_push(server_thread_cleanup, args);
     while ( true ) {
          debug_print("Server waiting for connections...\n");
          struct sockaddr_in peer_addr;
          socklen_t addrlen = sizeof(peer_addr);

          int new_sock_fd = accept(server_fd, (struct sockaddr*)&peer_addr, &addrlen);
          if ( new_sock_fd < 0 ) {
               perror("Failed to accept connection\n");
               continue;
          }

          peer_t* peer = peer_create(inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));
          if ( !peer ) {
               perror("Failed to allocate memory for peer\n");
               close(new_sock_fd);
               continue;
          }
          peer->sock_fd = new_sock_fd;

          debug_print("Connected to new peer: %s:%d\n", inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));
          printf("Connected to peer\n");
          fflush(stdout);
          peers_add(peers, peer);
          peer_create_thread(peer, peers, bpkgs);
          pthread_testcancel();
     }
     pthread_cleanup_pop(1);
}

