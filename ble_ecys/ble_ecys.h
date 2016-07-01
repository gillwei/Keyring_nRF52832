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

/** @file
 *
 * @defgroup ble_sdk_srv_ecys Immediate Alert Service
 * @{
 * @ingroup ble_sdk_srv
 * @brief Immediate Alert Service module.
 *
 * @details This module implements the Immediate Alert Service with the Alert Level characteristic.
 *          During initialization it adds the Immediate Alert Service and Alert Level characteristic
 *          to the BLE stack database.
 *
 *          The application must supply an event handler for receiving Immediate Alert Service 
 *          events. Using this handler, the service will notify the application when the 
 *          Alert Level characteristic value changes.
 *
 *          The service also provides a function for letting the application poll the current
 *          value of the Alert Level characteristic.
 *
 * @note The application must propagate BLE stack events to the Immediate Alert Service
 *       module by calling ble_ecys_on_ble_evt() from the @ref softdevice_handler callback.
 *
 * @note Attention! 
 *  To maintain compliance with Nordic Semiconductor ASA Bluetooth profile 
 *  qualification listings, this section of source code must not be modified.
*/

//#ifndef BLE_ECYS_H__
//#define BLE_ECYS_H__

#include <stdint.h>
#include "ble.h"

/**@brief Immediate Alert Service event type. */
typedef enum
{
    BLE_ECYS_EVT_RANDOM_NUMBER_UPDATED                     /**< Alert Level Updated event. */
} ble_ecys_evt_type_t;

/**@brief Immediate Alert Service event. */
typedef struct
{
    ble_ecys_evt_type_t evt_type;                        /**< Type of event. */
    union
    {
        uint32_t random_number;                            /**< New Alert Level value. */
    } params;
} ble_ecys_evt_t;

// Forward declaration of the ble_ecys_t type. 
typedef struct ble_ecys_s ble_ecys_t;

/**@brief Immediate Alert Service event handler type. */
typedef void (*ble_ecys_evt_handler_t) (ble_ecys_t * p_ecys, ble_ecys_evt_t * p_evt);

/**@brief Immediate Alert Service init structure. This contains all options and data needed for
 *        initialization of the service. */
typedef struct
{
    ble_ecys_evt_handler_t evt_handler;                  /**< Event handler to be called for handling events in the Immediate Alert Service. */
} ble_ecys_init_t;

/**@brief Immediate Alert Service structure. This contains various status information for the
 *        service. */
struct ble_ecys_s
{
    ble_ecys_evt_handler_t     evt_handler;              /**< Event handler to be called for handling events in the Immediate Alert Service. */
    uint16_t                  service_handle;           /**< Handle of Immediate Alert Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t  random_number_handles;      /**< Handles related to the Alert Level characteristic. */
    uint16_t                  conn_handle;              /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
};

/**@brief Function for initializing the Immediate Alert Service.
 *
 * @param[out]  p_ecys       Immediate Alert Service structure. This structure will have to be
 *                          supplied by the application. It will be initialized by this function,
 *                          and will later be used to identify this particular service instance.
 * @param[in]   p_ecys_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_ecys_init(ble_ecys_t * p_ecys, const ble_ecys_init_t * p_ecys_init);

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Immediate Alert Service.
 *
 * @param[in]   p_ecys      Immediate Alert Service structure.
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 */
void ble_ecys_on_ble_evt(ble_ecys_t * p_ecys, ble_evt_t * p_ble_evt);

/**@brief Function for getting current value of the Alert Level characteristic.
 *
 * @param[in]   p_ecys          Immediate Alert Service structure.
 * @param[out]  p_random_number  Current Alert Level value.
 */
uint32_t ble_ecys_random_number_get(ble_ecys_t * p_ecys, uint8_t * p_random_number);

//#endif // BLE_ECYS_H__

/** @} */
