#ifndef MY_UTILS_H
#define MY_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <merkletree.h>

#define ERR_MISC (0)

FILE *my_open(char *path, char *mode);


/**
 * @brief  Merges two character arrays, and returns the concatenation of the arrays (unsorted).
 */
char **merge_arrays(char** a, char** b, int asize, int bsize);

/**
 * @brief  Truncates a string if it is longer than a specified length, else does
 * nothing.
 * @param  str: The string to truncate
 * @param  limit: The max length of the string
 */
char* truncate_string(char* str, int limit);

int check_null(void* obj){
    if (obj == NULL) return -1;
    else return 0;
}

#endif
