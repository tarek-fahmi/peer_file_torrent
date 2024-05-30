#ifndef MY_UTILS_H
#define MY_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <merkletree.h>
#include <stdbool.h>

#define DEBUG_MODE (true)

#define ERR_MISC (0)

#ifndef MTLL_H
#define MTLL_H

#include <stdlib.h>

typedef struct q_node {
    void* data;
    struct q_node *next;
} q_node_t;

typedef struct queue {
    q_node_t *head, *tail;
} queue_t;

void check_err(int return_value, char* error_msg);

FILE *my_open(char *path, char *mode);

char **merge_arrays(char** a, char** b, int asize, int bsize);

char* truncate_string(char* str, int limit);

int check_null(void* obj);

void debug_print(char* msg);

queue_t* q_init();

void q_enqueue(queue_t* qobj, void* data);

void* q_dequeue(queue_t* qobj);

void q_destroy(queue_t* qobj);

int q_empty(queue_t* q_obj);


#endif // MTLL_H
#endif // MY_UTILS_H