/ Open a file in shared memory.
#ifndef CONFIG_H
#define CONFIG_H

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


void* open_file_and_map_to_shared_memory(const char* path) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open package...");
        exit(EXIT_FAILURE);
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("Failed to get file size");
        close(fd);
        return NULL;
    }

    void* addr = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("Failed to map file to memory");
        close(fd);
        return NULL;
    }

    close(fd);
    return addr;
}


void *my_malloc(size_t size)
{
    void* heap_obj = malloc(size);
    if (heap_obj == NULL)
    {
        perror("Malloc failed... :(");
        exit(EXIT_FAILURE);
    }
    return heap_obj;
}


/**
 * @brief  Merges two character arrays, and returns the concatenation of the arrays (unsorted).
 */
char** merge_arrays(char** a, char** b, int asize, int bsize){
    char** newarr = my_malloc(sizeof(asize + bsize) * sizeof(char*));
    memcpy(newarr, a, sizeof(char) * asize);
    memcpy(newarr + asize, b, sizeof(char) * bsize);

    free(a);
    free(b);
    return newarr;
}

/**
 * @brief  Truncates a string if it is longer than a specified length, else does
 * nothing.
 * @param  str: The string to truncate
 * @param  limit: The max length of the string
 */
char* truncate_string(char* str, int limit)
{
    if (str != NULL && limit >= 0 && strlen(str) > limit) {
        str[limit] = '\0';
    }
    return str;
    \
}

int check_null(void* obj){
    if (obj == NULL) return -1;
    else return 0;
}


check(int return_value, char* error_msg){
    if (return_value < 0){
        fprintf(stderr, "%s", error_msg);
        exit(EXIT_FAILURE);
    }
    return;
}



/* The following code pertains to my queue in linked list ADT, inspired by my code from "Assignment 2: Multi Type Linked List" */

// Initialies an empty queue on the heap, returning a pointer.
queue_t* q_init(queue_t* q_obj) {
    queue_t* queue = (queue_t*)my_malloc(sizeof(queue_t));
    q_obj->head = NULL;
    q_obj->tail = NULL;
    return queue;
}

// Enqueues a queue element, storing data in the end of the linked list and allocating memory.
void q_enqueue(queue_t* qobj, void* data) {
    q_node_t* new_node = (q_node_t*) my_malloc(sizeof(q_node_t));
    if (new_node == NULL) {
        perror("Failed to allocate memory for queue node...");
        exit(EXIT_FAILURE);
    }

    new_node->data = data;
    new_node->next = NULL;

    if (qobj->tail != NULL) {
        qobj->tail->next = new_node;
    } else {
        qobj->head = new_node;
    }

    qobj->tail = new_node;
}

// Dequeues a node, removing it from the head of the list and deallocating memory.
void* q_dequeue(queue_t* qobj) {
    if (qobj->head == NULL) {
        return NULL; // Queue is empty
    }

    q_node_t* temp = qobj->head;
    qobj->head = qobj->head->next;

    if (qobj->head == NULL) {
        qobj->tail = NULL; // Queue is now empty
    }

    void* data = temp->data;
    free(temp);
    return
    
     data;
}

int q_empty(queue_t* qobj)
{
    if (qobj->head == NULL && qobj->tail == NULL)
    {
        return 1;
    }
    return 0;
}

// Destroys the queue, freeing all dynamically allocated memory.
void q_destroy(queue_t* qobj) {
    q_node_t* current = qobj->head;

    while (current != NULL) {
        q_node_t* temp = current;
        current = current->next;
        free(temp);
    }
    free(qobj);
}

// Local Dependencies:
#include <btide.h>
#include <config.h>
#include <my_utils.h>
// Standard Linux Dependencies:
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
// Additional Linux Dependencies:
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#define MAX_PEERS            (2048)
#define MAX_PORT             (65535)
#define MAX_LINE_LENGTH      (1024)
#define MAX_DIRECTORY_LENGTH (256)

#define MIN_PEERS            (1)
#define MIN_PORT             (1024)

#define ERR_DIRECTORY        (3)
#define ERR_PEERS            (4)
#define ERR_PORT             (5)

typedef struct config_object{
    char directory[MAX_DIRECTORY_LENGTH];
    uint16_t max_peers;
    uint32_t port;
}config_t;


static int parse_entry(char* line, config_t* c_obj);

static int check_directory(char* pathname);

int config_load(char* filename, config_t* c_obj);