#include <utilities/my_utils.h>
#include <tree/merkletree.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Open a file in shared memory.
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
        free(heap_obj);
        exit(EXIT_FAILURE);
    }
    return heap_obj;
}


/**
 * @brief  Merges two arrays, and returns the concatenation of the arrays (unsorted).
 */
void** merge_arrays(void** a, void** b, int asize, int bsize) {
    void** newarr = (void**) my_malloc((asize + bsize) * sizeof(void*));
    memcpy(newarr, a, asize * sizeof(void*));
    memcpy(newarr + asize, b, bsize * sizeof(void*));

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
}

int check_null(void* obj){
    if (obj == NULL) return -1;
    else return 0;
}


int check_err(int return_value, char* error_msg){
    if (return_value < 0){
        fprintf(stderr, "%s", error_msg);
        exit(EXIT_FAILURE);
    }
    return-1;
}



/* The following code pertains to my queue in linked list ADT, inspired by my code from "Assignment 2: Multi Type Linked List" */

// Initialies an empty queue on the heap, returning a pointer.
queue_t* q_init() {
    queue_t* q_obj = (queue_t*)my_malloc(sizeof(queue_t));
    q_obj->head = NULL;
    q_obj->tail = NULL;
    return q_obj;
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

void q_node_destroy(q_node_t* node)
{
    free(node);
    return;
}

void print_hex(const char *data, size_t size) {
    printf("\n");
}