#include <peer_2_peer/package.h>
#include <peer_2_peer/packet.h>
#include <peer_2_peer/peer_data_sync.h>
#include <peer_2_peer/peer_handler.h>
#include <peer_2_peer/peer_server.h>

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

     if (bind(server_sock_fd, (struct sockaddr*)&server_addr,
              sizeof(server_addr)) < 0) {
          perror("Bind to new peer failed...\n");
          close(server_sock_fd);
          exit(EXIT_FAILURE);
     }
     debug_print("Server successfully bound to port: %d\n", port);

     if (listen(server_sock_fd, 3) < 0) {
          perror("Listen failed");
          close(server_sock_fd);
          exit(EXIT_FAILURE);
     }

     debug_print("Server listening on port: %d\n", port);
     return server_sock_fd;
}

void* server_thread_handler(void* args_void) {
     server_thr_args_t* args = (server_thr_args_t*)args_void;
     debug_print("Server thread handler started for port: %d\n",
                 args->server_port);
     p2p_server_listening(args->server_fd, args->server_port, args->reqs_q,
                          args->peers, args->bpkgs);
     debug_print("Server thread handler exiting for port: %d\n",
                 args->server_port);
     return NULL;
}

pthread_t create_p2p_server_thread(int server_fd, int server_port,
                                   request_q_t* reqs_q, peers_t* peers,
                                   bpkgs_t* bpkgs) {
     pthread_t server_thread;
     server_thr_args_t* args =
         (server_thr_args_t*)malloc(sizeof(server_thr_args_t));

     if (!args) {
          perror("Failed to allocate memory for server thread arguments\n");
          exit(EXIT_FAILURE);
     }

     args->server_fd = server_fd;
     args->server_port = server_port;
     args->reqs_q = reqs_q;
     args->peers = peers;
     args->bpkgs = bpkgs;  // Add missing assignment

     // Create the thread
     int result =
         pthread_create(&server_thread, NULL, server_thread_handler, args);
     if (result != 0) {
          perror("Server thread creation failed\n");
          free(args);
          exit(EXIT_FAILURE);
          return server_thread;
     }

     // Detach the thread to allow it to clean up after itself when it finishes
     result = pthread_detach(server_thread);
     if (result != 0) {
          perror("Failed to detach server thread\n");
          free(args);
          exit(EXIT_FAILURE);
          return server_thread;
     }

     debug_print("Server thread created successfully on port: %d\n",
                 server_port);
     return server_thread;
}

void server_thread_cleanup(void* arg) {
     int* server_fd = (int*)arg;
     if (server_fd && *server_fd >= 0) {
          close(*server_fd);
          debug_print("Server socket closed in cleanup.\n");
     }
}

void p2p_server_listening(int server_fd, int server_port, request_q_t* reqs_q,
                          peers_t* peers, bpkgs_t* bpkgs) {
     pthread_cleanup_push(server_thread_cleanup, &server_fd);
     while (true) {
          debug_print("Server on port %d waiting for connections...\n",
                      server_port);
          struct sockaddr_in peer_addr;
          socklen_t addrlen = sizeof(peer_addr);

          int new_sock_fd =
              accept(server_fd, (struct sockaddr*)&peer_addr, &addrlen);
          if (new_sock_fd < 0) {
               perror("Failed to accept connection\n");
               continue;
          }

          peer_t* peer = peer_create(inet_ntoa(peer_addr.sin_addr),
                                     ntohs(peer_addr.sin_port));
          peer->sock_fd = new_sock_fd;

          debug_print("Connected to new peer: %s:%d\n",
                      inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));
          peer_create_thread(peer, reqs_q, peers, bpkgs);
          pthread_testcancel();
     }
     pthread_cleanup_pop(1);  // Ensure the cleanup handler is executed
}