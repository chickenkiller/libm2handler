/* 
 * File:   m2websocket.h
 * Author: xavierlange
 *
 * Created on February 10, 2011, 3:07 PM
 */

#ifndef M2WEBSOCKET_H
#define	M2WEBSOCKET_H

#include "stdint.h"
#include "adt/dict.h"
#include "websocket/framing.h"
#include "websocket/session.h"

#define WEBSOCKET_SMALL_MAX 125
#define WEBSOCKET_MEDIUM_MAX 65536
#define WEBSOCKET_LARGE_MAX 184467440737095516

#ifdef __cplusplus
extern "C" {
#endif

int mongrel2_ws_reply_upgrade(mongrel2_request *req, mongrel2_socket *socket);
int mongrel2_ws_reply(mongrel2_socket *pub_socket, mongrel2_request *req, bstring data);

int mongrel2_ws_broadcast(mongrel2_socket *pub_socket, m2_ws_sessions_state *ses, bstring data);

// Expose to aid testing
// TODO: Wrap in a #DEFINE
bstring mongrel2_ws_upgrade_headers(mongrel2_request *req);
int mongrel2_ws_calculate_accept(mongrel2_request *req,bstring *accept);

#ifdef __cplusplus
}
#endif

#endif	/* M2WEBSOCKET_H */