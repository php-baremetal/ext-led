---
eyebrow: 'ext-led · Reference'
lede:    'Every exception ext-led throws, what it does not report, and the limits that come from the platform.'
see_also:
  - { href: '../usage/device.md', meta: 'PHP API' }
  - { href: 'package:php-baremetal/esp32-ledstrip-hal', meta: 'HAL', label: 'esp32-ledstrip-hal' }
prev: { label: 'LED devices', href: '../usage/device.md' }
next: { label: 'The led_drivers registry', href: '../drivers/registry.md' }
---

# Errors and limits

ext-led throws `\ValueError` for invalid arguments and plain `\Exception` when a strip cannot be created; it
defines no exception classes of its own.

## Exceptions

| Message | Thrown by | Cause |
| --- | --- | --- |
| `...: Argument #1 ($pin) must be a valid GPIO` | constructor | `$pin` is negative. |
| `...: Argument #2 ($count) must be at least 1` | constructor | `$count` is 0 or negative. |
| `...: Argument #3 ($order) must be one of the GRB, RGB constants` | `Ws2812` constructor | `$order` is not `GRB` or `RGB`. |
| `...: Argument #3 ($order) must be one of the GRB, RGB, GRBW, RGBW constants` | `Sk6812` constructor | `$order` is not one of the four. |
| `cannot init NAME on GPIO N` | constructor | The HAL could not create the strip: the pin cannot be an output, no transmit channel is free, or no memory. |
| `...: Argument #1 ($i) is out of range for this strip` | `pixel()`, `hsv()` | `$i` is negative or not below `count()`. |
| `Led\Device is not initialised` | any method but `count()` | The object has no strip: a bare `Device`, or a subclass whose constructor did not run. |

`NAME` is the driver name (`ws2812`, `sk6812`).

## Not reported

- `show()`, `set()` and `off()` ignore the result of the transfer: a failed frame raises nothing.
- Colour components out of 0 to 255 are clamped, and hues wrap modulo 360.
- Two objects on the same pin are not detected; each takes its own transmit channel. Use one object per pin.

## Limits

| Limit | Value | Source |
| --- | --- | --- |
| Strips at once | one RMT transmit channel each: 4 on ESP32-S3 and ESP32-P4, minus any the firmware uses | esp32-ledstrip-hal |
| Pixels per strip | `count` as given; memory is the practical limit (3 or 4 bytes per pixel in the buffer) | esp32-ledstrip-hal |
| LED models | WS2812 timing and SK6812 timing | esp32-ledstrip-hal |
| Channels | 3 (RGB) or 4 (RGBW); the white channel is always 0 | ext-led |
| RMT resolution | 10 MHz (0.1 microsecond per tick), no DMA | esp32-ledstrip-hal |

## Lifetime

Unlike the bus extensions, ext-led keeps no global table: each object owns its strip and its transmit channel,
and destroying the object releases both. The LEDs keep their last frame. Calling a chip constructor again on the
same object replaces its strip. In the `event-driven` runtime keep the object in a variable the handlers capture
(or a global); in the `web-server` runtime it lasts one request.

Frames are sent from the calling PHP task, which waits until the frame is out. ext-led does nothing special for
the `power-save` option.
