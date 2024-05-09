#include <my_utils.h>
#include <stdlib.h>
#include <stdio.h>

FILE *my_open(char *path, char *mode){
    FILE *f_ptr = fopen(path, "");

    if (f_ptr == NULL){
        printf("Failed to open %s...\n", path);
        return NULL;
    }
    
    return f_ptr;
}