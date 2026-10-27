---
eyebrow: 'ext-led · Drivers'
lede:    'A chip driver is a bm-lib package with one C file: a led_driver_desc_t naming the class, the LED model, the channel count and the default order. ext-led does the rest.'
see_also:
  - { href: './registry.md', meta: 'Drivers' }
  - { href: '../usage/device.md', meta: 'PHP API' }
  - { href: 'package:php-baremetal/led-ws2812', meta: 'Driver', label: 'led-ws2812 (a complete driver)' }
prev: { label: 'The led_drivers registry', href: './registry.md' }
next: { label: 'Status LED', href: '../recipes/status-led.md' }
---

# Writing a driver

A chip driver gives one LED type a PHP class. It is a `bm-lib` package that requires ext-led and registers a
descriptor symbol into the `led_drivers` registry. ext-led's shared code does the buffer, HSV and sending, so
the descriptor and a constructor are all a driver needs. The C file includes only `php_led.h`: no ESP-IDF.

The LED timing comes from the HAL, which knows two models: `BM_LED_MODEL_WS2812` and `BM_LED_MODEL_SK6812`. A
new chip must be compatible with one of them.

## The package

<!-- @code-block language="json" label="composer.json" -->
```json
{
  "name": "acme/led-xyz",
  "description": "XYZ RGBW LED driver for ext-led (Baremetal\\Led\\Driver\\Xyz).",
  "version": "1.0.0",
  "type": "bm-lib",
  "license": "MIT",
  "require": {
    "php": ">=8.3",
    "php-baremetal/ext-led": "^1.0"
  },
  "extra": {
    "baremetal": {
      "register": { "led_drivers": ["xyz_driver_desc"] }
    }
  }
}
```
<!-- @endcode-block -->

Sources default to `src/*.c`; no `CMakeLists.txt`. The symbol in `register` must be a non-static `const
led_driver_desc_t` defined in those sources.

## The C file

An RGBW chip with SK6812 timing in RGBW order, plus one method of its own that lights a pixel with its white
channel, which the shared methods always leave at 0.

<!-- @code-block language="c" label="src/xyz.c" -->
```c
#include "php_led.h"
#include "zend_exceptions.h"

extern const led_driver_desc_t xyz_driver_desc;

ZEND_BEGIN_ARG_INFO_EX(arginfo_xyz_ctor, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, pin, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, count, IS_LONG, 0, "1")
    ZEND_ARG_TYPE_INFO_WITH_DEFAULT_VALUE(0, order, IS_LONG, 1, "null")
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_xyz_white, 0, 2, IS_VOID, 0)
    ZEND_ARG_TYPE_INFO(0, i, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(Xyz, __construct)
{
    led_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU, &xyz_driver_desc);
}

/* white(int $i, int $level): pixel $i lit by its W channel only (R, G, B off), into the buffer */
PHP_METHOD(Xyz, white)
{
    zend_long i, level;
    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_LONG(i)
        Z_PARAM_LONG(level)
    ZEND_PARSE_PARAMETERS_END();

    led_object *o = led_object_from(Z_OBJ_P(ZEND_THIS));
    if (!o->strip) {
        zend_throw_exception(zend_ce_exception, "Xyz is not initialised", 0);
        RETURN_THROWS();
    }
    if (i < 0 || (uint32_t) i >= o->count) {
        zend_argument_value_error(1, "is out of range for this strip");
        RETURN_THROWS();
    }
    uint8_t w = level < 0 ? 0 : (level > 255 ? 255 : (uint8_t) level);
    bm_ledstrip_set_pixel_rgbw(o->strip, (uint32_t) i, 0, 0, 0, w);
}

static const zend_function_entry xyz_methods[] = {
    PHP_ME(Xyz, __construct, arginfo_xyz_ctor,  ZEND_ACC_PUBLIC)
    PHP_ME(Xyz, white,       arginfo_xyz_white, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

const led_driver_desc_t xyz_driver_desc = {
    .name = "xyz",
    .php_class = "Xyz",
    .model = BM_LED_MODEL_SK6812,
    .channels = 4,
    .default_order = LED_ORDER_RGBW,
    .methods = xyz_methods,
};
```
<!-- @endcode-block -->

PHP then has `Baremetal\Led\Driver\Xyz` with `white()`, every `Device` method and the constants `GRB`, `RGB`,
`GRBW`, `RGBW`. The led-ws2812 and led-sk6812 drivers are this file without the extra method.

<!-- @callout variant="warning" title="Always define __construct" -->
The method table must contain a `__construct` that calls `led_ctor()` with the descriptor. Without it the class
has no constructor that creates a strip, and every method throws `Led\Device is not initialised`.
<!-- @endcallout -->

## Descriptor fields

`led_driver_desc_t` is declared in `led_driver_desc.h`. ext-led 1.0 reads every field:

| Field | Meaning |
| --- | --- |
| `name` | Registry name, used in the constructor error `cannot init NAME on GPIO N`. |
| `php_class` | Short class name; the class is `Baremetal\Led\Driver\<php_class>`. Must be unique among installed drivers; the full name must fit in 95 characters. |
| `model` | `BM_LED_MODEL_WS2812` or `BM_LED_MODEL_SK6812` (from `bm_ledstrip_hal.h`): the timing the HAL uses. |
| `channels` | `3` (RGB) or `4` (RGBW). With 4, the class also gets `GRBW` and `RGBW`, and the shared methods write pixels as RGBW with W at 0. |
| `default_order` | `LED_ORDER_GRB`, `LED_ORDER_RGB`, `LED_ORDER_GRBW` or `LED_ORDER_RGBW`, used when `$order` is `null`. A 4-channel driver needs a 4-channel default. |
| `methods` | The class's method table (`zend_function_entry[]`), at least `__construct`. |

## Functions a driver may call

From `php_led.h` and `bm_ledstrip_hal.h`:

| Function | Does |
| --- | --- |
| `led_ctor(INTERNAL_FUNCTION_PARAM_PASSTHRU, &desc)` | The shared constructor: `(int $pin, int $count = 1, ?int $order = null)`, validates, creates and clears the strip, binds it to the object. |
| `led_object_from(Z_OBJ_P(ZEND_THIS))` | The object behind `$this`: `strip` (NULL before the constructor), `count`, `channels`. |
| `bm_ledstrip_set_pixel(s, i, r, g, b)`, `bm_ledstrip_set_pixel_rgbw(s, i, r, g, b, w)` | Write one pixel into the buffer. Return 0 on success. |
| `bm_ledstrip_refresh(s)`, `bm_ledstrip_clear(s)` | Send the buffer; clear it and send. |

All run on the PHP task.

## Try it in a project

Keep the driver next to the project as a path repository; conductor links it into `.phpbm/packages/`.

<!-- @code-block language="json" label="project composer.json" -->
```json
{
  "name": "acme/xyz-demo",
  "type": "project",
  "repositories": [
    { "type": "composer", "url": "https://repository.php-baremetal.com", "canonical": true },
    { "type": "path", "url": "extensions/led-xyz" }
  ],
  "require": {
    "php-baremetal/conductor": "^1.0",
    "acme/led-xyz": "^1.0"
  },
  "config": {
    "allow-plugins": { "php-baremetal/conductor": true },
    "platform": { "php": "8.5.11" }
  },
  "extra": {
    "baremetal": {
      "sdk": "esp32@^2.0",
      "board": "espressif-esp32-s3-devkitc-1-n8r8",
      "runtime": "init-loop",
      "storage": "embedded"
    }
  }
}
```
<!-- @endcode-block -->

<!-- @code-block language="php" label="project-src/index.php" -->
```php
<?php
use Baremetal\Led\Driver\Xyz;

$leds = new Xyz(pin: 5, count: 8);   // the pin your strip is wired to
$leds->fill(0, 0, 32);
$leds->white(0, 128);                // first pixel: white channel only
$leds->show();
```
<!-- @endcode-block -->
