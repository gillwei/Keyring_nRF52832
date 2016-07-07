
#ifndef MAIN_H
#define MAIN_H

#include "ble_advdata.h"
void adc_configure(void);
void measure_VDD_start(void);
void measure_VDD_stop(void);
uint32_t keyring_advdata_encode(ble_advdata_t const * const p_advdata,
                         uint8_t             * const p_encoded_data,
                         uint16_t            *  const p_len);
uint32_t keyring_scrdata_encode(ble_advdata_t const * const p_advdata,
                         uint8_t             * const p_encoded_data,
                         uint16_t            *  const p_len);
	
#endif
