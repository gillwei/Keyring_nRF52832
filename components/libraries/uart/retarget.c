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

#if !defined(NRF_LOG_USES_RTT) || NRF_LOG_USES_RTT != 1
#if !defined(HAS_SIMPLE_UART_RETARGET)

#include <stdio.h>
#include <stdint.h>
#include "app_uart.h"
#include "nordic_common.h"
#include "nrf_error.h"

#if !defined(__ICCARM__)
struct __FILE 
{
    int handle;
};
#endif

FILE __stdout;
FILE __stdin;


#if defined(__CC_ARM) ||  defined(__ICCARM__)
int fgetc(FILE * p_file)
{
    uint8_t input;
    while (app_uart_get(&input) == NRF_ERROR_NOT_FOUND)
    {
        // No implementation needed.
    }
    return input;
}


int fputc(int ch, FILE * p_file)
{
    UNUSED_PARAMETER(p_file);
    uint32_t err_code;
    do
    {
         err_code = app_uart_put((uint8_t)ch);
    }
    while(err_code);

    return ch;
}

#elif defined(__GNUC__)


int _write(int file, const char * p_char, int len)
{
    int i;

    UNUSED_PARAMETER(file);

    for (i = 0; i < len; i++)
    {
        UNUSED_VARIABLE(app_uart_put(*p_char++));
    }

    return len;
}


int _read(int file, char * p_char, int len)
{
    UNUSED_PARAMETER(file);
    while (app_uart_get((uint8_t *)p_char) == NRF_ERROR_NOT_FOUND)
    {
        // No implementation needed.
    }

    return 1;
}
#endif

#if defined(__ICCARM__)

__ATTRIBUTES size_t __write(int file, const unsigned char * p_char, size_t len)
{
    int i;

    UNUSED_PARAMETER(file);

    for (i = 0; i < len; i++)
    {
        UNUSED_VARIABLE(app_uart_put(*p_char++));
    }

    return len;
}

#endif
#endif // !defined(HAS_SIMPLE_UART_RETARGET)
#endif // NRF_LOG_USES_RTT != 1
