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
 * Neutral addressable-LED-strip HAL. led_core.c uses only these; a bm-hal package for the platform
 * implements them. The wire order uses led_driver_desc.h's LED_ORDER_*.
 */
#pragma once
#include <stdint.h>

/* Neutral LED models, mapped to the platform's strip driver at construction. */
enum { BM_LED_MODEL_WS2812, BM_LED_MODEL_SK6812 };

typedef struct bm_ledstrip bm_ledstrip_t;
bm_ledstrip_t *bm_ledstrip_new(int pin, uint32_t count, int model, int order);   /* NULL on failure */
void bm_ledstrip_del(bm_ledstrip_t *s);
int  bm_ledstrip_set_pixel(bm_ledstrip_t *s, uint32_t i, uint8_t r, uint8_t g, uint8_t b);
int  bm_ledstrip_set_pixel_rgbw(bm_ledstrip_t *s, uint32_t i, uint8_t r, uint8_t g, uint8_t b, uint8_t w);
int  bm_ledstrip_refresh(bm_ledstrip_t *s);
int  bm_ledstrip_clear(bm_ledstrip_t *s);
