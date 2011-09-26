/* 
 * File:   m2websocket.h
 * Author: xavierlange
 *
 * Created on February 10, 2011, 3:07 PM
 */

#include "stdint.h"

#ifndef M2WEBSOCKET_H
#define	M2WEBSOCKET_H

int mongrel2_ws_reply_upgrade(mongrel2_request *req, mongrel2_socket *socket);
int mongrel2_ws_reply(mongrel2_socket *pub_socket, mongrel2_request *req, bstring data);

// Expose to aid testing
// TODO: Wrap in a #DEFINE
bstring mongrel2_ws_upgrade_headers(mongrel2_request *req);
int mongrel2_ws_calculate_accept(mongrel2_request *req,bstring *accept);

#endif	/* M2WEBSOCKET_H */