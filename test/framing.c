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
	uint8_t type = (frame[1] & 0x7F);
	if(type <= 125){
		return SMALL;
	} else if(type == 126){
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
	uint8_t *sizeptr = (uint8_t*) (&size);

	if(size <= 125){
		frame[1] |= (size & 0x7F);
	} else if (size >= 126 && size <= 65536) {
		frame[1] |= (126 & 0x7F);
		memcpy(&(frame[2]),sizeptr,2);
	// TODO: Upper bounds check. && size <= 18446744073709551616LL doesn't work!
	// TODO: Add msg size assertions
	} else if (size >= 65537){
		frame[1] |= (127 & 0x7F);
		memcpy(&(frame[2]),sizeptr,8);
	} else {
		return -1;
	}
	return 0;
}

uint64_t mongrel2_ws_frame_get_payload_size(size_t len, uint8_t *frame){
	ftype type = mongrel2_ws_frame_get_payload_type(len,frame);

	uint64_t retval = 0x0000000000000000;
	switch(type){
		case SMALL:
			retval |= (frame[1] & 0x7F);
			break;
		case MEDIUM:
			memcpy(&retval,&(frame[2]),2);
			break;
		case LARGE:
			memcpy(&retval,&(frame[2]),8);
			break;
	}
	return retval;
}

int mongrel2_ws_frame_get_mask_start(size_t size, uint8_t *frame){
	if (size < 5 || mongrel2_ws_frame_get_mask_present(size,frame) != 1){
		return -1;
	}

	int mask_start = 2;
	fflag size_class = mongrel2_ws_frame_get_payload_size(size,frame);
	if(size_class == SMALL){
		// nothing
	} else if (size_class == MEDIUM){
		mask_start = mask_start + 2;
	} else if (size_class == LARGE){
		mask_start = mask_start + 8;
	} else {
		return -1;
	}
	// Reallign to make array lookup easy
	return mask_start-1;
}

int mongrel2_ws_frame_set_mask(size_t size, uint8_t *frame,uint32_t mask){
	assert(size > 5);
	// Set the high presence bit
	frame[1] |= (0x80);

	// Set the mask
	int mask_start = mongrel2_ws_frame_get_mask_start(size,frame);
	memcpy(&frame[mask_start],&mask,sizeof(uint32_t));

	return 0;
}

uint32_t mongrel2_ws_frame_get_mask(size_t size, uint8_t *frame){
	assert(size > 5);
	int mask_start = mongrel2_ws_frame_get_mask_start(size,frame);
	uint32_t retval;
	memcpy(&retval,&frame[mask_start],sizeof(uint32_t));
	return retval;
}

// This is broken if you are using a max payload size
uint64_t mongrel2_ws_frame_get_size_necessary(int mask_present,uint64_t payload_size){
	uint64_t retval = 0;
	// fin,rsv1,rsv2,rsv3,opcode
	retval = retval + 1;
	// mask,payload_len(1)
	retval = retval + 1;

	if(mask_present){
		retval = retval + 4;
	}

	if(payload_size < 126){
		retval = retval + 2;
	} else if (payload_size >= 126 && payload_size <= 65536){
		retval = retval + 8;
	} else {
		retval = retval + 16;
	}

	return retval;
}

int mongrel2_ws_frame_create(int use_mask,uint64_t payload_size,size_t *size,uint8_t **buf){
	*size = mongrel2_ws_frame_get_size_necessary(use_mask,payload_size);
	*buf  = calloc(sizeof(uint8_t),*size);
	if(*buf == NULL){
		return -1;
	}

	mongrel2_ws_frame_set_payload_size(*size,*buf,payload_size);
	return 0;
}

// TEST SETUP CODE

void test_frame_small_no_mask(size_t *size, uint8_t **frame){
	int retval = mongrel2_ws_frame_create(0,111,size,frame);
	assert(retval == 0);

	mongrel2_ws_frame_set_fin(*size,*frame);
	mongrel2_ws_frame_set_opcode(*size,*frame,OP_TEXT);
}

void test_frame_small_with_mask(size_t *size, uint8_t **frame){
	int retval = mongrel2_ws_frame_create(1,123,size,frame);
	assert(retval == 0);
	
	mongrel2_ws_frame_set_fin(*size,*frame);
	mongrel2_ws_frame_set_opcode(*size,*frame,OP_TEXT);
	mongrel2_ws_frame_set_mask(*size,*frame,0x11FF22FF);
}


void test_frame_medium_no_mask(size_t *size, uint8_t **frame){
	int retval = mongrel2_ws_frame_create(0,2000,size,frame);
	assert(retval == 0);

	mongrel2_ws_frame_set_fin(*size,*frame);
	mongrel2_ws_frame_set_opcode(*size,*frame,OP_BIN);
}

void test_frame_large_no_mask(size_t *size, uint8_t **frame){
	int retval = mongrel2_ws_frame_create(0,66000,size,frame);
	assert(retval == 0);
	
	mongrel2_ws_frame_set_fin(*size,*frame);
	mongrel2_ws_frame_set_opcode(*size,*frame,OP_TEXT);
}

void mongrel2_debug_frame(size_t len, uint8_t* header);
int main(int argc, char** args){
	uint8_t *frame;
	size_t size;

	test_frame_small_no_mask(&size,&frame);
	mongrel2_debug_frame(size,frame);
	free(frame);

	test_frame_medium_no_mask(&size,&frame);
	mongrel2_debug_frame(size,frame);
	free(frame);

	test_frame_large_no_mask(&size,&frame);
	mongrel2_debug_frame(size,frame);
	free(frame);

	test_frame_small_with_mask(&size,&frame);
	mongrel2_debug_frame(size,frame);
	free(frame);

	return 0;
}

void mongrel2_debug_frame(size_t len, uint8_t* header){
    fprintf(stdout,"=========== BYTES ============\n");
    printf("BYTES:   %zd\n",len);

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

    printf("MASKP:   %d\n",maskp);
        if(maskp == 1){
    	uint32_t mask = mongrel2_ws_frame_get_mask(len,header);
    	printf("MASK:    0x%8X\n",mask);
	}

    uint64_t msg_size = mongrel2_ws_frame_get_payload_size(len,header);
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