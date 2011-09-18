#include <assert.h>

#include <handler.h>
#include <websocket.h>
// #include <debug.h>

// framing flag
enum fflag_t {
	FIN      = 1,
	RSRVD1   = 2,
	RSRVD2   = 4,
	RSRVD3   = 8,
	OP_CONT  = 0,
	OP_TEXT  = 1,
	OP_BIN   = 2,
	MASK     = 1,
	NO_MASK  = 0,
	CLOSE    = 8,
	PING     = 9,
	PONG     = 10,
};
typedef enum fflag_t fflag;
enum ftype_t {
	SMALL    = 125,
	MEDIUM   = 126,
	LARGE    = 127
};
typedef enum ftype_t ftype;

uint64_t mongrel2_ws_frame_get_payload_size(size_t len, uint8_t *frame);


uint8_t mongrel2_ws_frame_get_fin(size_t len, uint8_t* frame){
    assert(len > 0);
    return ((frame[0] & 0x01) >> 0);
}

int mongrel2_ws_frame_set_fin(size_t len, uint8_t* frame){
	assert(len > 0);
	frame[0]  = FIN;
	return 0;
}

uint8_t mongrel2_ws_frame_get_rsrvd1(size_t len, uint8_t* frame){
    assert(len > 0);
    return ((frame[0] & 0x02) >> 1);
}

uint8_t mongrel2_ws_frame_get_rsrvd2(size_t len, uint8_t* frame){
    assert(len > 0);
    return ((frame[0] & 0x04) >> 2);
}

uint8_t mongrel2_ws_frame_get_rsrvd3(size_t len, uint8_t* frame){
    assert(len > 0);
    return ((frame[0] & 0x08) >> 3);
}

uint8_t mongrel2_ws_frame_get_opcode(size_t len, uint8_t* frame){
    assert(len > 0);
    return ((frame[0] & 0xF0) >> 4);
}

int mongrel2_ws_frame_set_opcode(size_t len, uint8_t* frame, fflag opcode){
	assert(len > 0);
	frame[0] |= (opcode << 4);
	return 0;
}

uint8_t mongrel2_ws_frame_get_mask_present(size_t len, uint8_t* frame){
    assert(len > 1);
    return ((frame[1] >> 7) & MASK);
}

ftype mongrel2_ws_frame_get_payload_type(size_t len, uint8_t* frame){
	assert(len > 1);
	uint8_t type = ((frame[1] & 0x7F));

	if(type <= 125){
		return SMALL;
	} else if(type == 126){
		printf("It's medium!\n");
		return MEDIUM;
	} else if(type == 127){
		return LARGE;
	} else {
		fprintf(stderr, "payload %d. Obviously wrong.\n",type);
		return -1;
	}
}

int mongrel2_ws_frame_set_payload_size(size_t len, uint8_t *frame, uint64_t size){
	assert(len > 2);

	uint16_t *medium_fptr = NULL;
	uint64_t *large_fptr  = NULL;

	if(size <= 125){
		printf("Setting a small (%lld) size!\n",size);
		frame[1] |= (size & 0x7F);
	} else if (size >= 126 && size <= 65536) {
		printf("Setting a medium (%lld) size!\n",size);
		frame[1] |= (126 & 0x7F);
		medium_fptr = (uint16_t*) &(frame[2]);
		*medium_fptr = (uint16_t) size;
	// TODO: Upper bounds check. && size <= 18446744073709551616LL doesn't work!
	} else if (size >= 65537){
		frame[1] |= (127 & 0x7F);
		large_fptr = (uint64_t*) &(frame[2]);
		*large_fptr = size;
	} else {
		return -1;
	}
	return 0;
}

uint64_t mongrel2_ws_frame_get_payload_size(size_t len, uint8_t *frame){
	ftype type = mongrel2_ws_frame_get_payload_type(len,frame);
	size_t retval = 0;
	switch(type){
		case SMALL:
			retval |= (frame[1] & 0x7F);
			break;
		case MEDIUM:
			retval |= (0x00000000000000FF & (frame[2]));
			retval |= (0x000000000000FF00 & (frame[3] << 8));
			break;
		case LARGE:
			return -1;
	}
	return retval;
}

void test_frame_small_no_mask(size_t *size_i, uint8_t **frame_i){
	*frame_i= calloc(sizeof(uint8_t),3);
	*size_i = 3;
	size_t size = *size_i;
	uint8_t *frame = *frame_i;

	mongrel2_ws_frame_set_fin(size,frame);
	mongrel2_ws_frame_set_opcode(size,frame,OP_TEXT);
	mongrel2_ws_frame_set_payload_size(size,frame,125);
	frame[1] |= 0x01;
	frame[2]  = 'h';
}

void test_frame_medium_no_mask(size_t *size_i, uint8_t **frame_i){
	*frame_i= calloc(sizeof(uint8_t),3);
	*size_i = 5;
	size_t size = *size_i;
	uint8_t *frame = *frame_i;

	mongrel2_ws_frame_set_fin(size,frame);
	mongrel2_ws_frame_set_opcode(size,frame,OP_BIN);
	mongrel2_ws_frame_set_payload_size(size,frame,1000);
	frame[1] |= 0x01;
	frame[2]  = 'h';
}

void mongrel2_debug_frame(size_t len, uint8_t* header);
int main(int argc, char** args){
	// small, no mask
	uint8_t *frame;
	size_t size;

	test_frame_small_no_mask(&size,&frame);
	mongrel2_debug_frame(size,frame);
	free(frame);

	// test_frame_medium_no_mask(&size,&frame);
	// mongrel2_debug_frame(size,frame);
	// free(frame);


	return 0;
}

void mongrel2_debug_frame(size_t len, uint8_t* header){
    fprintf(stdout,"=========== BYTES ============\n");
    uint8_t fin    = mongrel2_ws_frame_get_fin(len,header);
    uint8_t rsrvd1 = mongrel2_ws_frame_get_rsrvd1(len,header);
    uint8_t rsrvd2 = mongrel2_ws_frame_get_rsrvd2(len,header);
    uint8_t rsrvd3 = mongrel2_ws_frame_get_rsrvd3(len,header);
    printf("FIN:     %d\n",fin);
    printf("RSRVD1:  %d\n",rsrvd1);
    printf("RSRVD2:  %d\n",rsrvd2);
    printf("RSRVD3:  %d\n",rsrvd3);

    uint8_t opcode = mongrel2_ws_frame_get_opcode(len,header);
    switch(opcode){
    	case OP_CONT:
	    	printf("OPCODE:  OP_CONT\n");
	    	break;
    	case OP_TEXT:
	    	printf("OPCODE:  OP_TEXT\n");
	    	break;
    	case OP_BIN:
	    	printf("OPCODE:  OP_BIN\n");
	    	break;
    	default:
    		printf("OPCODE:  UNKNOWN!\n");
    }


    uint8_t maskp  = mongrel2_ws_frame_get_mask_present(len,header);
    fflag msg_type = mongrel2_ws_frame_get_payload_type(len,header);
    uint64_t msg_size = mongrel2_ws_frame_get_payload_size(len,header);

    printf("MASKP:   %d\n",maskp);
    switch(msg_type){
    	case SMALL:
    		printf("SIZE:    SMALL\n");
    		break;
    	case MEDIUM:
    		printf("SIZE:    MEDIUM\n");
    		break;
    	case LARGE:
    		printf("SIZE:    LARGE\n");
    		break;
    	default:
    		printf("SIZE:    UNKNOWN!");
    		break;
    }
    printf("MSG_SIZE:%lld\n",msg_size);

    // uint8_t payload= mongrel2_ws_frame_get_payload(len,header,size);
    fprintf(stdout,"==============================\n");
}