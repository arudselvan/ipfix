/*
 * Copyright (c) 2018 Plume Design Inc.
 * All rights reserverd.
 * Author (s): Arud selvan
 * 
 */
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include "ipFixHdr.h"

/*
 * For simple approach just get the ipfix packet decode it and print it.
 * TDB: storing of templates and decoding of data as per templates
 */
static ipfix_tmpl_rec_set_t latest_template;
 
/* TBD: Template DB */
#define MAX_DB_SIZE (8 * 1024)
ipfix_tmpl_rec_set_t tmplDBSets[MAX_DB_SIZE];
uint32_t db_free_space = MAX_DB_SIZE;
typedef struct ie_id_ {
    char *name;
    uint32_t id;
    uint32_t len;
} ie_id_t;
#if 0
static ie_id_t IE_Id_DB[] = {
    /* IE_ID Protocol Id */
    {"Protocol Id", 4, 8},
    /* IE_ID SRC_PORT */
    {"Src Port", 7, 16},
    /* IE_ID SRC_ADDR */
    {"Src Addr", 8, 32},
    /* IE_ID DEST_PORT */
    {"Dst Port", 11, 16},
    /* IE_ID DEST_ADDR */
    {"Dst Addr", 12, 32}
};
#endif

static int8_t decode_ipfix_msg_hdr(ipfix_msg_hdr_t *hdr);
static int8_t decode_ipfix_set_hdr(ipfix_set_hdr_t *hdr);
static uint16_t decode_ipfix_tmpl_rec_set(ipfix_tmpl_rec_set_t *rec);
static uint16_t decode_ipfix_tmpl_field_spec(ipfix_field_specifier_fmt_t *field, int cnt);
static uint16_t decode_ipfix_data_rec_set(ipfix_data_rec_set_t *data);



int8_t decode_ipfix_msg (char *msg) {

    ipfix_msg_hdr_t                *ipfix_hdr = NULL;
    ipfix_set_hdr_t                *ipfix_set_hdr = NULL;
    ipfix_tmpl_rec_set_t       *ipfix_tmpl_rec = NULL;
    ipfix_data_rec_set_t           *ipfix_data_rec = NULL;
    uint32_t  bytes_to_parse = 0;
    uint32_t  bytes_parsed = 0;
    uint32_t  bytes_to_copy = 0;
    /*
    uint32_t  bytes_to_dbcopy = 0;
    ipfix_opt_tmpl_rec_set_t   *ipfix_opt_tmpl_rec = NULL;
    */
    ipfix_hdr = (ipfix_msg_hdr_t *)msg;
    decode_ipfix_msg_hdr(ipfix_hdr);
    ipfix_set_hdr = (ipfix_set_hdr_t *)&ipfix_hdr->sets[0];
    bytes_to_parse = ipfix_set_hdr->len - sizeof(ipfix_set_hdr_t);
  
    while ( bytes_to_parse > 0 ) { 
        bytes_parsed += decode_ipfix_set_hdr(ipfix_set_hdr);
        if (ipfix_set_hdr->id == 2 ) {  /* Template set */
            ipfix_tmpl_rec = (ipfix_tmpl_rec_set_t *)ipfix_set_hdr;
            bytes_parsed += decode_ipfix_tmpl_rec_set(ipfix_tmpl_rec);
            latest_template = *ipfix_tmpl_rec;
            /* copy to Template DB */
            bytes_to_copy = sizeof(ipfix_tmpl_rec_set_t) + \
                            ipfix_tmpl_rec->tmpl_hdr.field_cnt * \
                            sizeof(ipfix_field_specifier_fmt_t);
             if (bytes_to_copy <= db_free_space) {
                 memcpy(tmplDBSets, ipfix_tmpl_rec, bytes_to_copy);
                 db_free_space -= bytes_to_copy;
             } else {
                 printf("Error: No enough memory in DB\n");
             }
        } else if (ipfix_set_hdr->id == 3) { /* Options Template set */
             ipfix_tmpl_rec = (ipfix_tmpl_rec_set_t *)ipfix_set_hdr;
             /* TDB: decode options tmpl */         
        } else if (ipfix_set_hdr->id >= 256) {
            ipfix_data_rec = (ipfix_data_rec_set_t *)ipfix_set_hdr;
            bytes_parsed += decode_ipfix_data_rec_set(ipfix_data_rec);
        } else {
             printf ("Error: unknown set header\n");
        }
       bytes_to_parse -= bytes_parsed;
    }
    return 0;
}

int8_t decode_ipfix_msg_hdr(ipfix_msg_hdr_t *hdr) {

    hdr->ver = ntohs(hdr->ver);
    hdr->len = ntohs(hdr->len);
    hdr->export_time = ntohl(hdr->export_time);
    hdr->seq = ntohs(hdr->seq);
    hdr->domain_id = ntohl(hdr->domain_id);
    return 0;
}

int8_t decode_ipfix_set_hdr(ipfix_set_hdr_t *hdr) {

    hdr->id  = ntohs(hdr->id);
    hdr->len = ntohs(hdr->len); 
    return 0;
}

uint16_t decode_ipfix_tmpl_rec_set(ipfix_tmpl_rec_set_t *rec) {
   
    int bytes_parsed = 0;
    rec->tmpl_hdr.id = ntohs(rec->tmpl_hdr.id);
    rec->tmpl_hdr.field_cnt = ntohs(rec->tmpl_hdr.field_cnt);
    bytes_parsed += sizeof(ipfix_tmpl_rec_set_t); 
    /* decode fields */
    bytes_parsed += decode_ipfix_tmpl_field_spec(&rec->fields[0], rec->tmpl_hdr.field_cnt); 
    return bytes_parsed;
}


uint16_t decode_ipfix_tmpl_field_spec(ipfix_field_specifier_fmt_t *field, int cnt) {
  
    int i = 0;
    int bytes_parsed = 0;
    for (; cnt > 0; cnt--, i++) {
    
        field[i].id = ntohs(field[i].id);
        bytes_parsed += sizeof(field[i].id);
        if (IS_ENT_SET(field[i].id)) {
            field[i].u.enterp.len = ntohs(field[i].u.enterp.len);
            bytes_parsed += sizeof(field[i].u.enterp.len);
            field[i].u.enterp.num = ntohl(field[i].u.enterp.num);
            bytes_parsed += sizeof(field[i].u.enterp.num);
        } else {
            field[i].u.std.len = ntohs(field[i].u.std.len);
            bytes_parsed += sizeof(field[i].u.std.len);
        }
    }
    return bytes_parsed;
}

uint16_t decode_ipfix_data_rec_set(ipfix_data_rec_set_t *data) {
    
    /* Print the 5-tuples of present in each record of data set */
    int bytes_parsed = 0;
    int fields_per_rec = 0;
    int i = 0;
    int rec_len = 0;
    int rec_cnt = 0;
    uint16_t ie_id = 0;
    uint16_t proto_id = 0;
    uint16_t src_port = 0, dst_port = 0;
    uint32_t src_addr = 0, dst_addr = 0;
    ipfix_field_specifier_fmt_t *field = NULL;
    uint8_t *rec_val = NULL;
    uint16_t field_len = 0;

    data->set_hdr.id = ntohs(data->set_hdr.id);
    data->set_hdr.len = ntohs(data->set_hdr.len);
    bytes_parsed += sizeof(data->set_hdr);

    /* TBD: Need to send the correct tmpl for decoding by finding it from DB*/

    /* Now use the latest template for decoding*/
    fields_per_rec = latest_template.tmpl_hdr.field_cnt;
    field = &latest_template.fields[0];

    rec_len = data->set_hdr.len - sizeof(data->set_hdr);
    /* for all records in data set */
    rec_val = &data->fields[0];
    while (rec_len > 0) {
        /* decode the fields in records using template*/
        for (i = 0; i < fields_per_rec && rec_len > 0; i++) {
           
            ie_id = field[i].id >> 1;
            if (IS_ENT_SET(field[i].id)) {
                field_len -= field[i].u.enterp.len;
            } else {
                field_len -= field[i].u.std.len;
            }
            /* In all the following assume incomming len is <= data type */
            switch (ie_id) {
                case 4: /* Protocol Id */
                    memcpy(&proto_id, rec_val, field_len);
                    proto_id = ntohs(proto_id);
                    rec_val += field_len;
                break;
                case 7: /* Src Port */
                    memcpy(&src_port, rec_val, field_len);
                    src_port = ntohs(src_port);
                    rec_val += field_len;
                break;
                case 8: /* Src Addr */
                    memcpy(&src_addr, rec_val, field_len);
                    src_addr = ntohs(src_addr);
                    rec_val += field_len;
                break;
                case 11: /* Dest Port */
                    memcpy(&dst_port, rec_val, field_len);
                    dst_port = ntohs(dst_port);
                    rec_val += field_len;
                break;
                case 12: /* Dest Addr */
                    memcpy(&dst_addr, rec_val, field_len);
                    dst_addr = ntohs(dst_addr);
                    rec_val += field_len;
                break;
                default:
                    rec_val += field_len;
                break;
            }
            rec_len -= field_len;
  
        }
        printf( "Decoded tuples of record[%d]: Protocol Id: [%d] Src Port: [%d] Src Addr: [%d] Dest Port: [%d] Dest Addr: [%d]\n", \
                 rec_cnt++,proto_id, src_port, src_addr, dst_port, dst_addr); 
    }


    return data->set_hdr.len;
}
