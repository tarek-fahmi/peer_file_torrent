#include <my_utils.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <merkletree.h>


/**
 * @brief Attempts to open a file, throwing an error 
 *         if the file failed to open.
 * @param path, filepath
 * @param mode, open mode.
*/
FILE *my_open(char *path, char *mode){
    FILE *f_ptr = fopen(path, "");

    if (f_ptr == NULL) {
        printf("Failed to open %s...\n", path);
        return NULL;
    }
    
    return f_ptr;
}


/**
 * @brief  Merges two character arrays, and returns the concatenation of the arrays (unsorted).
 */
char** merge_arrays(char** a, char** b, int asize, int bsize){
    char** newarr = malloc(sizeof(asize + bsize) * sizeof(char*));
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

void debug_print(char* msg){
    printf("%s", msg);
    return;
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
void q_init(queue_t* q_obj) {
    q_obj->head = NULL;
    q_obj->tail = NULL;
    return NULL;
}

// Enqueues a queue element, storing data in the end of the linked list and allocating memory.
void q_enqueue(queue_t* qobj, void* data) {
    q_node_t* new_node = (q_node_t*) malloc(sizeof(q_node_t));
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