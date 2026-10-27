/*
   +----------------------------------------------------------------------+
   | php-baremetal: ext-led                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2026 Gianfrancesco Aurecchia                           |
   +----------------------------------------------------------------------+
   | This source file is subject to the MIT license, that is bundled with |
   | this package in the file LICENSE.                                    |
   +----------------------------------------------------------------------+
   | Authors: Gianfrancesco Aurecchia <gianfri@php-baremetal.com>         |
   +----------------------------------------------------------------------+
*/

/*
 * led_driver_desc_t -- what a LED driver fills in: one descriptor per chip (WS2812, SK6812, ...) naming the
 * PHP class, the HAL LED model and the colour layout. led_core.c does the rest.
 */
#pragma once

#include <stddef.h>

/* PHP function table; only the pointer is used here, so the engine headers are not pulled in. */
typedef struct _zend_function_entry zend_function_entry;

/* Colour byte order on the wire (passed to the LED-strip HAL at construction). */
enum {
    LED_ORDER_GRB  = 0,   /* standard WS2812 */
    LED_ORDER_RGB  = 1,   /* some onboard LEDs */
    LED_ORDER_GRBW = 2,   /* standard SK6812 RGBW */
    LED_ORDER_RGBW = 3,
};

typedef struct led_driver_desc {
    const char *name;          /* driver name in the led_drivers registry, e.g. "ws2812" */
    const char *php_class;     /* class under Baremetal\Led\Driver, e.g. "Ws2812" */
    int         model;         /* BM_LED_MODEL_* (WS2812 / SK6812) */
    int         channels;      /* 3 (RGB) or 4 (RGBW) */
    int         default_order; /* LED_ORDER_* used when the constructor's $order is null */

    const zend_function_entry *methods;   /* the driver class's own methods (its __construct) */
} led_driver_desc_t;

/* The led_drivers registry, generated from the driver packages in the project. */
extern const led_driver_desc_t *const led_drivers[];
extern const size_t led_drivers_count;
