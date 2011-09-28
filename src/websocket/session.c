// Author: Xavier Lange
// Date: 9/26/2011

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#include "adt/dict.h"
#include "websocket/session.h"

static int compare_session_id(const void *ses1_void, const void *ses2_void){
    m2_ws_session_id *ses1 = (m2_ws_session_id*)ses1_void;
    m2_ws_session_id *ses2 = (m2_ws_session_id*)ses2_void;
    printf("Comparing %d and %d\n",ses1->req->conn_id, ses2->req->conn_id);
    if(ses1->req->conn_id > ses2->req->conn_id){
        return 1;
    } else if(ses1->req->conn_id < ses2->req->conn_id){
        return -1;
    } else {
        return 0;
    }
}

static dnode_t *alloc_dict(void *notused) {
    return (dnode_t *)calloc(sizeof(dnode_t), 1);
}

static void free_dict(dnode_t *node, void *notused) {
    m2_ws_session_id *keyptr = (m2_ws_session_id*)dnode_getkey(node);
    m2_ws_session_data *valptr = (m2_ws_session_data*)dnode_get(node);
    printf("Freeing conn_id = %d, seen %d times\n",keyptr->req->conn_id,valptr->times_seen);
    free(keyptr);
    free(valptr);
    free(node);
}

int mongrel2_ws_sessions_state_init(m2_ws_sessions_state *state){
	state->dict = dict_create(DICTCOUNT_T_MAX, compare_session_id);
    dict_set_allocator(state->dict, alloc_dict, free_dict, NULL);
    pthread_mutex_init(&(state->dict_mutex),NULL);
    mongrel2_ws_sessions_state_lock(state);

    return 0;
}

int mongrel2_ws_sessions_state_destroy(m2_ws_sessions_state *state){
	return 0;
}

int mongrel2_ws_sessions_state_add(m2_ws_sessions_state *container, mongrel2_request *req){
    return 0;
}
int mongrel2_ws_sessions_state_remove(m2_ws_sessions_state *container, mongrel2_request *req){
    return 0;
}

void mongrel2_ws_sessions_state_lock(m2_ws_sessions_state *container){
    int retval = pthread_mutex_lock(&(container->dict_mutex));
    assert(retval == 0);
}

void mongrel2_ws_sessions_state_unlock(m2_ws_sessions_state *container){
    int retval = pthread_mutex_unlock(&(container->dict_mutex));
    assert(retval == 0);
}