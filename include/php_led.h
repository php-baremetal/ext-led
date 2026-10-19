/*
   +----------------------------------------------------------------------+
   | php-baremetal: ext-led                                               |
   +----------------------------------------------------------------------+
   | Copyright (c) 2026 Gianfrancesco Aurecchia                           |
   +----------------------------------------------------------------------+
   | This source file is subject to the MIT license, that is bundled with |
   | this package in the file LICENSE.                                    |
   +----------------------------------------------------------------------+
   | Authors: Gianfrancesco Aurecchia <gianfri@aurecchia.com>             |
   +----------------------------------------------------------------------+
*/

/*
 * Internal glue for the led extension: the base class entry, the object wrapper (an addressable-LED
 * strip handle) and the registration entry points called from MINIT.
 */
#pragma once

#include "php.h"
#include "bm_ledstrip_hal.h"
#include "led_driver_desc.h"

extern zend_module_entry led_module_entry;

/* Baremetal\Led\Device -- the shared base; every driver class extends it. */
extern zend_class_entry *led_device_ce;

typedef struct {
    bm_ledstrip_t     *strip;    /* the strip handle, freed with the object */
    uint32_t           count;    /* number of pixels */
    int                channels; /* 3 (RGB) | 4 (RGBW) */
    zend_object        std;
} led_object;

static inline led_object *led_object_from(zend_object *o)
{
    return (led_object *)((char *)o - XtOffsetOf(led_object, std));
}

/* Allocate a Device-shaped object (used by the base and every driver class). */
zend_object *led_object_create(zend_class_entry *ce);

void led_capability_register(void);      /* Baremetal\Output\Led */
void led_device_class_register(void);     /* Baremetal\Led\Device + shared methods */
void led_driver_classes_register(void);   /* Baremetal\Led\Driver\* from the descriptor table */

/* Shared driver constructor: (int $pin, int $count = 1, ?int $order = null). Builds the strip from
 * the descriptor and binds it to $this. Each driver's __construct forwards here with its descriptor. */
void led_ctor(INTERNAL_FUNCTION_PARAMETERS, const led_driver_desc_t *desc);
