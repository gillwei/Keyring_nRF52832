/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
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
 * @defgroup pwm_example_main main.c
 * @{
 * @ingroup pwm_example
 * 
 * @brief  PWM Example Application main file.
 *
 * This file contains the source code for a sample application using PWM.
 *
 *
 */

#include "buzzer_pwm.h"

#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "app_error.h"
#include "bsp.h"
#include "nrf_delay.h"
#include "app_pwm.h"

APP_PWM_INSTANCE(PWM1,1);                   // Create the instance "PWM1" using TIMER1.

static volatile bool ready_flag;            // A flag indicating PWM status.

void pwm_ready_callback(uint32_t pwm_id)    // PWM callback function
{
    ready_flag = true;
}

#define PWM_PIN_CH1 17
#define PWM_PIN_CH2 18


void buzzer_init(void)
{
    ret_code_t err_code;
    
    /* 2-channel PWM, 200Hz, output on DK LED pins. */
    app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_2CH(250L, PWM_PIN_CH1, PWM_PIN_CH2);
    
    /* Switch the polarity of the second channel. */
    pwm1_cfg.pin_polarity[1] = APP_PWM_POLARITY_ACTIVE_HIGH;
    
    /* Initialize and enable PWM. */
    err_code = app_pwm_init(&PWM1,&pwm1_cfg,pwm_ready_callback);
    APP_ERROR_CHECK(err_code);

}

void buzzer_on(void)
{	
    uint32_t value;
		LEDS_OFF(BSP_LED_1_MASK);//turn on LED and sync with buzzer
		app_pwm_enable(&PWM1);
    value = 50;
    ready_flag = false;
    /* Set the duty cycle - keep trying until PWM is ready... */
    while (app_pwm_channel_duty_set(&PWM1, 0, value) == NRF_ERROR_BUSY);

//Tsungta	
//    /* ... or wait for callback. */
//    while(!ready_flag);
//    APP_ERROR_CHECK(app_pwm_channel_duty_set(&PWM1, 1, value));
//    nrf_delay_ms(25);
}

void buzzer_off(void)
{
		LEDS_ON(BSP_LED_1_MASK);//turn off LED and sync with buzzer
		app_pwm_disable(&PWM1);
}

/** @} */
