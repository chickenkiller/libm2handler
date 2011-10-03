// Author: Xavier Lange
// Date: 9/26/2011

#ifndef M2WSSESSION_H
#define M2WSSESSION_H

#include <pthread.h>

#include "handler.h"
#include "adt/dict.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct m2_ws_sessions_state_t{
	dict_t* dict;
	pthread_mutex_t dict_mutex;
} m2_ws_sessions_state;

typedef struct m2_ws_session_id_t {
    mongrel2_request *req;
} m2_ws_session_id;

typedef struct m2_ws_session_data_t {
    int times_seen;
    void* data;
} m2_ws_session_data;

int mongrel2_ws_sessions_state_init(m2_ws_sessions_state *container);
int mongrel2_ws_sessions_state_destroy(m2_ws_sessions_state *container);

dict_t *mongrel2_ws_sessions_state_get_dict(m2_ws_sessions_state *container);

int mongrel2_ws_sessions_state_add(m2_ws_sessions_state *container, mongrel2_request *req);
int mongrel2_ws_sessions_state_remove(m2_ws_sessions_state *container, mongrel2_request *req);
int mongrel2_ws_sessions_state_contains(m2_ws_sessions_state *container, mongrel2_request *req);

void mongrel2_ws_sessions_state_lock(m2_ws_sessions_state *container);
void mongrel2_ws_sessions_state_unlock(m2_ws_sessions_state *container);

#ifdef __cplusplus
}
#endif

// M2WEBSOCKETSESSION_H
#endif