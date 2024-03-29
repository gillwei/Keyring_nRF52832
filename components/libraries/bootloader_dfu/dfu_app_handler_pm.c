/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
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

#include <string.h>
#include "dfu_app_handler_pm.h"
#include "bootloader_util.h"
#include "nrf.h"
#include "nrf_sdm.h"
#include "ble_gatt.h"
#include "ble_gatts.h"
#include "app_error.h"
#include "dfu_ble_svc.h"
#include "peer_manager.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "id_manager.h"

#define IRQ_ENABLED            0x01                                     /**< Field that identifies if an interrupt is enabled. */
#define MAX_NUMBER_INTERRUPTS  32                                       /**< Maximum number of interrupts available. */

static void                    dfu_app_reset_prepare(uint16_t m_conn_handle);             /**< Forward declaration of default reset handler. */
static dfu_app_reset_prepare_t m_reset_prepare = dfu_app_reset_prepare; /**< Callback function to application to prepare for system reset. Allows application to clean up service and memory before reset. */
static dfu_ble_peer_data_t     m_peer_data;                             /**< Peer data to be used for data exchange when resetting into DFU mode. */



/**@brief Function for reset_prepare handler if the application has not registered a handler.
 */
static void dfu_app_reset_prepare(uint16_t m_conn_handle)
{
    // Reset prepare should be handled by application.
    // This function can be extended to include default handling if application does not implement
    // own handler.
}


/**@brief Function for disabling all interrupts before jumping from bootloader to application.
 */
static void interrupts_disable(void)
{
    uint32_t interrupt_setting_mask;
    uint32_t irq;

    // Fetch the current interrupt settings.
    interrupt_setting_mask = NVIC->ISER[0];

    // Loop from interrupt 0 for disabling of all interrupts.
    for (irq = 0; irq < MAX_NUMBER_INTERRUPTS; irq++)
    {
        if (interrupt_setting_mask & (IRQ_ENABLED << irq))
        {
            // The interrupt was enabled, hence disable it.
            NVIC_DisableIRQ((IRQn_Type)irq);
        }
    }
}


/**@brief Function for providing peer information to DFU for re-establishing a bonded connection in
 *        DFU mode.
 *
 * @param[in] conn_handle   Connection handle for the connection requesting DFU mode.
 */
static void dfu_app_peer_data_set(uint16_t conn_handle)
{
    uint32_t                 err_code;
    pm_peer_id_t             peer_id;
    pm_peer_data_bonding_t   peer_data_bonding;

    err_code = pm_peer_id_get(conn_handle,&peer_id);
    APP_ERROR_CHECK(err_code);
    
    err_code = pm_peer_data_bonding_load(peer_id, &peer_data_bonding);
    if(err_code == NRF_SUCCESS)
    {        
        m_peer_data.addr              = peer_data_bonding.peer_id.id_addr_info;
        m_peer_data.irk               = peer_data_bonding.peer_id.id_info;
        m_peer_data.enc_key.enc_info  = peer_data_bonding.own_ltk.enc_info; 
        m_peer_data.enc_key.master_id = peer_data_bonding.own_ltk.master_id;
              
        err_code = dfu_ble_svc_peer_data_set(&m_peer_data);
        APP_ERROR_CHECK(err_code);
        
    }
    else
    {
        err_code = im_ble_addr_get(conn_handle,&m_peer_data.addr);
        APP_ERROR_CHECK(err_code);
        
        err_code = dfu_ble_svc_peer_data_set(&m_peer_data);
        APP_ERROR_CHECK(err_code);      
    }
    

}


/**@brief Function for preparing the reset, disabling SoftDevice, and jumping to the bootloader.
 *
 * @param[in] conn_handle Connection handle for peer requesting to enter DFU mode.
 */
static void bootloader_start(uint16_t conn_handle)
{
    uint32_t err_code;
    uint16_t sys_serv_attr_len = sizeof(m_peer_data.sys_serv_attr);

    err_code = sd_ble_gatts_sys_attr_get(conn_handle,
                                         m_peer_data.sys_serv_attr,
                                         &sys_serv_attr_len,
                                         BLE_GATTS_SYS_ATTR_FLAG_SYS_SRVCS);
    if (err_code != NRF_SUCCESS)
    {
        // Any error at this stage means the system service attributes could not be fetched.
        // This means the service changed indication cannot be sent in DFU mode, but connection
        // is still possible to establish.
    }

    m_reset_prepare(conn_handle);

    err_code = sd_power_gpregret_set(BOOTLOADER_DFU_START);
    APP_ERROR_CHECK(err_code);

    err_code = sd_softdevice_disable();
    APP_ERROR_CHECK(err_code);

    err_code = sd_softdevice_vector_table_base_set(NRF_UICR->NRFFW[0]);
    APP_ERROR_CHECK(err_code);

    dfu_app_peer_data_set(conn_handle);

    NVIC_ClearPendingIRQ(SWI2_IRQn);
    interrupts_disable();
    bootloader_util_app_start(NRF_UICR->NRFFW[0]);
}


void dfu_app_on_dfu_evt(ble_dfu_t * p_dfu, ble_dfu_evt_t * p_evt)
{
    switch (p_evt->ble_dfu_evt_type)
    {
        case BLE_DFU_START:
            // Starting the bootloader - will cause reset.
            bootloader_start(p_dfu->conn_handle);
            break;

        default:
            {
                // Unsupported event received from DFU Service. 
                // Send back BLE_DFU_RESP_VAL_NOT_SUPPORTED message to peer.
                uint32_t err_code = ble_dfu_response_send(p_dfu,
                                                          BLE_DFU_START_PROCEDURE,
                                                          BLE_DFU_RESP_VAL_NOT_SUPPORTED);
                APP_ERROR_CHECK(err_code);
            }
            break;
    }
}


void dfu_app_reset_prepare_set(dfu_app_reset_prepare_t reset_prepare_func)
{
    m_reset_prepare = reset_prepare_func;
}

