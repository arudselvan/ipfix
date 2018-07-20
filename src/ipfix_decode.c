/*
 * Copyright (c) 2018 Plume Design Inc.
 * All rights reserverd.
 * Author (s): Arud selvan
 * 
 */

#include <arpa/inet.h>

/*
 * For simple approach just get the ipfix packet decode it and print it.
 * TDB: storing of templates and decoding of data as per templates
 */
static ipfix_tmpl_rec_set_t latest_template;
 
/* TBD: Template DB */
#define MAX_DB_SIZE (8 * 1024)
ipfix_tmpl_rec_set_t tmplDBSets[MAX_DB_SIZE];
unint32_t db_free_space = MAX_DB_SIZE;
typedef struct ie_id_ {
    char *name;
    uint32_t id;
    uint32_t len;
} ie_id_t;

static ied_id_t IE_Id_DB[] = {
    /* IE_ID Protocol Id */
    {4, 8},
    /* IE_ID SRC_PORT */
    {7, 16},
    /* IE_ID SRC_ADDR */
    {8, 32},
    /* IE_ID DEST_PORT */
    {11, 16},
    /* IE_ID DEST_ADDR */
    {12, 32}
};

int8_t decode_ipfix_msg (char *msg) {

    ipfix_msg_hdr_t                *ipfix_hdr = NULL;
    ipfix_set_hdr_t                *ipfix_set_hdr = NULL;
    ipfix_tmpl_rec_set_t       *ipfix_tmpl_rec = NULL;
    ipfix_opt_tmpl_rec_set_t   *ipfix_opt_tmpl_rec = NULL;
    ipfix_data_rec_set_t           *ipfix_data_rec = NULL;
    unint32_t bytes_to_dbcopy = 0;
    uint32_t  bytes_to_parse = 0;
    uint32_t  bytes_parsed = 0;

    ipfix_hdr = (ipfix_msg_hdr_t *)msg;
    decode_ipfix_msg_hdr(ipfix_hdr);
    ipfix_set_hdr = ipfix_hdr->sets;
    bytes_to_parse = ipfix_set_hdr->len - sizeof(ipfix_set_hdr_t);
  
    while ( bytes_to_parse > 0 ) { 
        bytes_parsed += decode_ipfix_set_hdr(ipfix_set_hdr);
        if (ipfix_set_hdr->id == 2 ) {  /* Template set */
            ipfix_tmpl_rec = (ipfix_tmpl_rec_set_t *)ipfix_set_hdr;
            bytes_parsed += decode_ipfix_tmpl_rec_set(ipfix_tmpl_rec);
            latest_template = *ipfix_tmpl_rec;
            /* copy to Template DB */
            bytes_to_copy = sizeof(ipfix_tmpl_rec_set_t) + \
                            ipfix_tmpl_rec->tmpl_hdr->field_cnt * \
                            sizeof(ipfix_field_specifier_fmt_t);
             if (bytes_to_copy <= db_free_space) {
                 memcpy(tmplDBSets, bytes_to_copy);
                 db_free_space -= bytes_to_copy;
             } else {
                 printf("Error: No enough memory in DB\n");
             }
        } else if (ipfix_set_hdr->id == 3) { /* Options Template set */
             ipfix_tmpl_rec = (ipfix_opt_tmpl_rec_set_t *)ipfix_set_hdr;
             /* TDB: decode options tmpl */         
        } else if (ipfix_set_hdr->id >= 256) {
            ipfix_data_rec = (ipfix_data_rec_set_t *)ipfix_set_hdr;
            /* TBD: Need to send the correct tmpl for decoding */
            bytes_parsed += decode_ipfix_date_rec_set(&latest_template, ipfix_data_rec);
        } else {
             printf ("Error: unknown set header\n");
        }
       bytes_to_parse -= bytes_parsed;
    }
    return 0;
}

int8_t decode_ipfix_msg_hdr(ipfix_msg_hdr_t *hdr) {

    hdr->ver = htons(hdr->ver);
    hdr->len = htons(hdr->len);
    hdr->export_time = htonl(hdr->export_time);
    hdr->seq = htons(hdr->seq);
    hdr->domain_id = htonl(hdr->domain_id);
    return 0;
}

int8_t decode_ipfix_set_hdr(char *msg, ipfix_set_hdr_t *hdr) {

    hdr->id  = htons(hdr->id);
    hdr->len = htons(hdr->len); 
    return 0;
}

int decode_ipfix_tmpl_rec_set(ipfix_tmpl_rec_set_t *rec) {
   
    int i = 0;
    int bytes_parsed = 0;
    rec->tmpl_hdr->id = htons(rec->tmpl_hdr->id);
    rec->tmpl_hdr->field_count = htons(rec->tmpl_hdr->field_count);
    bytes_parsed += sizeof(ipfix_tmpl_rec_set_t); 
    /* decode fields /
    bytes_parsed += decode_ipfix_tmpl_field_spec(&rec->tmpl->fields[0], rec->tmpl_hdr->field_count); 
    return bytes_parsed;
}


int decode_ipfix_tmpl_field_spec(ipfix_field_specifier_fmt_t *field, int cnt) {
  
    int i = 0;
    int bytes_parsed = 0;
    for (; cnt > 0; cnt--, i++) {
    
        field[i].id = htons(filed[i].id);
        bytes_parsed += sizeof(field[i].id);
        if (IS_ENT_SET(field[i].id) {
            field[i].u.enterp.len = htons(filed[i].u.enterp.len);
            bytes_parsed += sizeof(field[i].u.enterp.len);
            field[i].u.enterp.num = htonl(filed[i].u.enterp.num);
            bytes_parsed += sizeof(field[i].u.enterp.num);
        } else {
            field[i].u.std.len = htons(filed[i].u.std.len);
            bytes_parsed += sizeof(field[i].u.std.len);
        }
    }
    return bytes_parsed;
}

int8_t decode_ipfix_date_rec_set(ipfix_tmpl_rec_set_t *tmpl, ipfix_data_rec_set_t *data) {

    return 0;
}
