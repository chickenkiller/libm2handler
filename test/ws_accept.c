#include "handler.h"
#include "websocket.h"

int main(int argc, char **args){
    /**
     * Example from page 8 of the web socket protocol RFC : http://www.whatwg.org/specs/web-socket-protocol/
     */
    char *headers = "{\"PATH\":\"/dds_stream\",\"host\":\"localhost:6767\","
                    "\"sec-websocket-key\":\"dGhlIHNhbXBsZSBub25jZQ==\","
                    "\"sec-websocket-origin\":\"http://example.com\",\"x-forwarded-for\":\"::1\","
                    "\"upgrade\":\"WebSocket\",\"connection\":\"Upgrade\","
                    "\"sec-websocket-version\":\"8\","
                    "\"METHOD\":\"GET\",\"VERSION\":\"HTTP/1.1\",\"URI\":\"/dds_stream\","
                    "\"PATTERN\":\"/dds_stream\"}";
    mongrel2_request *req = calloc(1,sizeof(mongrel2_request));
    req->raw_headers = bfromcstr(headers);
    req->headers = json_loads(bdata(req->raw_headers),0,NULL);

    bstring accept = bfromcstr("");
    mongrel2_ws_calculate_accept(req,&accept);

    mongrel2_request_finalize(req);
    if(biseqcstr(accept,"s3pPLMBiTxaQ9kYGzzhZRbK+xOo=") == 1){
        fprintf(stdout,"ws accept succeeded\n");
        return 0;
    } else {
        fprintf(stderr,"ws_accept failed, was %s\n",bdata(accept));
        return -1;
    }
}

