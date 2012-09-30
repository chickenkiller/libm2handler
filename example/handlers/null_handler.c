/**
 * Start up your mongrel2 server
 * Run this program
 * Then test it out using curl
 *
 * Call mongrel:
 * curl localhost:6767/path_to_null_handler -v
 *
 * You should get back a HTTP 1.1 204 : No content
 *
 */
#include "handler.h"

static const struct tagbstring DISCONNECT = bsStatic("disconnect");
static const struct tagbstring EMPTY_BODY = bsStatic("");


int main(int argc, char **args){

    char* SENDER = NULL;

    if (argc > 1 ) {
        SENDER = args[1];
    }

    printf("Handler ID : %s\n", SENDER);
    bstring pull_addr = bfromcstr("tcp://127.0.0.1:1999");
    bstring pub_addr  = bfromcstr("tcp://127.0.0.1:1998");

    mongrel2_ctx *ctx = mongrel2_init(1); // Yes for threads?

    mongrel2_socket *pull_socket = mongrel2_pull_socket(ctx);
    mongrel2_socket *pub_socket = mongrel2_pub_socket(ctx);

    mongrel2_connect(pull_socket, bdata(pull_addr));
    mongrel2_connect(pub_socket, bdata(pub_addr));
    if (SENDER != NULL) {
        mongrel2_set_identity(pub_socket, SENDER);
    }

    pull_addr = bfromcstr("tcp://127.0.0.1:2999");
    pub_addr  = bfromcstr("tcp://127.0.0.1:2998");
    mongrel2_connect(pull_socket, bdata(pull_addr));
    mongrel2_connect(pub_socket, bdata(pub_addr));
    if (SENDER != NULL) {
        mongrel2_set_identity(pub_socket, SENDER);
    }





    const bstring headers = bfromcstr("HTTP/1.1 204 No Content\r\nDate: Fri, 01 Jan 2012 01:15:42 GMT\r\nStatus: 200 OK");
    mongrel2_request *request;

    while(1){
        request = mongrel2_recv(pull_socket);
        //better than json parsing?
        if (blength(request->body) > 0 &&
            binstrcaseless(request->body, 0, &DISCONNECT) != BSTR_ERR ) {
            continue;
        }
        mongrel2_reply_http(pub_socket, request, headers, &EMPTY_BODY);
        //Inform mongrel to close then client connection
        //mongrel2_disconnect(pub_socket, request);
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
