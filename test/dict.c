#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define DICT_IMPLEMENTATION
#include "adt/dict.h"

typedef struct key_t {
    int key;
} key;

typedef struct value_t {
    char *string;
} value;

int compare_request(const void *key1_void, const void *key2_void){
    key *key1 = (key*)key1_void;
    key *key2 = (key*)key2_void;
    printf("Comparing %d and %d\n",key1->key,key2->key);
    if(key1->key > key2->key){
        return 1;
    } else if(key1->key < key2->key){
        return -1;
    } else{
        return 0;
    }
}

static dnode_t *alloc_dict(void *notused) {
    printf("Asked to allocate\n");
    return (dnode_t *)calloc(sizeof(dnode_t), 1);
}

static void free_dict(dnode_t *node, void *notused) {
    printf("Asked to free a value\n");
    key *keyptr = (key*)dnode_getkey(node);
    value *valptr = (value*)dnode_get(node);
    free(keyptr);
    free(valptr);
    free(node);
}

int main(int argc, char **args){
    dict_t* dict = dict_create(DICTCOUNT_T_MAX, compare_request);
    dict_set_allocator(dict, alloc_dict, free_dict, NULL);

    key *key1 = calloc(1,sizeof(key));
    key1->key = 10;
    key *key2 = calloc(1,sizeof(key));
    key2->key = 5;

    value *val1 = calloc(1,sizeof(value));
    val1->string = "unsafe memory!";
    value *val2 = calloc(1,sizeof(value));
    val2->string = "unsafe memor!";

    assert(dict_alloc_insert(dict,key1,val1) == 1);
    assert(dict_count(dict) == 1);
    dnode_t *temp1 = dict_lookup(dict,key1);
    assert(temp1 != NULL);
    assert(((key*)dnode_getkey(temp1))->key == 10);
    assert(((value*)dnode_get(temp1))->string != NULL);

    assert(dict_alloc_insert(dict,key2,val2) == 1);
    assert(dict_count(dict) == 2);
    dnode_t *temp2 = dict_lookup(dict,key2);
    assert(temp2 != NULL);

    dict_free_nodes(dict);
    dict_destroy(dict);
    return 0;
}