/*
 * Copyright (c) 2018 Plume Design Inc.
 * All rights reserverd.
 * Author (s): Arud selvan
 * 
 */
#ifndef __IPFIXHDR_H__
#define __IPFIXHDR_H__

#include <stdint.h>

/*
 * IPFIX Message Header Structure RFC - 7011 section 3.1
 */
typedef struct ipfix_msg_hdr_ {
    uint16_t ver;
    uint16_t len;
    uint32_t export_time;
    uint32_t seq;
    uint32_t domain_id;   
    uint8_t sets[0];  /* Template record sets/Options Template record sets/Data sets */
} ipfix_msg_hdr_t;

/*
 * SET Header Format RFC - 7011 section 3.3.2
 */
typedef struct ipfix_set_hdr_ {
    uint16_t id;
    uint16_t len;
    uint8_t data[0]; /* Template/Options Template/
                      * Data - 
                      * ipfix_template_rec_set_t
                      * ipfix_opt_template_rec_set_t
                      * ipfix_template_rec_set_t
                      *
                      */
} ipfix_set_hdr_t;

/*
 * Field Specifier Format RFC - 7011 section 3.2
 */
typedef struct ipfix_field_specifier_fmt_ {
    uint16_t id;  /* bit 0 represents standard or enterprise field; 
                   * value 0 - standard ; value 1  - enterprise  
                   */
    union {
        struct {
            uint16_t len;
        } std;
        struct {
           uint16_t len;
           uint32_t num;
        } enterp;
    } u;
} ipfix_field_specifier_fmt_t;

#define IS_ENT_SET(id) ((id) & (0x01))

/*
 * Template Record Format RFC - 7011 section 3.4.1
 */
typedef struct ipfix_tmpl_rec_hdr_ {
    uint16_t id;
    uint16_t field_cnt;
} ipfix_tmpl_rec_hdr_t;

typedef struct ipfix_tmpl_rec_set_ {
    ipfix_set_hdr_t   set_hdr;
    ipfix_tmpl_rec_hdr_t tmpl_hdr;
    ipfix_field_specifier_fmt_t fields[0];
} ipfix_tmpl_rec_set_t;

/*
 * Options Template Record Format RFC - 7011 section 3.4.2.2
 */
typedef struct ipfix_opt_tmpl_rec_hdr_ {
    uint16_t id;
    uint16_t field_cnt;
    uint16_t scope_field_cnt;
} ipfix_opt_tmpl_rec_hdr_t;

typedef struct ipfix_opt_tmpl_rec_set_ {
    ipfix_set_hdr_t   set_hdr;
    ipfix_opt_tmpl_rec_hdr_t tmpl_hdr;
    ipfix_field_specifier_fmt_t fields[0];
} ipfix_opt_tmpl_rec_set_t;

/*
 * Data Record Format RFC - 7011 section 3.4.3
 */
typedef struct ipfix_data_rec_set_ {
    ipfix_set_hdr_t   set_hdr;
    uint8_t fields[0];
} ipfix_data_rec_set_t;

#endif
