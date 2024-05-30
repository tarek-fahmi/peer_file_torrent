// Local Dependencies:
#include <pkgchk.h>
#include <btide.h>
#include <my_utils.h>
#include <config.h>
#include <peer.h>
#include <pcomm.h>
#include <packet.h>
// Standard Linux Dependencies:
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// Additional Linux Dependencies:
#include <string.h>
#include <pthread.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

int p2p_setup_server(uint16_t port){
    errno = 0;
    int server_sock_fd;
    

    if ((server_sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("Socket creation for new peer failed...");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port),
    };
    

    if (bind(server_sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("Bind to new peer failed...");
        close(server_sock_fd);
        exit(EXIT_FAILURE);
    }
    debug_print("Server successful...");

        if (listen(server_sock_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    debug_print("Server listening...");


    debug_print("Successfully connected to new port...");
    return server_sock_fd;
}

void p2p_server_listening(int server_fd, int server_port, request_q_t* reqs_q, peers_t* peers)
{
    while (1) {
        printf("waiting for connections\n");

        socklen_t addrlen = sizeof(struct sockaddr);

        struct sockaddr_in peer_addr = {
            .sin_family = AF_INET,
            .sin_addr.s_addr = INADDR_ANY,
            .sin_port = server_port,
        };

        peer_t* new_peer = (peer_t*)malloc(sizeof(peer_t));

        if ((new_peer->sock_fd = accept(server_fd, (struct sockaddr *)&peer_addr, (socklen_t *)&peer_addr)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        peer_init(new_peer, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));
        

        debug_print("Connected to new peer successfully!");
        create_peer_thread(new_peer, reqs_q, peers);
    }
}

void create_peer_thread(peer_t* new_peer, request_q_t* reqs_q, peers_t* peers)
{

    peers_add(peers, new_peer);
    thread_args_t args = {
        .peer = new_peer,
        .requests = reqs_q,
        .peers = peers,
    };

    pthread_create(&new_peer->thread, NULL, peer_handler, &args);
}

void create_server_thread(){
    //TODO
}



packet_t* peer_try_receive(peer_t peer)
{
    uint8_t buffer[PAYLOAD_MAX];
    ssize_t nbytes = read(peer.sock_fd, buffer, sizeof(buffer));
    check_err(nbytes, "Failed to read from client....");
    if (nbytes == 0 ){
        return NULL;
    }

    packet_t pkt_in = packet_unmarshall(buffer);
    return &pkt_in;
}


void process_request_shared(peer_t* peer, request_q_t* reqs_q, peers_t* peers)
{
    request_t req = req_nextup(reqs_q);

    if (req.peer.port == peer->port){
        pthread_mutex_lock(&reqs_q->lock);
        request_t req = req_nextup(reqs_q);

        uint16_t code = req.packet.msg_code;

        switch(code){
            case PKT_MSG_PNG:
                send_png(peer);
                break;
            case PKT_MSG_ACP:
                send_acp(peer);
                break;
            case PKT_MSG_REQ:
                try_send(peer, req.packet);
                break;
            case PKT_MSG_DSN:
                send_dsn(peer);
                break;
            
        }

        pthread_mutex_unlock(&reqs_q->lock);
    }
}

void peer_handler(void *args_void) {

    thread_args_t *args = (thread_args_t*) args_void;
    peer_t* peer = args->peer;
    request_q_t *reqs_q = args->requests;
    peers_t* peers = args->peers;

    char buffer[4096];

    while (true) 
    {
        packet_t* pkt = peer_try_recieve(peer);
        
        if (pkt != NULL){
            handle_packet(peer, pkt);
        }
        process_request_shared(peer, reqs_q, peers);   
    }   
}
void handle_packet(peer_t* peer, packet_t* pkt)
{
    switch(pkt->msg_code){
        case PKT_MSG_PNG:
            send_pog(peer);
            break;
        case PKT_MSG_ACP:
            send_ack(peer);
            break;
        case PKT_MSG_REQ:
            //TODO: Write get_chunk function.
            get_chunk(&pkt);
            send_res(peer, pkt);
            break;
        case PKT_MSG_DSN:
            //TODO: Code for ending connection here.
            break;
    }
}


