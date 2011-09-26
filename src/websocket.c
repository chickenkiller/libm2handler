/*
 * File:   m2websocket.c
 * Author: xavierlange
 *
 * Created on February 10, 2011, 3:07 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include "handler.h"
#include "debug.h"
#include "websocket.h"
#include "websocket_framing.h"
#include "sha1/sha1.h"

#define START_CHAR (unsigned char)0x00
#define TERM_CHAR (unsigned char)0xFF
#define SHA1_LEN 20

static bstring mongrel2_ws_08_upgrade_headers(mongrel2_request *req);
static int mongrel2_ws_08_calculate_accept(mongrel2_request *req, bstring *ptr);

// static const char* WEBSOCKET_VERSION   = "sec-websocket-version";

static const char* WEBSOCKET_08_GUID    = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
static const char* WEBSOCKET_08_UPGRADE =
    "HTTP/1.1 101 Switching Protocols\r\n"
    "Upgrade: websocket\r\n"
    "Connection: Upgrade\r\n"
    "Sec-WebSocket-Accept: %*s\r\n\r\n";
static const char* WEBSOCKET_08_KEY      = "sec-websocket-key";
// static const char* WEBSOCKET_08_ORIGIN   = "sec-websocket-origin";
// static const char* WEBSOCKET_08_PROTOCOL = "sec-websocket-protocol";
// static const char* WEBSOCKET_08_CONN     = "connection";

int mongrel2_ws_reply_upgrade(mongrel2_request *req, mongrel2_socket *socket){
    bstring headers = mongrel2_ws_upgrade_headers(req);
    mongrel2_reply(socket,req,headers);
    bdestroy(headers);
    return 0;
}

bstring mongrel2_ws_upgrade_headers(mongrel2_request *req){
    // TODO, case state on the websocket version
    return mongrel2_ws_08_upgrade_headers(req);
}

int mongrel2_ws_calculate_accept(mongrel2_request *req, bstring *accept){
    return mongrel2_ws_08_calculate_accept(req,accept);
}

bstring mongrel2_ws_08_upgrade_headers(mongrel2_request *req){
    bstring accept = bfromcstr("");
    int retval;

    retval = mongrel2_ws_08_calculate_accept(req,&accept);
    if(retval != 0){
        return NULL;
    }
    bstring headers = bformat(WEBSOCKET_08_UPGRADE,blength(accept),bdata(accept));
    //fprintf(stdout,"upgrade headers\n=====\n%*s\n====\n",blength(headers),bdata(headers));
    //fprintf(stdout,"accept\n=====\n%*s\n====\n",blength(accept),bdata(accept));

    bdestroy(accept);

    return headers;
}

int mongrel2_ws_reply(mongrel2_socket *pub_socket, mongrel2_request *req, bstring data){
    size_t req_len = blength(data);
    uint8_t *req_data = (uint8_t*)bdata(data);

    size_t payload_len = 0;
    uint8_t *payload_data = NULL;

    int retval = mongrel2_ws_frame_create(0,req_len,&payload_len,&payload_data);

    if(retval != 0){
        fprintf(stderr,"mongrel2_ws_reply failed with errno %d\n",retval);
        return -1;
    }
    mongrel2_ws_frame_set_payload(payload_len,payload_data,req_len,req_data);
    mongrel2_ws_frame_set_opcode(payload_len,payload_data,OP_TEXT);
    mongrel2_ws_frame_set_fin(payload_len,payload_data);

    #ifndef NDEBUG
    mongrel2_ws_frame_debug(payload_len,payload_data);
    #endif

    bstring outgoing = bfromcstralloc((int)payload_len, (char*)payload_data);

    mongrel2_reply(pub_socket,req,outgoing);
    free(payload_data);

    bdestroy(outgoing);
    return 0;
}

/**
 * Take the request header and adds it to a magic GUID.
 * Which is returned in a SHA1-hash
 * @param req       - Mongrel2 request data
 * @return  0 on success
 */
static int mongrel2_ws_08_calculate_accept(mongrel2_request *req, bstring *ptr){
    int retval;
    bstring key    = mongrel2_request_get_header(req,WEBSOCKET_08_KEY);
    if(key == NULL){
        return -1;
    }

    retval = bcatcstr(key,WEBSOCKET_08_GUID);
    if(retval != 0){
        return -1;
    }

    void* buf = calloc(sizeof(char),SHA1_LEN);
    if(buf == NULL){
        return -1;
    }

    sha1((const unsigned char*)bdata(key),blength(key),buf);
    bstring accept = blk2bstr(buf,SHA1_LEN);
    bstring accept_encoded = bBase64Encode(accept);

    bdestroy(accept);
    free(buf);
    bdestroy(key);

    *ptr = accept_encoded;
    return 0;
}