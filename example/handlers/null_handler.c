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
static const uint64_t HWM = 100;


int main(int argc, char **args){

    char* SENDER = NULL;

    bstring pull_addr,
            pub_addr;

    mongrel2_socket *pull_socket,
                    *pub_socket;

    if (argc < 3 ) {
        printf("\nUsage: ./null_handler PULL_ADDR PUB_ADDR [PULL_ADDR PUB_ADDR ..] [HANDLER_ID]\n");
        printf("\nError: At least one PULL_ADDR, PUB_ADDR pair is required\n");
        exit(1);
    }

    mongrel2_ctx *ctx = mongrel2_init(3); // Yes for threads?

    pull_socket = mongrel2_pull_socket(ctx);
    //set high water mark to avoid flooding this handler
    mongrel2_set_rcvhwm(pull_socket, HWM);

    pub_socket = mongrel2_pub_socket(ctx);

    int i=0;
    for(; i< (argc-1 - (argc-1) % 2) ; i+=2){
        //PULL SOCKET "tcp://127.0.0.1:1999"
        //PUB  SOCKET "tcp://127.0.0.1:1998"
        pull_addr = bfromcstr(args[i+1]);
        pub_addr  = bfromcstr(args[i+2]);
        mongrel2_connect(pull_socket, bdata(pull_addr));
        mongrel2_connect(pub_socket, bdata(pub_addr));
        printf("Handler connected to 0mq PULL: %s  PUB:%s\n", bdata(pull_addr), bdata(pub_addr));
    }

    if (i+1<argc) {
        SENDER = args[i+1];
        printf("Handler ID : %s\n", SENDER);
        mongrel2_set_identity(pub_socket, SENDER);
    }
    char c[100];
    sprintf(c, "HTTP/1.1 204 No Content\r\nX-Ident: %s\r\nStatus: 200 OK", SENDER);
    const bstring headers = bfromcstr(c);
    mongrel2_request *request;

    printf("\nHandler configured to send back:\n%s\n", bdata(headers));

    printf("\nEntering main loop... \n");
    while(1){
        /* recv requests */
        request = mongrel2_recv(pull_socket);

        /* Retrieve request data */
        //fprintf(stdout,"PATH: %s\n",bdata(request->path));
        //fprintf(stdout,"HEADERS: %s\n",bdata(request->raw_headers));

        /**
        * JSON Header Parsing
        * parse headers (not done by default)
        */
        //mongrel2_parse_headers(request);

        /* Reqeuesting a header is also triggering json parsing */
        //fprintf(stdout,"HEADERS_JSON_URI: %s\n",bdata(mongrel2_request_get_header(request, "URI")));

        /* Check for disconnect (better than json parsing?) */
        if (blength(request->body) > 0 &&
            binstrcaseless(request->body, 0, &DISCONNECT) != BSTR_ERR ) {
            continue;
        }
        mongrel2_reply_http(pub_socket, request, headers, &EMPTY_BODY);
        /* Inform mongrel to close then client connection */
        //mongrel2_disconnect(pub_socket, request);
        mongrel2_request_finalize(request);
    }

    bdestroy(headers);

    //TODO: free all pull/pub addrs
    bdestroy(pull_addr);
    bdestroy(pub_addr);

    mongrel2_close(pull_socket);
    mongrel2_close(pub_socket);
    mongrel2_deinit(ctx);
    return 0;
}
