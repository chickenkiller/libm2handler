/**
 * Start up your mongrel2 server
 * Run this program
 * Then test it out using Chrome 10 or higher
 */
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>

#include "handler.h"
#include "websocket.h"
#include "websocket_framing.h"
#include "websocket_session.h"
#include "adt/dict.h"

// Static function definitions
static const struct tagbstring SENDER = bsStatic("82209006-86FF-4982-B5EA-D1E29E55D483");
static const struct tagbstring HELLO = bsStatic("Hi there");

// Shared variables
static mongrel2_socket *pub_socket;
static int shutdown = 0;

static void call_for_stop(int sig_id){
    if(sig_id == SIGINT){
        if(shutdown == 1){
            fprintf(stderr, "SHUTDOWN CALLED... ASSUMING MURDER!");
            exit(EXIT_FAILURE);
        }
        shutdown = 1;
    }
}

static int compare_session(const void *ses1_void, const void *ses2_void){
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

int main(int argc, char **args){
    if(argc != 3){
        fprintf(stderr,"%s RECV SEND\n",args[0]);
        exit(1);
    }
    signal(SIGINT,&call_for_stop);
    
    bstring pull_addr = bfromcstr(args[1]);
    bstring pub_addr  = bfromcstr(args[2]);

    mongrel2_ctx *ctx = mongrel2_init(1); // Yes for threads?

    mongrel2_socket *pull_socket = mongrel2_pull_socket(ctx,bdata(&SENDER));
    mongrel2_connect(pull_socket, bdata(pull_addr));

    pub_socket = mongrel2_pub_socket(ctx);
    mongrel2_connect(pub_socket, bdata(pub_addr));

    mongrel2_request *request;

    // Polling is done to show how to do a clean shutdown
    int poll_response;
    zmq_pollitem_t socket_tracker;
    socket_tracker.socket = pull_socket->zmq_socket;
    socket_tracker.events = ZMQ_POLLIN;

    // Let's try out some ADT goodness
    dict_t* dict = dict_create(DICTCOUNT_T_MAX, compare_session);
    dict_set_allocator(dict, alloc_dict, free_dict, NULL);

    dnode_t* tempnode = NULL;
    m2_ws_session_data *counter = NULL;
    int retval = 0;
    while(shutdown != 1){
        poll_response = zmq_poll(&socket_tracker,1,500*1000);
        if(poll_response > 0){
            request = mongrel2_recv(pull_socket);
            fprintf(stdout,"got something...\n");

            if(request != NULL && mongrel2_request_for_disconnect(request) != 1){
                m2_ws_session_id* incoming = calloc(1,sizeof(m2_ws_session_id));
                incoming->req = request;
                printf("Looking at incoming->conn_id = %d\n",incoming->req->conn_id);
                tempnode = dict_lookup(dict,incoming);

                if(tempnode == NULL){
                    mongrel2_ws_reply_upgrade(request,pub_socket);
                    counter = calloc(1,sizeof(m2_ws_session_data));
                    counter->times_seen = 0;
                    retval = dict_alloc_insert(dict,incoming,counter);
                    assert(retval == 1);
                } else {
                    free(incoming);
                    counter = dnode_get(tempnode);
                    counter->times_seen += 1;
                }

                if(blength(request->body) > 0){
                    if(tempnode && mongrel2_ws_frame_get_fin(blength(request->body),
                                                             (uint8_t*)bdata(request->body))){
                        printf("Hey, it's a close\n");
                        dict_delete_free(dict,tempnode);
                        mongrel2_disconnect(pub_socket,request);
                    }
                } else {
                    mongrel2_ws_reply(pub_socket,request,(const bstring)&HELLO);
                }
                

                printf("FYI: we've got %ld entries\n",dict_count(dict));
            } else {
                fprintf(stdout,"Connection %d disconnected\n", request->conn_id);
            }
        } else if (poll_response < 0){
            fprintf(stdout, "Error on poll!");
            shutdown = 1;
        }
    }
    // bstring msg = bformat("{\"msg\" : \"hi there %d\"}", request->conn_id);
    // fprintf(stdout,"Sending new msg: '%*s'",blength(msg),bdata(msg));
    // mongrel2_ws_reply(pub_socket,request,msg);
    // bdestroy(msg);
    // mongrel2_request_finalize(request);
    // mongrel2_reply(pub_socket,request,bfromcstr(""));
    
    bdestroy(pull_addr);
    bdestroy(pub_addr);

    mongrel2_close(pull_socket);
    mongrel2_close(pub_socket);
    mongrel2_deinit(ctx);
    fprintf(stdout,"\nClean shutdown done! Thanks for playing!\n");
    return 0;
}