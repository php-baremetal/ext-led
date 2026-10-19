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
 * led extension module: at MINIT it registers the Baremetal\Output\Led capability, the
 * Baremetal\Led\Device base and the registered driver classes (Baremetal\Led\Driver\*).
 */
#include "php_led.h"

PHP_MINIT_FUNCTION(led)
{
    led_capability_register();
    led_device_class_register();
    led_driver_classes_register();
    return SUCCESS;
}

static const zend_function_entry led_functions[] = {
    PHP_FE_END
};

zend_module_entry led_module_entry = {
    STANDARD_MODULE_HEADER,
    "led",
    led_functions,
    PHP_MINIT(led),
    NULL,   /* MSHUTDOWN */
    NULL,   /* RINIT */
    NULL,   /* RSHUTDOWN */
    NULL,   /* MINFO */
    "1.0",
    STANDARD_MODULE_PROPERTIES
};
