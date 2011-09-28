/** Trying out a C++ example **/

#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

#include "handler.h"
#include "websocket.h"
#include "websocket_framing.h"
#include "websocket_session.h"
#include "adt/dict.h"

static const struct tagbstring SENDER = bsStatic("82209006-86FF-4982-B5EA-D1E29E55D483");
static const struct tagbstring HELLO  = bsStatic("HI THERE");

static int shutdown = 0;
static void call_for_stop(int sig_id){
    if(sig_id == SIGINT){
        if(shutdown == 1){
            fprintf(stderr, "SHUTDOWN CALLED AGAIN... ASSUMING MURDER!");
            exit(EXIT_FAILURE);
        }
        shutdown = 1;
    }
}

int main(int argc, char **args){
    signal(SIGINT,&call_for_stop);

    if(argc != 3){
        fprintf(stderr,"%s RECV SEND\n",args[0]);
        exit(1);
    }
    bstring pull_addr = bfromcstr(args[1]);
    bstring pub_addr  = bfromcstr(args[2]);

    mongrel2_ctx *ctx = mongrel2_init(1);
    mongrel2_socket *pull_socket = mongrel2_pull_socket(ctx,bdata(&SENDER));
    mongrel2_connect(pull_socket, bdata(pull_addr));
    mongrel2_socket *pub_socket = mongrel2_pub_socket(ctx);
    mongrel2_connect(pub_socket, bdata(pub_addr));
    
    // Polling is done to show how to do a clean shutdown
    int poll_response;
    zmq_pollitem_t socket_tracker;
    socket_tracker.socket = pull_socket->zmq_socket;
    socket_tracker.events = ZMQ_POLLIN;

    // Let's try out some C-style ADT goodness
    m2_ws_sessions_state sessions;
    mongrel2_ws_sessions_state_init(&sessions);

    mongrel2_request *request = NULL;
    dnode_t* tempnode = NULL;
    int retval = 0;
    m2_ws_session_data* counter = NULL;
    while(shutdown != 1){
        poll_response = zmq_poll(&socket_tracker,1,500*1000);
        if(poll_response > 0){
            request = mongrel2_recv(pull_socket);

            if(request != NULL && mongrel2_request_for_disconnect(request) != 1){
                m2_ws_session_id* incoming = (m2_ws_session_id*)calloc(1,sizeof(m2_ws_session_id));
                incoming->conn_id = request->conn_id;
                printf("Looking at incoming->conn_id = %d\n",incoming->conn_id);
                tempnode = dict_lookup(sessions.dict,incoming);

                if(tempnode == NULL){
                    mongrel2_ws_reply_upgrade(request,pub_socket);
                    counter = (m2_ws_session_data*)calloc(1,sizeof(m2_ws_session_data));
                    counter->times_seen = 0;
                    retval = dict_alloc_insert(sessions.dict,incoming,counter);
                    assert(retval == 1);
                } else {
                    free(incoming);
                    counter = (m2_ws_session_data*)dnode_get(tempnode);
                    counter->times_seen += 1;
                }

                if(blength(request->body) > 0){
                    // mongrel2_ws_frame_debug(blength(request->body),(uint8_t*)bdata(request->body));
                    if(tempnode && mongrel2_ws_frame_get_fin(blength(request->body),(uint8_t*)bdata(request->body))){
                        printf("Hey, it's a close\n");
                        dict_delete_free(sessions.dict,tempnode);
                        mongrel2_disconnect(pub_socket,request);
                    }
                } else {
                    mongrel2_ws_reply(pub_socket,request,(const bstring)&HELLO);
                }
                

                printf("FYI: we've got %ld entries\n",dict_count(sessions.dict));
            } else {
                fprintf(stdout,"Connection %d disconnected\n", request->conn_id);
            }

        } else if(poll_response < 0) {
            printf("ZMQ poller had an error");
        }
    }

    printf("\nThanks for playing\n");

    return 0;
}