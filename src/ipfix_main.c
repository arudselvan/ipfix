/*
 * Copyright (c) 2018 Plume Design Inc.
 * All rights reserverd.
 * Author (s): Arud selvan
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "ipFixHdr.h"

extern int8_t decode_ipfix_msg (char *msg);

int main(int argc, char **argv) {

    FILE *fd = NULL;
    char *rec_buf = NULL, *tmp = NULL;
    uint32_t bytes2read = 0;
    ipfix_msg_hdr_t msg_hdr;

    if (argc < 2) {
        printf("Usage: a.out ipfix_filename\n");
        return -1;
    }

    memset(&msg_hdr, 0, sizeof(msg_hdr));
    fd = fopen(argv[1], "r");
    
    if (fread((void *)&msg_hdr, sizeof(ipfix_msg_hdr_t), 1, fd) > 0) {

     /* calculate the bytes to read for a record */
     msg_hdr.len = ntohs(msg_hdr.len);
     rec_buf = calloc(1, msg_hdr.len);
     memcpy(rec_buf, &msg_hdr, sizeof(msg_hdr));
     tmp = rec_buf+sizeof(msg_hdr);
     bytes2read = msg_hdr.len - sizeof(msg_hdr);

     /* Now read the whole set excluding msg hdr which is already read */
     fread((void *)tmp, bytes2read, 1, fd);
     decode_ipfix_msg(rec_buf);

    } else {
        printf("File read error\n");
    }
      
    return 0;
}
