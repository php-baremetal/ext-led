# Changelog

## 1.1.0

- Added: Device::onboard() on every LED driver (Ws2812::onboard(), Sk6812::onboard()), from the board's led block.

## 1.0.2

- Fixed: an `Sk6812` constructed with `GRB` or `RGB` stayed dark, because its pixels were written as RGBW on a
  three-byte strip. The order now sets the bytes per pixel, so SK6812 chips without a white LED work.
- Fixed: `pixel()`, `fill()`, `hsv()`, `set()`, `show()`, `off()` and the constructor ignored HAL errors.
  They now throw `LED write failed on GPIO N (error E)` or `LED refresh failed on GPIO N (error E)`.
- Fixed: two objects on the same GPIO each took a transmit channel and fought over the pin. The second
  constructor now throws `GPIO N is already used by another LED strip`.
- Fixed: in the `web-server` runtime every request switched the LEDs off when it built its strip. The
  constructor no longer clears the strip while serving an HTTP request.
- Fixed: a driver class name longer than 96 characters (with its namespace) read past a fixed buffer.
- Changed: `Baremetal\Led\Device` is abstract; only the driver classes can be constructed.
- Fixed: `clone` on a strip read outside the object and could crash the firmware. It now throws `Error`.

## 1.0.1

- Changed: the author's contact email.
- Docs: projects use `"sdk": "esp32@^2.0"`.

## 1.0.0

Initial release.
