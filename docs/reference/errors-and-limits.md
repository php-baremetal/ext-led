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

ext-led throws `\ValueError` for invalid arguments, plain `\Exception` when a strip cannot be created, and
`\RuntimeException` for a board block `onboard()` cannot use; it defines no exception classes of its own.

## Exceptions

| Message | Thrown by | Cause |
| --- | --- | --- |
| `board '<id>' has no '<block>' block` | `onboard()` | The board does not describe that block. |
| `board '<id>' has a <block> of type '<type>', not '<chip>'` | `onboard()` | The block describes another chip. |
| `board '<id>': <block>.pin is missing` | `onboard()` | The block has no pin. |
| `board '<id>': <block>.order must be grb, rgb, grbw or rgbw` | `onboard()` | The block's `order` is another word. |
| `board '<id>': <block>.pin must be a valid GPIO` | `onboard()` | The block's pin is negative. |
| `board '<id>': <block>.order '<order>' is not supported by <chip>` | `onboard()` | The order is `grbw` or `rgbw` on a three-channel chip (`ws2812`). |
| `Cannot instantiate abstract class Baremetal\Led\Device` | `Device::onboard()` | Called on the abstract base: call it on a chip class. |
| `...: Argument #1 ($pin) must be a valid GPIO` | constructor | `$pin` is negative. |
| `...: Argument #2 ($count) must be at least 1` | constructor | `$count` is 0 or negative. |
| `...: Argument #3 ($order) must be one of the GRB, RGB constants` | `Ws2812` constructor | `$order` is not `GRB` or `RGB`. |
| `...: Argument #3 ($order) must be one of the GRB, RGB, GRBW, RGBW constants` | `Sk6812` constructor | `$order` is not one of the four. |
| `cannot init NAME on GPIO N` | constructor | The HAL could not create the strip: the pin cannot be an output, no transmit channel is free, or no memory. |
| `cannot init NAME: GPIO N is already used by another LED strip` | constructor | Another live object drives that pin. Reuse it, or drop it first. |
| `cannot init NAME on GPIO N: 16 LED strips are already open` | constructor | ext-led's table of strips is full (the transmit channels usually run out first). |
| `LED write failed on GPIO N (error E)` | `pixel()`, `fill()`, `hsv()`, `set()` | The HAL refused the pixel. |
| `LED refresh failed on GPIO N (error E)` | `show()`, `set()`, `off()`, constructor | The frame could not be sent (the transmit channel is busy or failed). |
| `...: Argument #1 ($i) is out of range for this strip` | `pixel()`, `hsv()` | `$i` is negative or not below `count()`. |
| `Trying to clone an uncloneable object of class ...` | `clone` | A copy would not own the strip: keep one object per strip. |
| `Led\Device is not initialised` | any method but `count()` | The object has no strip: a subclass whose constructor did not run. `Baremetal\Led\Device` is abstract: `new` on it throws `Error`. |

`NAME` is the driver name (`ws2812`, `sk6812`).

## Not reported

- Colour components out of 0 to 255 are clamped, and hues wrap modulo 360.

## Limits

| Limit | Value | Source |
| --- | --- | --- |
| Strips at once | one RMT transmit channel each: 4 on ESP32-S3 and ESP32-P4, minus any the firmware uses | esp32-ledstrip-hal |
| Pixels per strip | `count` as given; memory is the practical limit (3 or 4 bytes per pixel in the buffer) | esp32-ledstrip-hal |
| LED models | WS2812 timing and SK6812 timing | esp32-ledstrip-hal |
| Channels | 3 or 4, from the order (`GRB`, `RGB` or `GRBW`, `RGBW`); the white channel is always 0 | ext-led |
| RMT resolution | 10 MHz (0.1 microsecond per tick), no DMA | esp32-ledstrip-hal |

## Lifetime

Each object owns its strip, its transmit channel and its pin, and destroying the object releases all three;
ext-led only remembers which pins are taken, to refuse a second strip on one. The LEDs keep their last frame.
Calling a chip constructor again on the same object replaces its strip. In the `event-driven` runtime keep the
object in a variable the handlers capture (or a global); in the `web-server` runtime it lasts one request, and
the constructor does not clear the strip there, so the LEDs keep what the previous request showed.

Frames are sent from the calling PHP task, which waits until the frame is out. ext-led does nothing special for
the `power-save` option.
