#ifndef WEBSOCKET_FRAMING_H

#include <stddef.h>
#include <stdint.h>

enum fflag_t {
	FIN      = 1,
	RSRVD1   = 2,
	RSRVD2   = 4,
	RSRVD3   = 8,
	OP_CONT  = 0,
	OP_TEXT  = 1,
	OP_BIN   = 2,
	OP_CLOSE = 8,
	OP_PING  = 9,
	OP_PONG  = 10,
	MASK     = 0x80,
};
typedef enum fflag_t fflag;

enum ftype_t {
	SMALL    = 125,
	MEDIUM   = 126,
	LARGE    = 127
};
typedef enum ftype_t ftype;

int mongrel2_ws_frame_set_fin(size_t len, uint8_t* frame);
uint8_t mongrel2_ws_frame_get_fin(size_t len, uint8_t* frame);
uint8_t mongrel2_ws_frame_get_opcode(size_t len, uint8_t* frame);
int mongrel2_ws_frame_set_opcode(size_t len, uint8_t* frame, fflag opcode);

int mongrel2_ws_frame_set_mask(size_t size, uint8_t *frame,uint32_t mask);
uint32_t mongrel2_ws_frame_get_mask(size_t size, uint8_t *frame);
int mongrel2_ws_frame_unmask(uint32_t mask, size_t size, uint8_t *frame);

ftype mongrel2_ws_frame_get_payload_type(size_t len, uint8_t* frame);
uint64_t mongrel2_ws_frame_get_payload_size(size_t len, uint8_t *frame);
int mongrel2_ws_frame_set_payload(size_t size, uint8_t *frame, uint64_t i_size, uint8_t *incoming);
int mongrel2_ws_frame_get_payload(size_t size, uint8_t *frame, size_t *osize, uint8_t **opayload);

int mongrel2_ws_frame_create(int use_mask,uint64_t payload_size,size_t *size,uint8_t **buf);

void mongrel2_ws_frame_debug(size_t len, uint8_t* header);

#endif