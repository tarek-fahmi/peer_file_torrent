#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <cstdint>

#include <sys/stat.h>
#include <sys/mman.h>

#include <pkgchk.h>
#include <merkletree.h>
#include <my_utils.h>

// Helper Functions

/**
 * @brief  Truncates a string if it is longer than a specified length, else does nothing.
 * @param  str: The string to truncate
 * @param  limit: The max length of the string
 */
char* truncate_string(char* str, int limit) {
    if (str != NULL && limit >= 0 && strlen(str) > limit) {
        str[limit] = '\0';
    }
    return str;
}

/**
 * @brief  Parses a chunk into the attributes of a node and chunk object.
 * @note   
 * @param  node*: The data container.
 * @param  dataStart: Pointer to the start of the data from bpkg file.
 * @param  rest: Pointer to the rest of the file.
 * @retval None
 */
void chunk_parse(mtree_node* node, char* dataStart, char* rest){
    char* exp_hash = strtok_r(dataStart, ",", &rest);
    ((chunk*) node->value)->offset = strtou(strtok_r(dataStart, ",", &rest));
    ((chunk*) node->value)->size = (uint32_t) strtou(strtok_r(dataStart, ",", &rest));
}

/**
 * Unpacks a key-value pair which is stored on one line. 
 *
 * @param bpkg, pointer to empty bpkg object
 * @param bpkgString, package file stored in continuous and complete string.
 * 
 * @return bpkg, pointer to constructed bpkg object
 */
void bpkg_monoparse(struct bpkg_obj* bpkg, char* dataStart, char* key, char* rest){
    char* data = strtok_r(dataStart, "\n", &rest);
    if (strcmp(key, "ident") == 0){
        truncate_string(data, IDENT_MAX);
        bpkg->ident = truncate(data);
        
    }else if (strcmp(key, "filename") == 0){
        truncate_strint(data, FILENAME_MAX);
        bpkg->filename = data;

    }else if (strcmp(key, "size") == 0){
        bpkg->size = strtoi(data);

    }else if (strcmp(key, "nhashes") == 0){
        bpkg->nhashes = strtoi(data);

    }else if (strcmp(key, "nchunks") == 0){
        bpkg->nchunks = strtoi(data);
    }
}

/**
 * Unpacks a key-value pair which is stored on multiple lines. 
 *
 * @param bpkg, pointer to empty bpkg object
 * @param bpkgString, package file stored in continuous and complete string.
 * 
 * @return bpkg, pointer to constructed bpkg object
 */
void bpkg_multiparse(bpkg_obj* bpkg, char* startData, char* key, char* rest){
    if (bpkg->nhashes != NULL && strcmp(key, "hashes") == 0){
        for(int i = 0; i < bpkg->nhashes, i++;){
            mtree_node* node = (mtree_node*) malloc(sizeof(mtree_node));

            bpkg->hashes[i] = string_truncate(strtok_r(startData, "\n", &rest), HASH_SIZE);
            node->is_leaf = 1;

            strcpy(node->expected_hash, bpkg->hashes[i]);
        }
    }else if (bpkg->nchunks != NULL && strcmp(key, "chunks") == 0){
        for(int i = 0; i < bpkg->nhashes, i++;){
            mtree_node* node = (mtree_node*) malloc(sizeof(mtree_node));

            bpkg->chunks[i] = string_truncate(strtok_r(startData, "\n", &rest), HASH_SIZE);
            node->is_leaf = 0;

            strcpy(node->expected_hash, bpkg->hashes[i]);
        }
    }
}

/**
 * Unpacks the contents of the package file into a bpkg_object.
 *
 * @param bpkg, pointer to empty bpkg object
 * @param bpkgString, package file stored in continuous and complete string.
 * 
 * @return bpkg, pointer to constructed bpkg object
 */
bpkg_obj* bpkg_unpack(struct bpkg_obj* bpkg, char* bpkgString){

    char* key;
    char* dataStart;
    char* rest;

        while((key = strtok_r(bpkgString, ":", &rest))){
            if (strncmp(key, "chunks") != 0 && strcmp(key, "hashes") != 0){
                bpkg_multiparse(bpkg, dataStart, key, rest);
            }else{
                char* defStr = strtok_r(bpkgString, "\n", &rest);
                bpkg_monoparse(bpkg, dataStart, key, rest);
            }
        }

}

// PART 1 Scaffold

/**
 * Gets only the required/min hashes to represent the current completion state
 * Return the smallest set of hashes of completed branches to represent
 * the completion state of the file.
 *
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
struct bpkg_obj* bpkg_load(const char* path) {
    

    int fd = open(path, O_RDONLY);
    if(fd < 0){
        perror("Failed to open file...");
        return NULL;
    }


    struct stat fstats;
    if(fstat(fd, &fstats)){
        perror("Fstat failure");
    }

    char* data = (char*) mmap(NULL, fstats.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data = MAP_FAILED){
        perror("mmap");
        return NULL;
    }

    close(fd);

    struct bpkg_obj* obj = NULL;

    return obj;
}

/**
 * Checks to see if the referenced filename in the bpkg file
 * exists or not.
 * @param bpkg, constructed bpkg object
 * @return query_result, a single string should be
 *      printable in hashes with len sized to 1.
 * 		If the file exists, hashes[0] should contain "File Exists"
 *		If the file does not exist, hashes[0] should contain "File Created"
 */
struct bpkg_query bpkg_file_check(struct bpkg_obj* bpkg){
    

    if(access(bpkg->filename, R_OK)){
        bpkg->hashes[0] = "File exists";

    }else{
        FILE * fptr = fopen(bpkg->filename, "wb");
        if(fptr == NULL){
            perror("Failed to create bpkg data file...");
        }

        fseek(fptr, bpkg->size - 1, SEEK_SET);
        fwrite("\0", 1, 1, fptr);
        bpkg->hashes[0] = "File created";
    }
    
    return;
}

/**
 * Retrieves a list of all hashes within the package/tree
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
struct bpkg_query bpkg_get_all_hashes(struct bpkg_obj* bpkg) {
    struct bpkg_query qry = { 0 };
    
    return qry;
}

/**
 * Retrieves all completed chunks of a package object
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
struct bpkg_query bpkg_get_completed_chunks(struct bpkg_obj* bpkg) { 
    struct bpkg_query qry = { 0 };
    return qry;
}


/**
 * Gets only the required/min hashes to represent the current completion state
 * Return the smallest set of hashes of completed branches to represent
 * the completion state of the file.
 *
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
struct bpkg_query bpkg_get_min_completed_hashes(struct bpkg_obj* bpkg) {
    struct bpkg_query qry = { 0 };
    return qry;
}


/**
 * Retrieves all chunk hashes given a certain an ancestor hash (or itself)
 * Example: If the root hash was given, all chunk hashes will be outputted
 * 	If the root's left child hash was given, all chunks corresponding to
 * 	the first half of the file will be outputted
 * 	If the root's right child hash was given, all chunks corresponding to
 * 	the second half of the file will be outputted
 * @param bpkg, constructed bpkg object
 * @return query_result, This structure will contain a list of hashes
 * 		and the number of hashes that have been retrieved
 */
struct bpkg_query bpkg_get_all_chunk_hashes_from_hash(struct bpkg_obj* bpkg, 
    char* hash) {
    
    struct bpkg_query qry = { 0 };
    return qry;
}


/**
 * Deallocates the query result after it has been constructed from
 * the relevant queries above.
 */
void bpkg_query_destroy(struct bpkg_query* qry) {
    //TODO: Deallocate here!

}

/**
 * Deallocates memory at the end of the program,
 * make sure it has been completely deallocated
 */
void bpkg_obj_destroy(struct bpkg_obj* obj) {
    //TODO: Deallocate here!

}


