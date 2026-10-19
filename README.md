# ext-led

Addressable RGB LEDs (WS2812, SK6812 and compatible strips, rings and on-board LEDs) for the PHP that runs on
your board. `Baremetal\Led\Device` holds a pixel buffer with `pixel()`, `fill()`, `hsv()`, `show()`, `set()` and
`off()`; chip packages add one class per LED type under `Baremetal\Led\Driver\` (`Ws2812`, `Sk6812`), which you
construct with the data pin and the pixel count.

ext-led is platform-neutral: it requires the virtual `php-baremetal/ledstrip-hal`, and conductor adds the HAL for
your SDK platform (`php-baremetal/esp32-ledstrip-hal` on ESP32, over Espressif's `led_strip` component).

## Install

In a project created with `phpbm init`, require the chip package; it requires ext-led:

```bash
composer require php-baremetal/led-ws2812
```

For SK6812 RGBW LEDs use `php-baremetal/led-sk6812`. Both can be installed together.

## Example

```php
<?php
use Baremetal\Led\Driver\Ws2812;

$strip = new Ws2812(pin: 5, count: 8);       // GRB order, all pixels off

for ($i = 0; $i < $strip->count(); $i++) {
    $strip->hsv($i, $i * 45, 255, 32);       // a dim rainbow into the buffer
}
$strip->show();                              // nothing lights up before this
```

## Documentation

- [Overview](docs/overview.md): requirements, install, a first project, notes.
- [LED devices](docs/usage/device.md): constructors, order constants and every method.
- [Errors and limits](docs/reference/errors-and-limits.md)
- [The led_drivers registry](docs/drivers/registry.md) and [Writing a driver](docs/drivers/writing-a-driver.md)
- Recipes: [Status LED](docs/recipes/status-led.md), [Rainbow strip](docs/recipes/rainbow-strip.md)

## License

MIT.
