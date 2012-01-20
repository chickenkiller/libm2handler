/** Trying out a C++ example **/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <signal.h>

#include "handler.h"
#include "websocket.h"
#include "websocket_framing.h"
#include "websocket_session.h"
#include "debug.h"
#include "adt/dict.h"

static const struct tagbstring SENDER = bsStatic("82209006-86FF-4982-B5EA-D1E29E55D483");
// static const struct tagbstring HELLO  = bsStatic("HI THERE");
// static const struct tagbstring HELLO = bsStatic("{\"msg\": \"hi there\"}");
static const struct tagbstring HELLO = bsStatic("{\"message\": \"{\\\"guid\\\": \\\"63636330350000000000000000000000\\\"}\", \"metadata\": \"{\\\"participant\\\": \\\"ccc05-03\\\", \\\"timestamp_second\\\": \\\"1326922280\\\", \\\"timestamp_nanosec\\\": \\\"553700663\\\", \\\"instance_handle\\\": 12287918211081, \\\"topic\\\": \\\"HeartBeat_Pulse\\\"}\"}");

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

    mongrel2_ws_sessions_state_unlock(&sessions);
    mongrel2_request *request = NULL;
    while(shutdown != 1){
        poll_response = zmq_poll(&socket_tracker,1,500*1000);
        if(poll_response > 0){
            request = mongrel2_recv(pull_socket);

            if(request != NULL && mongrel2_request_for_disconnect(request) != 1){
                if(mongrel2_ws_sessions_state_contains(&sessions,request)){
                    printf("We had it already\n");
                } else {
                    mongrel2_ws_reply_upgrade(request,pub_socket);
                    mongrel2_ws_sessions_state_add(&sessions,request);
                }

                if(blength(request->body) > 0){
                    // mongrel2_ws_frame_debug(blength(request->body),(uint8_t*)bdata(request->body));
                    if(OP_CLOSE == mongrel2_ws_frame_get_opcode(blength(request->body),
                                                                (uint8_t*)bdata(request->body))){
                        printf("Hey, it's a close\n");
                    } else {
                        printf("Not a OP_CLOSE. bstring length: '%d'\n",blength(request->body));
                        mongrel2_ws_frame_debug(blength(request->body),
                                                                (uint8_t*)bdata(request->body));
                    }
                } else {
                    // mongrel2_ws_reply(pub_socket,request,(const bstring)&HELLO);
                    mongrel2_ws_broadcast(pub_socket,&sessions,(const bstring)&HELLO);
                }
            } else {
                fprintf(stdout,"Connection %d disconnected\n", request->conn_id);
                mongrel2_ws_sessions_state_remove(&sessions,request);
                mongrel2_disconnect(pub_socket,request);
            }

        } else if(poll_response < 0) {
            printf("ZMQ poller had an error");
        }
    }

    printf("\nThanks for playing\n");

    return 0;
}

