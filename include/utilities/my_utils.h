#ifndef UTILITIES_MY_UTILS_H
#define UTILITIES_MY_UTILS_H
#define _GNU_SOURCE

#ifdef DEBUG
#define debug_print(fmt, ...) fprintf(stderr, "DEBUG: " fmt, ##__VA_ARGS__)
#else
#define debug_print(fmt, ...) do {} while (0)
#endif

// Standard Linux Dependencies:
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// Additional Linux Dependencies:
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <features.h>
#include <math.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>

typedef struct q_node {
    void* data;
    struct q_node* next;
} q_node_t;

typedef struct {
    q_node_t* head;
    q_node_t* tail;
} queue_t;

/**
 * @brief Removes leading and trailing whitespace from a string.
 *
 * @param str Pointer to the string to trim.
 * @return Pointer to the trimmed string.
 */
char* trim_whitespace(char* str);

/**
 * @brief Sanitizes and processes a file path.
 *
 * @param path Pointer to the original path.
 * @return Pointer to the sanitized path.
 */
char* sanitize_path(const char* path);

/**
 * @brief Opens a file and maps it to shared memory.
 *
 * @param path Pointer to the file path.
 * @return Pointer to the shared memory.
 */
void* open_file_and_map_to_shared_memory(const char* path);

/**
 * @brief Allocates memory and handles errors.
 *
 * @param size Amount of memory to allocate.
 * @return Pointer to the allocated memory.
 */
void* my_malloc(size_t size);

/**
 * @brief Prints data in hexadecimal format.
 *
 * @param data Pointer to the data to print.
 * @param size Size of the data.
 */
void print_hex(const char* data, size_t size);

/**
 * @brief Merges two arrays into one.
 *
 * @param a First array.
 * @param b Second array.
 * @param asize Size of the first array.
 * @param bsize Size of the second array.
 * @return Pointer to the merged array.
 */
void** merge_arrays(void** a, void** b, int asize, int bsize);

/**
 * @brief Truncates a string if it exceeds a specified length.
 *
 * @param str Pointer to the string to truncate.
 * @param limit Maximum length of the string.
 * @return Pointer to the truncated string.
 */
char* truncate_string(char* str, int limit);

/**
 * @brief Checks if an object is NULL.
 *
 * @param obj Pointer to the object.
 * @return 1 if the object is NULL, 0 otherwise.
 */
int check_null(void* obj);

/**
 * @brief Checks for errors and prints a message if an error occurs.
 *
 * @param return_value Return value to check.
 * @param error_msg Pointer to the error message.
 * @return The return value.
 */
int check_err(int return_value, char* error_msg);

// Queue functions

/**
 * @brief Initializes an empty queue.
 *
 * @return Pointer to the initialized queue.
 */
queue_t* q_init();

/**
 * @brief Adds an element to the end of the queue.
 *
 * @param qobj Pointer to the queue.
 * @param data Pointer to the data to enqueue.
 */
void q_enqueue(queue_t* qobj, void* data);

/**
 * @brief Removes and returns the element at the front of the queue.
 *
 * @param qobj Pointer to the queue.
 * @return Pointer to the dequeued data.
 */
void* q_dequeue(queue_t* qobj);

/**
 * @brief Checks if the queue is empty.
 *
 * @param qobj Pointer to the queue.
 * @return 1 if the queue is empty, 0 otherwise.
 */
int q_empty(queue_t* qobj);

/**
 * @brief Destroys the queue and frees all allocated memory.
 *
 * @param qobj Pointer to the queue.
 */
void q_destroy(queue_t* qobj);

#endif