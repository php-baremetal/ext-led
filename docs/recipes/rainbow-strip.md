---
eyebrow: 'ext-led · Recipe'
lede:    'A rotating HSV rainbow on a strip of WS2812 pixels: fill the buffer with hsv(), send it with show(), shift the hue each frame.'
see_also:
  - { href: '../usage/device.md', meta: 'PHP API' }
  - { href: '../reference/errors-and-limits.md', meta: 'Reference' }
prev: { label: 'Status LED', href: './status-led.md' }
next: { label: 'No next page', href: '#' }
---

# Rainbow strip

A 30-pixel WS2812 strip with its data line on GPIO 5 of an ESP32-S3-DevKitC-1, powered from its own 5 V supply
with the ground shared with the board. Change the pin and `PIXELS` to match your strip.

<!-- @code-block language="json" label="composer.json" -->
```json
{
  "name": "acme/rainbow-strip",
  "type": "project",
  "repositories": [
    { "type": "composer", "url": "https://repository.php-baremetal.com", "canonical": true }
  ],
  "require": {
    "php-baremetal/conductor": "^1.0",
    "php-baremetal/ext-sys": "^1.0",
    "php-baremetal/led-ws2812": "^1.0"
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

## The script

<!-- @code-block language="php" label="project-src/index.php" -->
```php
<?php
use Baremetal\Led\Driver\Ws2812;

const PIN = 5;
const PIXELS = 30;
const BRIGHT = 48;         // 0 to 255

$strip = new Ws2812(PIN, PIXELS);   // GRB, the usual order for strips

function loop(int $tick): void
{
    global $strip;

    $shift = ($tick * 4) % 360;
    for ($i = 0; $i < PIXELS; $i++) {
        $strip->hsv($i, $shift + intdiv($i * 360, PIXELS), 255, BRIGHT);
    }
    $strip->show();
    delay(20);
}
```
<!-- @endcode-block -->

- Each frame writes all 30 pixels into the buffer, then sends them with one `show()`.
- Hue values above 359 wrap, so `$shift + ...` needs no modulo.
- If the colours come out swapped (red shows green), the strip is in `RGB` order: pass `Ws2812::RGB`.
- A 3.3 V data line usually drives a 5 V strip over a short wire; for long runs add a level shifter.
