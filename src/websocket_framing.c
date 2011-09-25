/**
 * Author: Xavier Lange
 * Date: 9/23/2011
 **/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "websocket_framing.h"

/**
 *   CONTROL FLAG FUNCTIONALITY
 **/

int mongrel2_ws_frame_set_fin(size_t len, uint8_t* frame){
	assert(len > 0);
	frame[0]  = FIN;
	return 0;
}

uint8_t mongrel2_ws_frame_get_fin(size_t len, uint8_t* frame){
    assert(len > 0);
    return ((frame[0] & 0x01) >> 0);
}

static uint8_t mongrel2_ws_frame_get_rsrvd1(size_t len, uint8_t* frame){
    assert(len > 0);
    return ((frame[0] & 0x02) >> 1);
}

static uint8_t mongrel2_ws_frame_get_rsrvd2(size_t len, uint8_t* frame){
    assert(len > 0);
    return ((frame[0] & 0x04) >> 2);
}

static uint8_t mongrel2_ws_frame_get_rsrvd3(size_t len, uint8_t* frame){
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

/**
 *   PAYLOAD FUNCTIONALITY
 **/

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

static int mongrel2_ws_frame_set_payload_size(size_t len, uint8_t *frame, uint64_t size){
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

// Future defs to keep things clean
static uint8_t mongrel2_ws_frame_get_mask_present(size_t len, uint8_t* frame);
static int mongrel2_ws_frame_get_mask_start(size_t size, uint8_t *frame);
static int mongrel2_ws_frame_get_payload_start(size_t size, uint8_t *frame){
	ftype type = mongrel2_ws_frame_get_payload_type(size,frame);
	int mask_present = mongrel2_ws_frame_get_mask_present(size,frame);

	int retval = 0;

	// fin,rsv + maskbit+length(1)
	retval = retval+2;

	if(mask_present == 1){
		// Oops, magic value. TODO: typedef mask to uint32_t and use sizeof.
		retval = retval+4;
	}
	switch(type){
		case SMALL:
			// nothing
			break;
		case MEDIUM:
			retval = retval+2;
			break;
		case LARGE:
			retval = retval+8;
			break;
		default:
			return -1;
	}
	return retval;
}

int mongrel2_ws_frame_set_payload(size_t size, uint8_t *frame, uint64_t i_size, uint8_t *incoming){
	int payload_start = mongrel2_ws_frame_get_payload_start(size,frame);
	#ifndef NDEBUG
	printf("&frame[%d] =   incoming\n",payload_start);
	#endif
	memcpy(&(frame[payload_start]),incoming,i_size);

	return 0;
	
}

int mongrel2_ws_frame_get_payload(size_t size, uint8_t *frame, size_t *osize, uint8_t **opayload){
	int payload_start = mongrel2_ws_frame_get_payload_start(size,frame);
	uint64_t payload_size =  mongrel2_ws_frame_get_payload_size(size,frame);

	*opayload = &(frame[payload_start]);
	*osize = payload_size;
	return 0;
}

/**
 *   MASK FUNCTIONALITY
 **/

static uint8_t mongrel2_ws_frame_get_mask_present(size_t len, uint8_t* frame){
    assert(len > 1);
    return ((frame[1] >> 7) & MASK);
}

static int mongrel2_ws_frame_get_mask_start(size_t size, uint8_t *frame){
	if (size < 5){
		return -2;
	} else if ((mongrel2_ws_frame_get_mask_present(size,frame)) != 1){
		return -3;
	}

	int mask_start = 2;
	fflag size_class = mongrel2_ws_frame_get_payload_type(size,frame);
	if(size_class == SMALL){
		// nothing
	} else if (size_class == MEDIUM){
		mask_start = mask_start + 2;
	} else if (size_class == LARGE){
		mask_start = mask_start + 8;
	} else {
		printf("unknown size_class got %d!\n",size_class);
		return -1;
	}
	// Reallign to make array lookup easy
	return mask_start;
}

int mongrel2_ws_frame_set_mask(size_t size, uint8_t *frame,uint32_t mask){
	#ifndef NDEBUG
	printf("FRAME SIZE  : %zd\n",size);
	printf("SIZEOF(mask): %ld\n",sizeof(mask));
	#endif
	assert(size > 5);

	// Set the mask only if we have that flag set. If they did not say so ahead
	// of time we have the wrong amount of memory allocated.
	// TODO: Let this be a safe state -- we would need to allow the modification of frame.
	// so a **frame would be necessary. Nah!
	int mask_start = mongrel2_ws_frame_get_mask_start(size,frame);
	#ifndef NDEBUG
	printf("MASK_START:   %d\n",mask_start);
	#endif
	if(mask_start < 0){
		return -1;
	}
	#ifndef NDEBUG
	printf("&frame[%d] =   0x%8X\n",mask_start,mask);
	#endif
	memcpy(&(frame[mask_start]),&mask,sizeof(uint32_t));

	// Assumption: the payload byte order never matters. Algorithm is the same.
	uint8_t *mask_i = (uint8_t*)&mask;
	int payload_start = mongrel2_ws_frame_get_payload_start(size,frame);
	int payload_size  = mongrel2_ws_frame_get_payload_size(size,frame);

	uint8_t *payload = &(frame[payload_start]);
	for(int i=0; i<payload_size; i++){
		payload[i] = payload[i] ^ mask_i[i%4];
	}

	return 0;
}

uint32_t mongrel2_ws_frame_get_mask(size_t size, uint8_t *frame){
	assert(size > 5);
	int mask_start = mongrel2_ws_frame_get_mask_start(size,frame);
	uint32_t retval;
	memcpy(&retval,&frame[mask_start],sizeof(uint32_t));
	return retval;
}

int mongrel2_ws_frame_unmask(uint32_t mask, size_t size, uint8_t *frame){
	uint8_t *mask_i = (uint8_t*)(&mask);
	for(int i=0; i<size; i++){
		frame[i] = frame[i] ^ mask_i[i%4];
	}
	return 0;
}

/**
 *   GENERAL FRAME CREATION
 **/

// This is broken if you are using a max payload size
static uint64_t mongrel2_ws_frame_get_size_necessary(int mask_present,uint64_t payload_size){
	uint64_t retval = 0;
	// fin,rsv1,rsv2,rsv3,opcode
	retval = retval + 1;
	// mask,payload_len(1)
	retval = retval + 1;

	if(mask_present){
		retval = retval + 4;
	}

	if(payload_size < 126){
		retval = retval;
	} else if (payload_size >= 126 && payload_size <= 65536){
		retval = retval + 8;
	} else {
		retval = retval + 16;
	}

	retval = retval + payload_size;

	return retval;
}

int mongrel2_ws_frame_create(int use_mask,uint64_t payload_size,size_t *size,uint8_t **buf){
	*size = mongrel2_ws_frame_get_size_necessary(use_mask,payload_size);
	*buf  = calloc(sizeof(uint8_t),*size);
	if(*buf == NULL){
		return -1;
	}

	mongrel2_ws_frame_set_payload_size(*size,*buf,payload_size);
	if(use_mask){
		// Set the high presence bit
		(*buf)[1] |= (0x80);
		// They will set the mask later. Otherwise it's zero!
	}
	return 0;
}

void mongrel2_ws_frame_debug(size_t len, uint8_t* header){
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
	    case OP_CLOSE:
	    	printf("OPCODE:  OP_CLOSE\n");
	    	break;
	    case OP_PING:
	    	printf("OPCODE:  OP_PING\n");
	    	break;
	    case OP_PONG:
	    	printf("OPCODE:  OP_PONG\n");
	    	break;
    	default:
    		printf("OPCODE:  UNKNOWN: %d\n",opcode);
    }


    uint8_t maskp  = mongrel2_ws_frame_get_mask_present(len,header);
    fflag msg_type = mongrel2_ws_frame_get_payload_type(len,header);

    uint32_t mask = 0;
    printf("MASKP:   %d\n",maskp);
        if(maskp == 1){
    	mask = mongrel2_ws_frame_get_mask(len,header);
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

    uint8_t *payload = NULL;
    size_t size = 0;
    mongrel2_ws_frame_get_payload(len,header,&size,&payload);
    if(size > 0 && size < 60 && payload != NULL){
    	printf("MSG:     %*s\n",(int)size,payload);
    	if(maskp == 1){
    		uint8_t *unmasked = calloc(size,sizeof(uint8_t));
    		memcpy(unmasked,payload,size);
    		mongrel2_ws_frame_unmask(mask,size,unmasked);
    		printf("UNMASKED: %.*s\n",(int)size,unmasked);
    		free(unmasked);
    	}
    } else if (size > 0){
    	printf("MSG:     %*s\n",60,payload);
    }

    fprintf(stdout,"==============================\n");
}