#include <assert.h>

#include "handler.h"
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