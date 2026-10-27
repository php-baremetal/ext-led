---
eyebrow: 'ext-led · Recipe'
lede:    'Use the on-board RGB LED as a status indicator: a colour per state, with a slow breathing effect while idle.'
see_also:
  - { href: '../usage/device.md', meta: 'PHP API' }
  - { href: 'package:php-baremetal/led-ws2812', meta: 'Driver', label: 'led-ws2812' }
prev: { label: 'Writing a driver', href: '../drivers/writing-a-driver.md' }
next: { label: 'Rainbow strip', href: './rainbow-strip.md' }
---

# Status LED

The Waveshare ESP32-S3-Zero (N8R8) has one WS2812 on GPIO 48, in RGB byte order. On another board, check the
data pin (some ESP32-S3 boards use GPIO 38) and the order.

<!-- @code-block language="json" label="composer.json" -->
```json
{
  "name": "acme/status-led",
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
      "board": "waveshare-esp32-s3-zero-n8r8",
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

const HUES = ['idle' => 210, 'busy' => 40, 'ok' => 120, 'error' => 0];

$led = new Ws2812(48, 1, Ws2812::RGB);
$state = 'idle';

function status(string $s): void
{
    global $led, $state;

    $state = $s;
    $led->hsv(0, HUES[$s], 255, 24);
    $led->show();
}

function loop(int $tick): void
{
    global $led, $state;

    if ($state === 'idle') {
        $v = (int) (4 + 20 * (0.5 - 0.5 * cos($tick / 10)));   // breathe between 4 and 24
        $led->hsv(0, HUES['idle'], 255, $v);
        $led->show();
    }
    if ($tick % 200 === 100) {
        status('busy');                                         // stand-ins for real work
    } elseif ($tick % 200 === 150) {
        status(random_int(0, 4) ? 'ok' : 'error');
    } elseif ($tick % 200 === 199) {
        $state = 'idle';
    }
    delay(30);
}

status('idle');
```
<!-- @endcode-block -->

- Values stay low on purpose: a WS2812 at 255 is dazzling a few centimetres away.
- `hsv()` writes the buffer; nothing changes on the LED until `show()`.
- `$led` lives in a global, so the strip stays allocated for the whole run.
