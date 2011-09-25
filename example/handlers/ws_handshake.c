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

#include "handler.h"
#include "websocket.h"
#include "websocket_framing.h"

// Static function definitions
static const struct tagbstring SENDER = bsStatic("82209006-86FF-4982-B5EA-D1E29E55D483");

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
    
    while(shutdown != 1){
        poll_response = zmq_poll(&socket_tracker,1,500*1000);
        if(poll_response > 0){
            request = mongrel2_recv(pull_socket);
            fprintf(stdout,"got something...\n");
            if(request != NULL && mongrel2_request_for_disconnect(request) != 1){
                printf("Got a request with length: %d\n",blength(request->body));
                mongrel2_ws_reply_upgrade(request,pub_socket);
                if(blength(request->body) > 0){
                    mongrel2_ws_frame_debug(blength(request->body),(uint8_t*)bdata(request->body));
                }
                // bstring msg = bformat("{\"msg\" : \"hi there %d\"}", request->conn_id);
                // fprintf(stdout,"Sending new msg: '%*s'",blength(msg),bdata(msg));
                // mongrel2_ws_reply(pub_socket,request,msg);
                // bdestroy(msg);
                // mongrel2_request_finalize(request);
                // mongrel2_reply(pub_socket,request,bfromcstr(""));
            } else {
                fprintf(stdout,"Connection %d disconnected\n", request->conn_id);
            }
        } else if (poll_response < 0){
            fprintf(stdout, "Error on poll!");
            shutdown = 1;
        }
    }
    
    bdestroy(pull_addr);
    bdestroy(pub_addr);

    mongrel2_close(pull_socket);
    mongrel2_close(pub_socket);
    mongrel2_deinit(ctx);
    fprintf(stdout,"\nClean shutdown done! Thanks for playing!\n");
    return 0;
}