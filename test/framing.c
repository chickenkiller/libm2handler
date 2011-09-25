#include <assert.h>

#include <handler.h>
#include <websocket.h>
#include <websocket_framing.h>
// #include <debug.h>

// TEST SETUP CODE

void test_frame_small_no_mask(size_t *size, uint8_t **frame){
	int retval = mongrel2_ws_frame_create(0,111,size,frame);
	assert(retval == 0);

	mongrel2_ws_frame_set_fin(*size,*frame);
	mongrel2_ws_frame_set_opcode(*size,*frame,OP_TEXT);
}

/* Expected format
 * BYTE[0]     FIN  = 1 | 0x00 | 0x00 | 0x00
 * BYTE[1]     MASKP = 1 | SIZE = 0x04
 * BYTE[2]     MASK        0xFF
 * BYTE[3]     MASK        0xEE
 * BYTE[4]     MASK        0xAA
 * BYTE[5]     MASK        0xDD
 * BYTE[6]     PAYLOAD     'A'
 * BYTE[7]     PAYLOAD     'S'
 * BYTE[8]     PAYLOAD     'D'
 * BYTE[9]     PAYLOAD     'F'
 */
void test_frame_small_with_mask(size_t *size, uint8_t **frame){
	int retval = mongrel2_ws_frame_create(1,4,size,frame);
	assert(retval == 0);

	mongrel2_ws_frame_set_fin(*size,*frame);
	mongrel2_ws_frame_set_opcode(*size,*frame,OP_TEXT);
	uint8_t data[4];
	data[0] = 0xFF;
	data[1] = 0xEE;
	data[2] = 0xAA;
	data[3] = 0xDD;

	mongrel2_ws_frame_set_payload(*size,*frame,4,data);
	mongrel2_ws_frame_set_mask(*size,*frame,0xFFEEAADD);
}


void test_frame_medium_no_mask(size_t *size, uint8_t **frame){
	int retval = mongrel2_ws_frame_create(0,2000,size,frame);
	assert(retval == 0);

	mongrel2_ws_frame_set_fin(*size,*frame);
	mongrel2_ws_frame_set_opcode(*size,*frame,OP_BIN);
}

void test_frame_medium_with_mask(size_t *size, uint8_t **frame){
	int retval = mongrel2_ws_frame_create(1,2244,size,frame);
	assert(retval == 0);

	mongrel2_ws_frame_set_fin(*size,*frame);
	mongrel2_ws_frame_set_opcode(*size,*frame,OP_BIN);
	mongrel2_ws_frame_set_mask(*size,*frame,0xFFFFFFFF);
}

void test_frame_large_no_mask(size_t *size, uint8_t **frame){
	int retval = mongrel2_ws_frame_create(0,66000,size,frame);
	assert(retval == 0);
	
	mongrel2_ws_frame_set_fin(*size,*frame);
	mongrel2_ws_frame_set_opcode(*size,*frame,OP_TEXT);
}

void mongrel2_ws_frame_debug(size_t len, uint8_t* header);
int main(int argc, char** args){
	uint8_t *frame;
	size_t size;

	printf("==== NO MASK ====\n");

	test_frame_small_no_mask(&size,&frame);
	mongrel2_ws_frame_debug(size,frame);
	free(frame);

	test_frame_medium_no_mask(&size,&frame);
	mongrel2_ws_frame_debug(size,frame);
	free(frame);

	test_frame_large_no_mask(&size,&frame);
	mongrel2_ws_frame_debug(size,frame);
	free(frame);

	printf("======== W/ MASK ========\n");

	test_frame_small_with_mask(&size,&frame);
	mongrel2_ws_frame_debug(size,frame);
	free(frame);

	test_frame_medium_with_mask(&size,&frame);
	mongrel2_ws_frame_debug(size,frame);
	free(frame);

	return 0;
}