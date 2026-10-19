---
eyebrow: 'ext-led · Drivers'
lede:    'ext-led owns the led_drivers registry. A chip package registers a descriptor into it, and each descriptor becomes a PHP class under Baremetal\Led\Driver.'
see_also:
  - { href: './writing-a-driver.md', meta: 'Drivers' }
  - { href: 'package:php-baremetal/led-ws2812', meta: 'Driver', label: 'led-ws2812' }
  - { href: 'package:php-baremetal/led-sk6812', meta: 'Driver', label: 'led-sk6812' }
prev: { label: 'Errors and limits', href: '../reference/errors-and-limits.md' }
next: { label: 'Writing a driver', href: './writing-a-driver.md' }
---

# The led_drivers registry

ext-led declares one registry in its `composer.json`:

<!-- @code-block language="json" label="ext-led composer.json, extra.baremetal" -->
```json
{
  "module": "led",
  "priority": 100,
  "registries": {
    "led_drivers": { "type": "led_driver_desc_t", "header": "led_driver_desc.h" }
  }
}
```
<!-- @endcode-block -->

A chip package registers into it by naming its descriptor symbol:

<!-- @code-block language="json" label="led-ws2812 composer.json, extra.baremetal" -->
```json
{ "register": { "led_drivers": ["ws2812_driver_desc"] } }
```
<!-- @endcode-block -->

## What happens at build and startup

<!-- @steps -->
- **The build collects the entries**

  The SDK driver generates a table from every package in the project that registers into `led_drivers`,
  ordered by package and then symbol, NULL-terminated, with a count. With no chip package installed the table
  is empty and ext-led still builds.

- **ext-led registers the classes**

  At module startup, for each descriptor ext-led registers `Baremetal\Led\Driver\<php_class>` as a subclass of
  `Baremetal\Led\Device`, with the descriptor's method table, and declares the order constants `GRB` and `RGB`
  on it, plus `GRBW` and `RGBW` when the descriptor has 4 channels.

- **PHP uses the class**

  `new Ws2812($pin, $count)` creates the strip with the descriptor's LED model, channel count and default
  order, then clears it.
<!-- @endsteps -->

So a project gets a chip class just by requiring the chip package. Classes of chips that are not installed do
not exist; `class_exists(Baremetal\Led\Driver\Ws2812::class)` tells at runtime.

## Drivers available today

| Package | Class | LED model | Channels | Default order |
| --- | --- | --- | --- | --- |
| [led-ws2812](package:php-baremetal/led-ws2812) | `Ws2812` | WS2812 | 3 | `GRB` |
| [led-sk6812](package:php-baremetal/led-sk6812) | `Sk6812` | SK6812 | 4 | `GRBW` |

Both constructors are `(int $pin, int $count = 1, ?int $order = null)`, and both classes have every `Device`
method.

## Rules the build enforces

- Only ext-led may declare `led_drivers`; a second package declaring it is a build error.
- A symbol registered twice, an invalid C name, or a registry that no installed package declares is a build
  error that names the package.
- The header `led_driver_desc.h` comes from ext-led's `include/`, so a driver needs no include path of its own.
