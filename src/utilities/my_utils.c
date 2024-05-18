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
}

