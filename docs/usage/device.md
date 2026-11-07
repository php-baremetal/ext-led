---
eyebrow: 'ext-led · PHP API'
lede:    'The chip classes and their constructor, the colour-order constants, and the buffer methods every LED device has.'
see_also:
  - { href: '../reference/errors-and-limits.md', meta: 'Reference' }
  - { href: '../recipes/rainbow-strip.md', meta: 'Recipe' }
  - { href: 'package:php-baremetal/led-ws2812', meta: 'Driver', label: 'led-ws2812' }
  - { href: 'package:php-baremetal/led-sk6812', meta: 'Driver', label: 'led-sk6812' }
prev: { label: 'Overview', href: '../overview.md' }
next: { label: 'Errors and limits', href: '../reference/errors-and-limits.md' }
---

# LED devices

Each chip package adds one class under `Baremetal\Led\Driver\`. All of them extend `Baremetal\Led\Device`
(which implements `Baremetal\Output\Led`) and share the same constructor shape. A device is a strip of
`count` pixels on one data pin, with a pixel buffer in memory.

## Constructor

<!-- @method name="new Ws2812(int $pin, int $count = 1, ?int $order = null)" visibility="public" -->

<!-- @method name="new Sk6812(int $pin, int $count = 1, ?int $order = null)" visibility="public" -->

<!-- @params -->
<!-- @param name="$pin" type="int" required -->
GPIO number of the data line.
<!-- @endparam -->
<!-- @param name="$count" type="int" default="1" -->
Number of pixels, at least 1. `1` for a single on-board LED.
<!-- @endparam -->
<!-- @param name="$order" type="?int" default="null" -->
Byte order on the wire, one of the class constants. `null` uses the chip's default: `GRB` for `Ws2812`,
`GRBW` for `Sk6812`.
<!-- @endparam -->
<!-- @endparams -->

The constructor creates the strip, then clears it: every pixel is sent as off. While serving an HTTP request
(`web-server` runtime) it does not clear it, so the LEDs keep what the previous request showed; the buffer
starts at off either way. Throws `\ValueError` for a
negative `$pin` (`must be a valid GPIO`), a `$count` below 1 (`must be at least 1`) or an `$order` that is not
one of the class's constants, and `\Exception` `cannot init ws2812 on GPIO N` (`sk6812` for `Sk6812`) when the
HAL cannot create the strip (pin not usable, no free transmit channel, no memory) or another object already
drives the pin (`cannot init ws2812: GPIO N is already used by another LED strip`). To replace a strip, set the
old variable to `null` first: in `$led = new Ws2812(8, 16)` the new object is built before the old `$led` is
released. A strip object cannot be cloned.

`Baremetal\Led\Device` is abstract: always construct a chip class.

## onboard()

<!-- @method name="Ws2812::onboard(string $block = 'led', int $count = 1): static" returns="static" visibility="public" static -->

<!-- @method name="Sk6812::onboard(string $block = 'led', int $count = 1): static" returns="static" visibility="public" static -->

Builds the strip the board describes in its `led` block: the same as `new Ws2812($pin, $count, $order)` with
`$pin` and `$order` read from the block. It is declared on `Device` and returns the class it is called on, so
`Ws2812::onboard()` gives a `Ws2812` and `Sk6812::onboard()` an `Sk6812`. The block's `type` must be that
chip (`ws2812`, `sk6812`); its `order` (`grb`, `rgb`, `grbw`, `rgbw`) becomes the matching colour constant,
and the chip's default is used when the block has none. `$block` names another block of the same kind.

```php
$led = Ws2812::onboard();
$led->fill(0, 32, 0);
$led->show();
```

Throws `\RuntimeException` `board '<id>' has no '<block>' block`,
`board '<id>' has a <block> of type '<type>', not '<chip>'`, `board '<id>': <block>.pin is missing` and
`board '<id>': <block>.pin must be a valid GPIO`, `board '<id>': <block>.order must be grb, rgb, grbw or rgbw`
and `board '<id>': <block>.order '<order>' is not supported by <chip>` (a four-channel order on a three-channel
chip). `$count` below 1 throws `\ValueError` as in the constructor, and the rest throws what the constructor throws.
`Device::onboard()` on the abstract class throws `Error`.

`onboard()` builds the object itself: a PHP subclass of a chip class that defines its own constructor does not
have it called.

## Colour order constants

| Constant | Value | Wire order | On |
| --- | --- | --- | --- |
| `GRB` | `0` | green, red, blue | `Ws2812`, `Sk6812` |
| `RGB` | `1` | red, green, blue | `Ws2812`, `Sk6812` |
| `GRBW` | `2` | green, red, blue, white | `Sk6812` |
| `RGBW` | `3` | red, green, blue, white | `Sk6812` |

Standard WS2812 strips are `GRB`; some on-board LEDs (the one on the Waveshare ESP32-S3-Zero, for example) are
WS2812s in `RGB` order. Standard SK6812 RGBW strips are `GRBW`; SK6812 chips without a white LED take `GRB` or
`RGB`. The order sets the bytes per pixel: three for `GRB` and `RGB`, four for `GRBW` and `RGBW`.

## Buffer methods

These write the buffer only; nothing reaches the LEDs until `show()`. Colour components are clamped to 0 to 255;
out-of-range values never throw.

<!-- @method name="Device::pixel(int $i, int $r, int $g, int $b): void" returns="void" visibility="public" -->

Sets pixel `$i` (0-based) to an RGB colour. `\ValueError` `is out of range for this strip` when `$i` is not
between 0 and `count() - 1`.

<!-- @method name="Device::hsv(int $i, int $h, int $s, int $v): void" returns="void" visibility="public" -->

Sets pixel `$i` from hue `$h` in degrees (any integer, taken modulo 360), saturation `$s` and value `$v` (0 to
255 each). `$v` is the brightness knob. Same range check as `pixel()`.

<!-- @method name="Device::fill(int $r, int $g, int $b): void" returns="void" visibility="public" -->

Sets every pixel to one RGB colour.

## Output methods

<!-- @method name="Device::show(): void" returns="void" visibility="public" -->

Sends the buffer to the LEDs. Blocks until the frame is out.

<!-- @method name="Device::set(int $r, int $g, int $b): void" returns="void" visibility="public" -->

`fill()` and `show()` in one call: the whole strip one colour, at once.

<!-- @method name="Device::off(): void" returns="void" visibility="public" -->

Sets every pixel to off, in the buffer too, and sends the frame.

<!-- @method name="Device::count(): int" returns="int" visibility="public" -->

The number of pixels given to the constructor.

On `Sk6812` the white channel is always written as 0: the methods set R, G and B only. None of the output
methods reports a transfer error.

## Example

<!-- @code-block language="php" label="a strip of 30 WS2812" -->
```php
<?php
use Baremetal\Led\Driver\Ws2812;

$strip = new Ws2812(pin: 5, count: 30);

$strip->fill(0, 0, 16);           // dim blue background
$strip->pixel(0, 64, 0, 0);       // first pixel red
$strip->hsv(29, 120, 255, 64);    // last pixel green, by hue
$strip->show();

delay(2000);                      // ext-sys
$strip->set(16, 16, 16);          // all dim white, sent at once
delay(2000);
$strip->off();
```
<!-- @endcode-block -->
