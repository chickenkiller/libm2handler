#include <assert.h>
#include <inttypes.h>

#include "handler.h"
#include "websocket.h"
#include "debug.h"

#define START_CHAR (unsigned char)0x00
#define TERM_CHAR (unsigned char)0xFF

void mongrel2_debug_bstring(bstring data){
    if(data == NULL){
        fprintf(stderr, "cannot debug null data");
        return;
    }
    bstring single_hex = NULL;
    bstring single_char = NULL;

    bstring hex_dump = bfromcstr("");
    bstring san_dump = bfromcstr("");
    char* buf = calloc(4,sizeof(char));
    if(buf == NULL || data == NULL){
        fprintf(stderr, "debug could not allocate a conversion buffer");
        goto exit;
    }

    unsigned char* raw_char;
    unsigned char* cstr = (unsigned char*)bdata(data);
    if(cstr == NULL){
        goto exit;
        return;
    }

    for(int i=0; i<blength(data); i++){
        snprintf(buf,4,"%02X ",cstr[i]);
        single_hex = bfromcstr(buf);
        bconcat(hex_dump,single_hex);
        bdestroy(single_hex);

        raw_char = &cstr[i];
        if(*raw_char == START_CHAR){
            buf[0] = '@';
        } else if(*raw_char == TERM_CHAR){
            buf[0] = '@';
        } else {
            buf[0] = *raw_char;
        }
        buf[1] = '\0';
        
        single_char = bfromcstr(buf);
        bconcat(san_dump,single_char);
        bdestroy(single_char);
    }
    fprintf(stdout, "########################\n");
    fprintf(stdout, "SANITIZED DATA\n%.*s\n",blength(san_dump), bdata(san_dump));
    fprintf(stdout, "DEBUGGER SEZ\n%.*s\n", blength(hex_dump), bdata(hex_dump));
    fprintf(stdout, "########################\n");

    exit:
    bdestroy(san_dump);
    bdestroy(hex_dump);
    free(buf);

    return;
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
    ftype msg_type = mongrel2_ws_frame_get_payload_type(len,header);

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
    printf("MSG_SIZE:%" PRIu64 "\n",msg_size);

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

