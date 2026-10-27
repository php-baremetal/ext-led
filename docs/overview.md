---
eyebrow: 'Package · Extension'
lede:    'Addressable RGB LEDs from PHP: one class per chip under Baremetal\Led\Driver, a pixel buffer you fill in RGB or HSV and send with show(). Neutral extension; conductor adds the LED-strip HAL for your platform.'
see_also:
  - { href: './usage/device.md', meta: 'PHP API' }
  - { href: './drivers/registry.md', meta: 'Drivers' }
  - { href: 'package:php-baremetal/esp32-ledstrip-hal', meta: 'HAL', label: 'esp32-ledstrip-hal' }
  - { href: 'package:php-baremetal/led-ws2812', meta: 'Driver', label: 'led-ws2812' }
  - { href: 'package:php-baremetal/led-sk6812', meta: 'Driver', label: 'led-sk6812' }
prev: { label: 'No previous page', href: '#' }
next: { label: 'LED devices', href: './usage/device.md' }
---

# Addressable LEDs

ext-led drives addressable LEDs on one data pin: a strip, a ring, or the single RGB LED many boards carry. You
construct the class for your chip (`Baremetal\Led\Driver\Ws2812`, `Baremetal\Led\Driver\Sk6812`) with the pin
and the pixel count, write colours into its buffer by RGB or HSV, and send the buffer to the LEDs with `show()`.
Every chip class extends `Baremetal\Led\Device`, which carries all the methods.

## Requirements

| Requires | Why |
| --- | --- |
| `php-baremetal/ledstrip-hal` (virtual) | The platform side: the strip driver and its timing. On ESP32 it is `php-baremetal/esp32-ledstrip-hal` (platform `esp32`, over Espressif's `led_strip` component on the RMT peripheral). |

You never require the HAL yourself. Conductor reads the platform from `extra.baremetal.sdk` (`esp32@^2.0` gives
`esp32`) and adds `php-baremetal/esp32-ledstrip-hal` while Composer resolves. The ESP32 HAL depends on the
ESP-IDF registry component `espressif/led_strip` `==3.1.0~1`, which the first build downloads, so that build
needs network access. The extension's PHP module is `led`.

A chip class comes from a chip package: ext-led alone gives `Baremetal\Led\Device` and no class you can
construct. Require the chip package; it requires ext-led.

## Install

<!-- @steps -->
- **Install conductor once per machine**

  `composer global require php-baremetal/conductor`

- **Create the project**

  `phpbm init my-leds` writes `composer.json` with conductor, the package registry, and the SDK, board,
  runtime and storage you pick.

- **Require the chip package**

  `composer require php-baremetal/led-ws2812` (or `php-baremetal/led-sk6812`, or both).

- **Build and flash**

  `phpbm flash`, then `phpbm monitor` for the console.
<!-- @endsteps -->

The resulting `composer.json`, for a Waveshare ESP32-S3-Zero (N8R8), whose on-board RGB LED is a WS2812 on
GPIO 48:

<!-- @code-block language="json" label="composer.json" -->
```json
{
  "name": "acme/my-leds",
  "type": "project",
  "repositories": [
    { "type": "composer", "url": "https://repository.php-baremetal.com", "canonical": true }
  ],
  "require": {
    "php-baremetal/conductor": "^1.0",
    "php-baremetal/ext-sys": "^1.0",
    "php-baremetal/ext-led": "^1.0",
    "php-baremetal/led-ws2812": "^1.0"
  },
  "config": {
    "allow-plugins": { "php-baremetal/conductor": true },
    "platform": { "php": "8.5.11" }
  },
  "extra": {
    "baremetal": {
      "sdk": "esp32@^2.0",
      "board": "waveshare-esp32-s3-zero-n8r8",
      "runtime": "init-loop",
      "storage": "embedded"
    }
  }
}
```
<!-- @endcode-block -->

Requiring ext-led next to the chip package is optional; it states that the script uses it directly. `ext-sys`
provides `delay()` for the script below.

## A first script

`project-src/index.php` for the `init-loop` runtime: the top level runs once, then `loop($tick)` runs repeatedly.

<!-- @code-block language="php" label="project-src/index.php" -->
```php
<?php
use Baremetal\Led\Driver\Ws2812;

// the on-board LED of this board is a WS2812 in RGB byte order
$led = new Ws2812(48, 1, Ws2812::RGB);

function loop(int $tick): void
{
    global $led;

    $led->hsv(0, ($tick * 2) % 360, 255, 10);   // slow hue sweep, very dim
    $led->show();
    delay(25);
}
```
<!-- @endcode-block -->

## PHP API at a glance

| Name | Kind | Meaning |
| --- | --- | --- |
| `Baremetal\Led\Device` | class | The base of every chip class: `pixel()`, `fill()`, `hsv()`, `set()`, `show()`, `off()`, `count()`. |
| `Baremetal\Led\Driver\Ws2812` | class | From led-ws2812: 3 channels, GRB order by default. Constants `GRB`, `RGB`. |
| `Baremetal\Led\Driver\Sk6812` | class | From led-sk6812: 4 channels (RGBW), GRBW order by default. Constants `GRB`, `RGB`, `GRBW`, `RGBW`. |
| `Baremetal\Output\Led` | interface | Capability marker implemented by `Device`, so by every chip class. No methods. |

The extension defines no functions. Invalid arguments throw `\ValueError`; a strip that cannot be created throws
`\Exception`. See [LED devices](./usage/device.md) and [Errors and limits](./reference/errors-and-limits.md).

## Drivers

ext-led declares the registry `led_drivers` (element type `led_driver_desc_t`, header `led_driver_desc.h`).
Chip packages (`bm-lib`) register a descriptor into it with `extra.baremetal.register`, and at module startup
ext-led registers one PHP class per descriptor. Available today:

| Package | Class | Channels | Default order |
| --- | --- | --- | --- |
| [led-ws2812](package:php-baremetal/led-ws2812) | `Baremetal\Led\Driver\Ws2812` | 3 | GRB |
| [led-sk6812](package:php-baremetal/led-sk6812) | `Baremetal\Led\Driver\Sk6812` | 4 | GRBW |

See [The led_drivers registry](./drivers/registry.md) and [Writing a driver](./drivers/writing-a-driver.md).

## Notes

- A strip belongs to its PHP object: it is created by the constructor and released when the object is
  destroyed. Keep the object alive (a global, as above) for as long as you use the LEDs. In the `web-server`
  runtime an object created during a request is released at the end of it.
- Releasing a strip does not turn the LEDs off: they keep the last frame. Call `off()` first.
- `pixel()`, `fill()` and `hsv()` only write the buffer; `show()`, `set()` and `off()` send it.
- There is no global brightness: scale the colours yourself (the `$v` of `hsv()` is the easy knob). A long
  strip at full white can draw more current than a USB port supplies; power it separately with a common ground.
