/**
 * A sample body to upper daemon. Second generation handler
 * with argument handling and daemonization.
 *
 * Author: Xavier Lange
 * Created on May 23, 2011, 4:59 PM
 *
 **/
#include "handler.h"
static const struct tagbstring SENDER = bsStatic("82209006-86FF-4982-B5EA-D1E29E55D481");

int main(int argc, char **args){
    char* SENDER = NULL;

    if (argc > 1 ) {
        SENDER = args[1];
    }
    printf("Handler ID : %s\n", SENDER);

    bstring pull_addr = bfromcstr("tcp://127.0.0.1:9999");
    bstring pub_addr  = bfromcstr("tcp://127.0.0.1:9998");

    mongrel2_ctx *ctx = mongrel2_init(1); // Yes for threads?

    mongrel2_socket *pull_socket = mongrel2_pull_socket(ctx);
    mongrel2_connect(pull_socket, bdata(pull_addr));

    mongrel2_socket *pub_socket = mongrel2_pub_socket(ctx);
    mongrel2_connect(pub_socket, bdata(pub_addr));
    if (SENDER != NULL) {
        mongrel2_set_identity(pub_socket, SENDER);
    }

    const bstring headers = bfromcstr("HTTP/1.1 200 OK\r\nDate: Fri, 07 Jan 2011 01:15:42 GMT\r\nStatus: 200 OK\r\nConnection: close");
    mongrel2_request *request;

    while(1){
        request = mongrel2_recv(pull_socket);
        btoupper(request->body);
        mongrel2_reply_http(pub_socket, request, headers, request->body);
        mongrel2_disconnect(pub_socket, request);
        mongrel2_request_finalize(request);
    }

    bdestroy(headers);

    bdestroy(pull_addr);
    bdestroy(pub_addr);

    mongrel2_close(pull_socket);
    mongrel2_close(pub_socket);
    mongrel2_deinit(ctx);
    return 0;
}

