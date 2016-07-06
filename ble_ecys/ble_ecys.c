/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/* Attention! 
*  To maintain compliance with Nordic Semiconductor ASA’s Bluetooth profile 
*  qualification listings, this section of source code must not be modified.
*/

#include "ble_ecys.h"
#include <string.h>
#include "ble_srv_common.h"


//#define INITIAL_RANDOM_NUMBER              0x1199
#define ENCRY_UUID_BASE									 {0x2B,0xF0,0xEC,0xBE,0xC6,0x3B,0xE4,0xBB,0x55,0x40,0xEA,0x92,0x00,0x00,0xA1,0xD6}
#define RANDNUM_UUID_BASE								 {0x44,0x85,0x6F,0x19,0x9D,0x82,0xD2,0xAE,0x4A,0x4B,0x3E,0x65,0x00,0x00,0x6E,0x7D}
#define ECYS_SERVICE_SHORT_UUID 					 0xD79C
#define RANDNUM_CHAR_SHORT_UUID 					 0xDF90
#define RANDOM_NUMBER_LENGTH							 4

static const uint8_t p_initial_randomnum[4] = {0x5C,0x21,0x34,0xC1};
static uint32_t ecys_debug;

//static uint32_t RN_uint32_big_decode(const ble_gatts_evt_write_t * p_evt)
//{
//    return ( (((uint32_t) p_evt->data[0]) << 24) |
//             (((uint32_t) p_evt->data[1]) << 16) |
//             (((uint32_t) p_evt->data[2]) << 8)  |
//             (((uint32_t) p_evt->data[3]) << 0) );
//}

/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_ecys       Immediate Alert Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_ecys_t * p_ecys, ble_evt_t * p_ble_evt)
{
    p_ecys->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}

/**@brief Function for handling the Write event.
 *
 * @param[in]   p_ecys       Immediate Alert Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_write(ble_ecys_t * p_ecys, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if ((p_evt_write->handle == p_ecys->random_number_handles.value_handle) && (p_evt_write->len == RANDOM_NUMBER_LENGTH))
    {
        // Alert level written, call application event handler
        ble_ecys_evt_t evt;

        evt.evt_type           = BLE_ECYS_EVT_RANDOM_NUMBER_UPDATED;
        evt.params.random_number = p_evt_write->data[0];

        p_ecys->evt_handler(p_ecys, &evt);
    }
}


void ble_ecys_on_ble_evt(ble_ecys_t * p_ecys, ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_ecys, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_ecys, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for adding Alert Level characteristics.
 *
 * @param[in]   p_ecys        Immediate Alert Service structure.
 * @param[in]   p_ecys_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t alert_level_char_add(ble_ecys_t * p_ecys,ble_uuid_t input_char_uuid)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    //ble_uuid_t          char_uuid;
    ble_gatts_attr_md_t attr_md;
//    uint32_t             initial_random_number;

    memset(&char_md, 0, sizeof(char_md));

    //char_md.char_props.write_wo_resp = 1;
		char_md.char_props.write         = 1;
		char_md.char_props.read          = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    //BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_ALERT_LEVEL_CHAR);
		
    memset(&attr_md, 0, sizeof(attr_md));

    // read_perm just for test
		//BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.write_perm);


    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));
    //initial_random_number = INITIAL_RANDOM_NUMBER;
		uint8_t initial_encoded_random_number[RANDOM_NUMBER_LENGTH];
		memcpy(initial_encoded_random_number,p_initial_randomnum,RANDOM_NUMBER_LENGTH);
//		uint32_big_encode(INITIAL_RANDOM_NUMBER,initial_encoded_random_number);
		
//		char_uuid.type = BLE_UUID_TYPE_VENDOR_BEGIN;
//		char_uuid.uuid = ECYS_SERVICE_SHORT_UUID;
		
		attr_char_value.p_uuid    = &input_char_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof (uint32_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof (uint32_t);
    attr_char_value.p_value   = initial_encoded_random_number;

    return sd_ble_gatts_characteristic_add(p_ecys->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ecys->random_number_handles);
}

static ble_uuid_t ble_uuid_test;
uint32_t ble_ecys_init(ble_ecys_t * p_ecys, const ble_ecys_init_t * p_ecys_init)
{
    uint32_t   err_code;
    //ble_uuid_t ble_uuid;
		ble_uuid_t service_uuid;
		ble_uuid128_t base_uuid = ENCRY_UUID_BASE;  // It's invert added from the array sequence, uint8_t [16] array
		
    // Initialize service structure
    if (p_ecys_init->evt_handler == NULL)
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    p_ecys->evt_handler = p_ecys_init->evt_handler;

    // Add service
    //BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_IMMEDIATE_ALERT_SERVICE);
		err_code = sd_ble_uuid_vs_add(&base_uuid, &ble_uuid_test.type); // add to Nordic VS UUID table
		service_uuid.type = ble_uuid_test.type;
		service_uuid.uuid = ECYS_SERVICE_SHORT_UUID; // assign short UUID		
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &service_uuid,
                                        &p_ecys->service_handle);
		ecys_debug = err_code;
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
		
		ble_uuid_t char_uuid;
		ble_uuid128_t base2_uuid = RANDNUM_UUID_BASE;
		err_code = sd_ble_uuid_vs_add(&base2_uuid, &ble_uuid_test.type);
		ecys_debug = err_code;
		//char_uuid.type = BLE_UUID_TYPE_VENDOR_BEGIN;
		char_uuid.type = ble_uuid_test.type;
		char_uuid.uuid = RANDNUM_CHAR_SHORT_UUID;
		ecys_debug +=0;
    // Add alert level characteristic
    return alert_level_char_add(p_ecys,char_uuid);
		//return 0;
}


uint32_t ble_ecys_random_number_get(ble_ecys_t * p_ecys, uint8_t * p_random_number)
{
    ble_gatts_value_t gatts_value;

    // Initialize value struct.
    memset(&gatts_value, 0, sizeof(gatts_value));

    gatts_value.len     = RANDOM_NUMBER_LENGTH;
    gatts_value.offset  = 0;
    gatts_value.p_value = p_random_number;

    return sd_ble_gatts_value_get(p_ecys->conn_handle,
                                  p_ecys->random_number_handles.value_handle,
                                  &gatts_value);
}
